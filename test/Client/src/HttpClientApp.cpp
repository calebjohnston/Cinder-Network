#include "cinder/app/AppBasic.h"
#include "cinder/params/Params.h"

#include "TcpCLient.h"

class HttpClientApp : public ci::app::AppBasic 
{
public:
	void						draw();
	void						setup();
	void						update();
private:
	TcpClientRef				mClient;
	
	void						onError( std::string error, size_t bytesTransferred );
	void						onRead( ci::Buffer buffer );
	void						onWrite( ci::Buffer buffer );
	
	float						mFrameRate;
	bool						mFullScreen;
	ci::params::InterfaceGlRef	mParams;
};

#include "cinder/gl/gl.h"

using namespace ci;
using namespace ci::app;
using namespace std;

void HttpClientApp::draw()
{
	gl::clear( Colorf::black() );
	
	mParams->draw();
}

void HttpClientApp::onError( string error, size_t bytesTransferred )
{
	console() << "Error: " << error << "." << endl;
}

void HttpClientApp::onRead( ci::Buffer buffer )
{
	console() << buffer.getDataSize() << " bytes read." << endl;
}

void HttpClientApp::onWrite( ci::Buffer buffer )
{
	console() << buffer.getDataSize() << " bytes written." << endl;
	
	size_t dataSize = buffer.getDataSize();
	Buffer padded( dataSize + 1 );
	memcpy( padded.getData(), buffer.getData(), dataSize );
	( static_cast<uint8_t*>( padded.getData() ) )[ dataSize ] = 0;
	string s( static_cast<const char*>( padded.getData() ) );
	
	app::console() << s << endl; // This is crap right now
	
	mClient->read();
}

void HttpClientApp::setup()
{	
	mFrameRate	= 0.0f;
	mFullScreen	= false;
	
	mClient = TcpClient::create( io_service() );
	mClient->addErrorCallback( &HttpClientApp::onError, this );
	mClient->addReadCallback( &HttpClientApp::onRead, this );
	mClient->addWriteCallback( &HttpClientApp::onWrite, this );
	
	string header	= "GET / HTTP/1.1\r\n";
	header			+= "Accept: */*\r\n";
	header			+= "Accept-Language: en-us\r\n";
	header			+= "Cache-Control: no-cache\r\n";
	header			+= "Connection: close\r\n";
	header			+= "Host: www.google.com\r\n";
	
	ci::Buffer buffer( &header[ 0 ], header.size() * sizeof( char ) );
	
	try {
		mClient->connect( "www.google.com", 80 );
		mClient->write( buffer );
	} catch ( Client::ExcConnectionFailed ex ) {
		console() << ex.what() << endl;
		quit();
	}
	
	mParams = params::InterfaceGl::create( "Params", Vec2i( 200, 150 ) );
	mParams->addParam( "Frame rate",	&mFrameRate,					"", true );
	mParams->addParam( "Full screen",	&mFullScreen,					"key=f" );
	mParams->addButton( "Quit", bind(	&HttpClientApp::quit, this ),	"key=q" );
}

void HttpClientApp::update()
{
	mFrameRate = getFrameRate();
	
	if ( mFullScreen != isFullScreen() ) {
		setFullScreen( mFullScreen );
		mFullScreen = isFullScreen();
	}
}

CINDER_APP_BASIC( HttpClientApp, RendererGl )
