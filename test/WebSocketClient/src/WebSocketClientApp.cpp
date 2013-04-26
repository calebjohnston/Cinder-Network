#include "cinder/app/AppBasic.h"
#include "cinder/params/Params.h"

#include "TcpClient.h"

class WebSocketClientApp : public ci::app::AppBasic 
{
public:
	void						draw();
	void						setup();
	void						update();
private:
	TcpClientRef				mClient;
	std::string					mHandshake;
	std::string					mHost;
	uint16_t					mPort;
	void						send();
	
	void						onConnect();
	void						onError( std::string error, size_t bytesTransferred );
	void						onRead( ci::Buffer buffer );
	void						onReadComplete();
	void						onResolve();
	void						onWrite( ci::Buffer buffer );
	
	std::string					mResponse;
	
	float						mFrameRate;
	bool						mFullScreen;
	ci::params::InterfaceGlRef	mParams;
};

#include "cinder/gl/gl.h"

using namespace ci;
using namespace ci::app;
using namespace std;

void WebSocketClientApp::draw()
{
	gl::clear( Colorf::black() );
	
	mParams->draw();
}

void WebSocketClientApp::onConnect()
{
	console() << "Connected." << endl;
	mResponse.clear();
	if ( !mHandshake.empty() ) {
		mClient->write( Buffer( &mHandshake[ 0 ], mHandshake.size() ) );
		mHandshake.clear();
	} else {
		string echo( "echo" );
		mClient->write( Buffer( &echo[ 0 ], 4 ) );
	}
}

void WebSocketClientApp::onError( string error, size_t bytesTransferred )
{
	console() << "Error: " << error << "." << endl;
}

void WebSocketClientApp::onReadComplete()
{
	console() << "Read complete." << endl;
	console() << mResponse << endl;
	
	// TODO process response
	mResponse.clear();
	
	mClient->disconnect();
}

void WebSocketClientApp::onRead( ci::Buffer buffer )
{
	console() << buffer.getDataSize() << " bytes read." << endl;
	
	// Stringify buffer
	string response( static_cast<const char*>( buffer.getData() ) );
	for ( size_t i = 0; i < 5; ++i ) {
		response.pop_back(); // Discard the last five bytes
	}
	mResponse += response;
	
	mClient->read();
}

void WebSocketClientApp::onResolve()
{
	console() << "Endpoint resolved." << endl;
}

void WebSocketClientApp::onWrite( ci::Buffer buffer )
{
	console() << buffer.getDataSize() << " bytes written." << endl << endl;
	
	string request( static_cast<const char*>( buffer.getData() ) );
	console() << request << endl;
	
	mClient->read();
}

void WebSocketClientApp::send()
{
	mClient->connect( mHost, mPort );
}

void WebSocketClientApp::setup()
{	
	mFrameRate	= 0.0f;
	mFullScreen	= false;
	
	mHost		= "echo.websocket.org";
	mPort		= 80;
	
	mHandshake += "GET /?encoding=text HTTP/1.1\r\n";
	mHandshake += "Upgrade: websocket\r\n";
	mHandshake += "Connection: Upgrade\r\n";
	mHandshake += "Host: " + mHost + "\r\n";
	mHandshake += "Origin: WebSocketClient\r\n";
	mHandshake += "Pragma: no-cache\r\n";
	mHandshake += "Cache-Control: no-cache\r\n";
	mHandshake += "Sec-WebSocket-Key: e0ReqUBOu8zyDInE07NrrA==\r\n";
	mHandshake += "Sec-WebSocket-Version: 13\r\n";
	mHandshake += "Sec-WebSocket-Extensions: x-webkit-deflate-frame\r\n";

	mParams = params::InterfaceGl::create( "Params", Vec2i( 200, 150 ) );
	mParams->addParam( "Frame rate",	&mFrameRate,					"", true );
	mParams->addParam( "Full screen",	&mFullScreen,					"key=f" );
	mParams->addButton( "Send", bind(	&WebSocketClientApp::send, this ),	"key=s" );
	mParams->addButton( "Quit", bind(	&WebSocketClientApp::quit, this ),	"key=q" );
	
	mClient = TcpClient::create( io_service() );
	mClient->addConnectCallback( &WebSocketClientApp::onConnect, this );
	mClient->addErrorCallback( &WebSocketClientApp::onError, this );
	mClient->addReadCallback( &WebSocketClientApp::onRead, this );
	mClient->addReadCompleteCallback( &WebSocketClientApp::onReadComplete, this );
	mClient->addResolveCallback( &WebSocketClientApp::onResolve, this );
	mClient->addWriteCallback( &WebSocketClientApp::onWrite, this );
	
	send();
}

void WebSocketClientApp::update()
{
	mFrameRate = getFrameRate();
	
	if ( mFullScreen != isFullScreen() ) {
		setFullScreen( mFullScreen );
		mFullScreen = isFullScreen();
	}
}

CINDER_APP_BASIC( WebSocketClientApp, RendererGl )
