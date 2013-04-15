#pragma once

#include "cinder/Surface.h"
#include "cinder/Buffer.h"
#include "cinder/Timer.h"
#include "cinder/Thread.h"

#include "ConcurrentQueue.h"
#include "HttpClient.h"

typedef std::shared_ptr<std::thread> ThreadRef;
typedef std::shared_ptr<class HttpStreamingClient>	HttpStreamingClientRef;

static const size_t IMAGE_BUFFER_SIZE = 1048576;	// 1Mb
static const std::string MIME_BOUNDARY_PREFIX = "--myboundary";
static const std::string CONTENT_TYPE = "Content-Type: ";
static const std::string CONTENT_LENGTH = "Content-Length: ";
static const std::string JPEG_CONTENT_TYPE = "image/jpeg";

class HttpStreamingClient : public HttpClient
{
public:
	static HttpStreamingClientRef create();
	virtual ~HttpStreamingClient();
	
	virtual void connect( const std::string& host = "localhost", const uint16_t port = 80 );

	//void send( const std::string& path, const std::string& method = "GET", uint_fast8_t* buffer = 0, size_t count = 0 );
	virtual void update();
	
	/** Returns true if a previously uncaptured image is available, false otherwise */
	inline bool isNewImageAvailable() const { return mNewImageIsReady; }
	
	/** Accessor method for the latest Surface image loaded from the camera */
	ci::Surface8u getLatestImage();
	
	/** Accessor method for the restart connection policy. If TRUE, the connection will always be re-established (if possible) after disconnection */
	inline bool getAutoRestartConnection() const { return mRestartConnection; }

		/** Updates the restart connection policy. If TRUE, the connection will always be re-established (if possible) after disconnection */
	inline void setAutoRestartConnection(bool restart = true) { mRestartConnection = restart; }
	
protected:
	HttpStreamingClient();
	
	mutable bool mNewImageIsReady;
	bool mRestartConnection;

	int64_t mFramesCaptured;						//!< Running count of the number of jpg frames captured from device
	int64_t mFramesExtracted;						//!< Running count of the number of frames taken from the instance
	ci::Timer mTimer;								//!< Timer used in monitoring the connection performance

	/** stream data */
	bool mStreaming;									//!< If TRUE, then the streaming thread is currently running
	ci::Buffer mImageBuffer;						//!< Buffer used by Cinder/OSX to load an image
	ThreadRef mStreamThreadRef;						//!< Thread used for streaming data
	ci::Surface8u mStreamSurface_external;			//!< Surface data collected from camera data stream
	ci::Surface8u mStreamSurface_internal;			//!< Surface data collected from camera data stream
	std::mutex mStreamSurfaceMutex;					//!< Mutex to ensure atomic operations on the stream surface instances
	std::mutex mSocketConnectionMutex;				//!< 
	ConcurrentQueue<std::string>* mQueueFromServer;	//!< Special thread-safe queue used to pass data between threads
	std::string mBufferRemainder;					//!< Temporary storage buffer between buffer reads
	std::string mJpegBuffer;						//!< Buffer used to store complete JPG file
	boost::asio::streambuf mResponse;				//!< Communication stream buffer to read data from
	std::istream mResponseStream;					//!< Response stream for data processing
	
	/** Opens a connection with the desired remote location, returns success boolean. */
	bool openConnection();
	
	/** Initiates a stream from IP camera given state of the Camera members */
	void onStreamData();
	
	void closeStream();
};
