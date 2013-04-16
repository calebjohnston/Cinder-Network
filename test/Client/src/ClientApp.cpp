#include "cinder/app/AppBasic.h"

#include "HttpClient.h"

class ClientApp : public ci::app::AppBasic 
{
public:
	ClientApp() {}
	
	void draw();
	void setup();
	void update();
private:
	HttpClientRef mClient;
};

#include "cinder/gl/gl.h"

using namespace ci;
using namespace ci::app;

void ClientApp::draw()
{
	gl::clear( Colorf::black() );
}

void ClientApp::setup()
{
	mClient = HttpClient::create();
	
	HttpClient::Request request;
	request.setHttpVersion(HttpClient::HTTP_1_1);
	request.setHeaderField("Host", "www.google.com");
	request.setHeaderField("Connection", "close");
	request.setHeaderField("Accept", "*/*");
	request.setHeaderField("Accept-Language", "en-us");
	request.setHeaderField("Cache-Control", "no-cache");
	request.setMethod("GET");
	request.setPath("/");
	
	// works, but response isn't parsed that well yet
	mClient->connect("www.google.com", 80);
	mClient->send(request);
	
	// This also works with a raw string header --could be generated elsewhere...
	//mClient->connect("www.google.com", 80);
	//mClient->send("GET / HTTP/1.1\r\nHost: www.google.com\r\nConnection: close\r\nAccept: */*\r\nAccept-Language: en-us\r\nAccept-Charset: ISO-8859-1,UTF-8;q=0.7,*;q=0.7\r\nCache-Control: no-cache\r\n\r\n");
}

void ClientApp::update()
{
}

CINDER_APP_BASIC( ClientApp, RendererGl )
