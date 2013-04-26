#include <boost/algorithm/string.hpp>
#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/archive/iterators/ostream_iterator.hpp>

#include "cinder/app/AppNative.h"
#include "cinder/params/Params.h"

#include "TcpClient.h"

class HttpStreamingClientApp : public ci::app::AppNative {
public:
	void setup();
	void mouseDown( ci::app::MouseEvent event );
	void update();
	void draw();
	
	friend std::string encode( const std::string& username, const std::string& password );
	
private:
	TcpClientRef				mClient;
	std::string					mHost;
	uint16_t					mPort;
	std::string					mRequest;
	std::string					mResponse;
	
	bool						mFullScreen;
	float						mFrameRate;
	ci::params::InterfaceGlRef	mParams;
	
	void						send();
	void						onConnect();
	void						onError( std::string error, size_t bytesTransferred );
	void						onRead( ci::Buffer buffer );
	void						onReadComplete();
	void						onResolve();
	void						onWrite( size_t bytesTransferred );
	
};

std::string encode( const std::string& username, const std::string& password )
{
	std::string http_credentials = username + ":" + password;
	std::stringstream os;
	// Base64 encode username and password for HTTP access authorization
	typedef	boost::archive::iterators::base64_from_binary<boost::archive::iterators::transform_width<const char*, 6, 8> > Base64Text;
	std::copy(Base64Text(http_credentials.c_str()), Base64Text(http_credentials.c_str() + http_credentials.size()), boost::archive::iterators::ostream_iterator<char>(os));
	return os.str();
}

#include "cinder/gl/gl.h"

using namespace ci;
using namespace ci::app;
using namespace std;

void HttpStreamingClientApp::onConnect()
{
	console() << "Connected." << endl;
	mResponse.clear();
	mClient->write( Buffer( &mRequest[ 0 ], mRequest.size() ) );
}

void HttpStreamingClientApp::onError( string error, size_t bytesTransferred )
{
	console() << "Error: " << error << "." << endl;
}

void HttpStreamingClientApp::onReadComplete()
{
	console() << "Read complete." << endl;
	console() << mResponse << endl;
	
	mClient->disconnect();
}

void HttpStreamingClientApp::onRead( ci::Buffer buffer )
{
	// collect headers
	// collect content type
	// collect mime boundary
	
	console() << buffer.getDataSize() << " bytes read." << endl;
	
	// Stringify buffer
	string response( static_cast<const char*>( buffer.getData() ) );
	
	// TODO get rid of this hack
	size_t count = buffer.getDataSize() == 512 ? 5 : 3;
	for ( size_t i = 0; i < count; ++i ) {
		response.pop_back(); // Discard the last few bytes
	}
	
	mResponse += response;
	
	mClient->read();
}

void HttpStreamingClientApp::read()
{
	
	// collect all the data into a string buffer for processing
	string buffer = string((istreambuf_iterator<char>(mObj->mResponseStream)), istreambuf_iterator<char>());
	buffer = mObj->mBufferRemainder + buffer;	// prepend any extra data from the previous cycle
	
	// scrape out the first chunk of data, up until the next file boundary
	size_t first_index = getByteIndex(MIME_BOUNDARY_PREFIX, buffer);
	if (first_index == std::string::npos) continue;			// If we can't find the next segment, then we need to start over
	mObj->mJpegBuffer = buffer.substr(0,first_index);		// here's our JPG data, yah!
	
	// Now, we need to find the next file starting point by skipping past the meta data
	mObj->mBufferRemainder = buffer.substr(first_index);
	if (boost::starts_with(mObj->mBufferRemainder, MIME_BOUNDARY_PREFIX)) {
		size_t index = getByteIndex("\r\n\r\n", mObj->mBufferRemainder);
		
		if (index == std::string::npos) continue;
		
		mObj->mBufferRemainder = mObj->mBufferRemainder.substr(index + 4);
	}
}

void HttpStreamingClientApp::onResolve()
{
	console() << "Endpoint resolved." << endl;
}

void HttpStreamingClientApp::onWrite( size_t bytesTransferred )
{
	console() << bytesTransferred << " bytes written." << endl;
	
	mClient->read();
}

void HttpStreamingClientApp::send()
{
	mClient->connect( mHost, mPort );
}

void HttpStreamingClientApp::setup()
{
	mFrameRate	= 0.0f;
	mFullScreen	= false;
	
	mHost = "libcinder.org";
	mPort = 80;
	
	mRequest = "GET / HTTP/1.0\r\n";
	mRequest += "Host: " + mHost + "\r\n";
	mRequest += "Accept: */*\r\n";
	mRequest += "Connection: close\r\n\r\n";
	mRequest += "Authorization: Basic " + encode("username", "password") + "=\r\n";
	
	mParams = params::InterfaceGl::create( "Params", Vec2i( 200, 150 ) );
	mParams->addParam( "Frame rate",	&mFrameRate,					"", true );
	mParams->addParam( "Full screen",	&mFullScreen,					"key=f" );
	mParams->addButton( "Send", bind(	&HttpStreamingClientApp::send, this ),	"key=s" );
	mParams->addButton( "Quit", bind(	&HttpStreamingClientApp::quit, this ),	"key=q" );
	
	mClient = TcpClient::create( io_service() );
	mClient->addConnectCallback( &HttpStreamingClientApp::onConnect, this );
	mClient->addErrorCallback( &HttpStreamingClientApp::onError, this );
	mClient->addReadCallback( &HttpStreamingClientApp::onRead, this );
	mClient->addReadCompleteCallback( &HttpStreamingClientApp::onReadComplete, this );
	mClient->addResolveCallback( &HttpStreamingClientApp::onResolve, this );
	mClient->addWriteCallback( &HttpStreamingClientApp::onWrite, this );
	
	send();
}

void HttpStreamingClientApp::mouseDown( MouseEvent event )
{
}

void HttpStreamingClientApp::update()
{
	mFrameRate = getFrameRate();
	
	if ( mFullScreen != isFullScreen() ) {
		setFullScreen( mFullScreen );
		mFullScreen = isFullScreen();
	}
}

void HttpStreamingClientApp::draw()
{
	gl::clear( Colorf::black() );
	
	mParams->draw();
}

CINDER_APP_NATIVE( HttpStreamingClientApp, RendererGl )
