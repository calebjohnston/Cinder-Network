
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/archive/iterators/ostream_iterator.hpp>

#include "cinder/Utilities.h"

#include "HttpStreamingClient.h"

using namespace ci;
using namespace std;

HttpStreamingClientRef HttpStreamingClient::create()
{
	return HttpStreamingClientRef( new HttpStreamingClient() );
}

HttpStreamingClient::HttpStreamingClient() : HttpClient(), mResponseStream(&mResponse)
{
	mQueueFromServer = new (std::nothrow) ConcurrentQueue<string>();
	if (mQueueFromServer) mImageBuffer = Buffer(IMAGE_BUFFER_SIZE);
}

HttpStreamingClient::~HttpStreamingClient()
{
	if (mStreaming) closeStream();
}

void HttpStreamingClient::connect( const std::string& host, uint16_t port )
{	
	if (mStreaming) {
		if (mStreamThreadRef && mStreamThreadRef->joinable()) mStreamThreadRef->join();
		mBufferRemainder.clear();
		mJpegBuffer.clear();
	}
	
	// allocate image data
	// gotta fix this issue
	//mStreamSurface_external = Surface8u(mSession.mResolution.width, mSession.mResolution.height, mSession.mResolution.width * mSession.mResolution.height * 3, SurfaceChannelOrder::RGB);
	//mStreamSurface_internal = Surface8u(mSession.mResolution.width, mSession.mResolution.height, mSession.mResolution.width * mSession.mResolution.height * 3, SurfaceChannelOrder::RGB);
	mNewImageIsReady = false;
	
	if (this->openConnection()) {
		mStreaming = true;
		
		// start network thread
		mStreamThreadRef = shared_ptr<thread>(new (std::nothrow) thread(boost::bind(&HttpStreamingClient::onStreamData, this)));
	}
}

Surface8u HttpStreamingClient::getLatestImage()
{
	mNewImageIsReady = false;
	std::lock_guard<std::mutex> lock(mStreamSurfaceMutex);
	return mStreamSurface_external;
}

void HttpStreamingClient::update()
{
//	if (!mSocket->is_open()) {
//		if (!mStreaming && mRestartConnection) reconnect();
//		return;
//	}

	if (mStreaming && mQueueFromServer && !mQueueFromServer->empty()) {
		
		string* temp_jpg_data = new (std::nothrow) string();
		if (!temp_jpg_data) return;
		
		if (mQueueFromServer->try_pop(*temp_jpg_data) && !temp_jpg_data->empty()) {
			mImageBuffer.resize(temp_jpg_data->size());	// can we avoid this??
			mImageBuffer.copyFrom((char*) temp_jpg_data->c_str(), temp_jpg_data->size());
			DataSourceBufferRef source = DataSourceBuffer::create(mImageBuffer);
			mStreamSurfaceMutex.lock();
			try {
				mStreamSurface_internal = loadImage(source, ImageSource::Options(), "jpg");
				mStreamSurface_external = mStreamSurface_internal.clone();
				mFramesExtracted++;
				mNewImageIsReady = true;
			}
			catch (...) {
				// handle!
			}
			mStreamSurfaceMutex.unlock();
		}
		
		// If we get behind on processing the feed, then we'll force it to catch up a bit
		while(mQueueFromServer->size() > 2) {
			mQueueFromServer->try_pop(*temp_jpg_data);
		}
		
		delete temp_jpg_data;
	}
}

void HttpStreamingClient::closeStream()
{	
	mStreaming = false;
	mStreamThreadRef.reset();
	
	std::lock_guard<std::mutex> lock(mSocketConnectionMutex);
	mSocket->close();
	mIoService.stop();
	mIoService.reset();
	mResponse.consume(mResponse.size());
	mResponseStream.clear();
	mBufferRemainder.clear();
	mJpegBuffer.clear();
	mTimer.stop();
	if (mQueueFromServer) mQueueFromServer->try_clear();
}

bool HttpStreamingClient::openConnection()
{
	// Assume we're streaming, will be toggled upon finding trouble
	bool is_streaming = true;
	
//	HttpClient::connect(mHost, mPort);
//	HttpClient::send(mPath);
	
	// Form the HTTP request for data
	boost::asio::streambuf request;
	ostream request_stream(&request);
	request_stream << "GET " << mPath << " HTTP/1.0\r\n";
	request_stream << "Host: " << mHost << "\r\n";
	request_stream << "Accept: */*\r\n";
	request_stream << "Connection: close\r\n\r\n";
	
	// Dispatch the request
	boost::system::error_code err;	// using error codes instead of exceptions
	boost::asio::write(*mSocket.get(), request);
	size_t bytes_read = boost::asio::read_until(*mSocket.get(), mResponse, "\r\n", err);
	if (!bytes_read || err) {
		return false;
	}
	
	// Check that response is OK.
	uint32_t status_code;
	string http_version, http_header, http_status;
	mResponseStream >> http_version;
	mResponseStream >> status_code;
	getline(mResponseStream, http_status);
	if (!mResponseStream || http_version.substr(0, 5) != "HTTP/") {
		return false;
	}
	else if (status_code != 200) {
		return false;
	}
	else {
		mConnected = true;
	}
	
	// Read the response headers, which are terminated by a blank line.
	bytes_read = boost::asio::read_until(*mSocket.get(), mResponse, "\r\n\r\n");
	if (!bytes_read || err) {
		is_streaming = false;
	}
	
	// Process the HTTP response headers
	while (getline(mResponseStream, http_header) && http_header != "\r"){
		//INFO() << http_header;
	}
	
	// Verify the file boundary position.
	getline(mResponseStream, http_header);
	boost::replace_all(http_header, "\r", "");
	if (http_header != MIME_BOUNDARY_PREFIX) {
		return false;
	}
	
	// Verify the content type
	string content_type, content_length;
	getline(mResponseStream, http_header);
	if (boost::starts_with(http_header, CONTENT_TYPE)) {
		content_type = http_header.substr(CONTENT_TYPE.size());
		boost::replace_all(content_type, "\r", "");
	}
	if (content_type != JPEG_CONTENT_TYPE) {
		return false;
	}
	
	// Verify the content length
	uint32_t length = 0;
	getline(mResponseStream, http_header);
	if (boost::starts_with(http_header, CONTENT_LENGTH)) {
		content_length = http_header.substr(CONTENT_LENGTH.size());
		boost::replace_all(content_length, "\r", "");
		length = boost::lexical_cast<uint32_t>(content_length);
	}
	if (length == 0 || length > IMAGE_BUFFER_SIZE) {
		return false;
	}
	
	if (!is_streaming) return false;
	
	// strip off the empty line delimiter
	getline(mResponseStream, http_header);
	
	mTimer.start();
	
	return is_streaming;
}

void HttpStreamingClient::onStreamData()
{
	size_t bytes_read;
	boost::system::error_code err;
	uint8_t failure_tolernce = 20;
	uint8_t read_failures = 0;
	while (mStreaming) {
		// keep reading until we find the next file boundary
		bytes_read = boost::asio::read_until(*mSocket.get(), mResponse, MIME_BOUNDARY_PREFIX, err);
		if (err == boost::asio::error::eof) {
			this->closeStream();
			return;
		}
		else if (!bytes_read || err) {
			read_failures++;
			if(read_failures > failure_tolernce){
				// NOT YET IMPLEMENTED
				//this->connect();
				return;
			}
		}
		else {
			read_failures = 0;
		}
		
		std::lock_guard<std::mutex> lock(mSocketConnectionMutex);
		
		// collect all the data into a string buffer for processing
		string buffer = string((istreambuf_iterator<char>(mResponseStream)), istreambuf_iterator<char>());
		buffer = mBufferRemainder + buffer;	// prepend any extra data from the previous cycle
		
		// scrape out the first chunk of data, up until the next file boundary
		size_t first_index = buffer.rfind(MIME_BOUNDARY_PREFIX);
		if (first_index == std::string::npos) continue;			// If we can't find the next segment, then we need to start over
		mJpegBuffer = buffer.substr(0,first_index);		// here's our JPG data, yah!
		
		// Now, we need to find the next file starting point by skipping past the meta data
		mBufferRemainder = buffer.substr(first_index);
		if (boost::starts_with(mBufferRemainder, MIME_BOUNDARY_PREFIX)) {
			size_t index = mBufferRemainder.rfind("\r\n\r\n");
			
			if (index == std::string::npos) continue;
			
			mBufferRemainder = mBufferRemainder.substr(index + 4);
		}

		// Add raw JPEG data to the concurrent buffer; to be used during the update method
		mQueueFromServer->push(mJpegBuffer);
		mFramesCaptured++;
	}
}
