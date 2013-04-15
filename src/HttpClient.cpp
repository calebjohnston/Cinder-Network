
#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/archive/iterators/ostream_iterator.hpp>

#include "HttpClient.h"

using namespace ci;
using namespace std;

HttpRequest::HttpRequest( const std::string& rawHeaders )
:	mRawHeaders(rawHeaders), mRawHeaderInvalidated(false)
{
}

HttpRequest::~HttpRequest()
{
	mHeaders.clear();
}

void HttpRequest::eraseHeaderField( const string& key )
{
	KeyValueMapIter value_iter = mHeaders.find( key );
	if ( value_iter != mHeaders.end() ) {
		mHeaders.erase( value_iter );
		mRawHeaderInvalidated = true;
	}
}

string HttpRequest::getHeaderField( const string& key ) const
{
	KeyValueMapConstIter value_iter = mHeaders.find( key );
	if ( value_iter != mHeaders.end() ) {
		return value_iter->second;
	}
	else return "";
}

void HttpRequest::setHeaderField( const string& key, const string& value )
{
	mHeaders[ key ] = value;
	mRawHeaderInvalidated = true;
}

void HttpRequest::setFollowRedirect( bool value )
{
	//setHeaderField("","");
}

void HttpRequest::setCacheResponse( bool value )
{
	//setHeaderField("","");
}

void HttpRequest::setUserAgent( const std::string& value )
{
	//setHeaderField("","");
}

void HttpRequest::setTimeout( const uint32_t value )
{
	//setHeaderField("","");
}

void HttpRequest::setConnection()
{
	//setHeaderField("","");
}

void HttpRequest::setAcceptType( const std::string& value )
{
	//setHeaderField("","");
}

void HttpRequest::setAcceptCharset( const std::string& value )
{
	//setHeaderField("","");
}

void HttpRequest::setAcceptLanguage( const std::string& value )
{
	//setHeaderField("","");
}

void HttpRequest::setAcceptEncoding( const std::string& value )
{
	//setHeaderField("","");
}

void HttpRequest::setAuthorization( const string& username, const string& password )
{
	namespace boost_iters = boost::archive::iterators;
	
	typedef	boost_iters::base64_from_binary<boost_iters::transform_width<const char*, 6, 8> > Base64Text;
	
	if (!username.empty() && !password.empty()) {
		string http_credentials = username + ":" + password;
		stringstream os;
		// Base64 encode username and password for HTTP access authorization
		std::copy(Base64Text(http_credentials.c_str()), Base64Text(http_credentials.c_str() + http_credentials.size()), boost_iters::ostream_iterator<char>(os));
		http_credentials = "Basic " + os.str() + "=\r\n";
		setHeaderField("Authorization", http_credentials);
	}
}


const string& HttpRequest::toString() const
{
	mRawHeaders = concatenateHeader();
	return mRawHeaders;
}

string HttpRequest::concatenateHeader() const
{
	string output = mMethod + " " + mPath + " HTTP/";
	switch ( mHttpVersion ) {
		case HTTP_1_0:
			output += "1.0";
			break;
		case HTTP_1_1:
			output += "1.1";
			break;
		case HTTP_2_0:
			output += "2.0";
			break;
	}
	output += "\r\n";
	for ( KeyValueMapConstIter iter = mHeaders.begin(); iter != mHeaders.end(); ++iter ) {
		output += iter->first + ": " + iter->second + "\r\n";
	}
	
	mRawHeaderInvalidated = false;
	
	return output;
}

HttpClientRef HttpClient::create()
{
	return HttpClientRef( new HttpClient() );
}

HttpClient::HttpClient() : TcpClient(), mHeader( "" )
{
}

HttpClient::~HttpClient()
{
	mHeader.clear();
}

void HttpClient::connect( const string& host, uint16_t port )
{
	TcpClient::connect( host, port );
}

void HttpClient::send( const string& path, const string& method, uint_fast8_t* buffer, size_t count )
{
	/*
	bool concat = path != mPath || method != mMethod;
	mPath	= path;
	mMethod	= method;
	if ( concat ) {
		concatenateHeader();
	}
	*/
	sendImpl( buffer, count );
}

void HttpClient::send( const HttpRequest& request )
{
	
}

void HttpClient::send( const std::string& header )
{
	
}

void HttpClient::sendImpl( uint_fast8_t* buffer, size_t count )
{
	// Append header to buffer
	size_t headerSize			= mHeader.size() * sizeof( uint_fast8_t );
	size_t total				= count + headerSize;
	uint_fast8_t* headerBuffer	= new uint_fast8_t[ total ];
	memcpy( headerBuffer, &mHeader[ 0 ], headerSize );
	
	// TODO separator
	
	memcpy( &headerBuffer[ headerSize ], buffer, count );
	
	if ( mSocket ) {
		mSocket->async_send( boost::asio::buffer( headerBuffer, total ),
							boost::bind(& HttpClient::onSend, this, "", boost::asio::placeholders::error, count )
							);
	}
	
	delete [] headerBuffer;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

HttpClient::ExcHeaderNotFound::ExcHeaderNotFound( const string& msg ) throw()
{
	sprintf( mMessage, "Header field not found: %s", msg.c_str() );
}
