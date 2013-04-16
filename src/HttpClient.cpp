
#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/archive/iterators/ostream_iterator.hpp>

#include "cinder/Utilities.h"

#include "HttpClient.h"

using namespace ci;
using namespace std;

void HttpClient::Request::eraseHeaderField( const string& key )
{
	KeyValueMapIter value_iter = mHeaders.find( key );
	if ( value_iter != mHeaders.end() ) {
		mHeaders.erase( value_iter );
		mRawHeaderInvalidated = true;
	}
}

string HttpClient::Request::getHeaderField( const string& key ) const
{
	KeyValueMapConstIter value_iter = mHeaders.find( key );
	if ( value_iter != mHeaders.end() ) {
		return value_iter->second;
	}
	else return "";
}

void HttpClient::Request::setHeaderField( const string& key, const string& value )
{
	mHeaders[ key ] = value;
	mRawHeaderInvalidated = true;
}

void HttpClient::Request::setAuthField( const string& username, const string& password )
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


const string& HttpClient::Request::toString() const
{
	if (mRawHeaderInvalidated) mRawHeaders = concatenateHeader();
	
	return mRawHeaders;
}

string HttpClient::Request::concatenateHeader() const
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


void HttpClient::Request::setData( const ci::Buffer& data, const std::string& contentType )
{
	if (!contentType.empty()) {
		setHeaderField("Content-Type", contentType);
	}
	
	mData = data;
}

HttpClient::Response::Response( std::istream& response )
{
	// WARNING This parsing method is really lazy and dumb, but it works well enough for a demo
	std::list<std::string> header_lines;
	
	string http_version, http_header, http_status;
	response >> http_version;
	response >> mStatusCode;
	response >> http_status;
	
	//-- TEST! ---------------------
	app::console() << "Response status code " << mStatusCode << std::endl;
	//------------------------------
	
	while (getline(response, http_header)){
		header_lines.push_back(http_header);
	}
	for (std::list<std::string>::iterator itr = header_lines.begin(); itr != header_lines.end(); ++itr) {
		vector<string> keyValuePair = split(*itr, ":");
		if (keyValuePair.size() < 2) continue;
		
		string key = keyValuePair.at(0);
		string value = keyValuePair.at(1);
		mHeaders[ key ] = value;
		
		
		//-- TEST! ---------------------
		app::console() << key << ": " << value << std::endl;
		//------------------------------
	}
}

HttpClient::Response::Response( std::string& content )
{
	// parse...
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
	Request req;
	req.setPath(path);
	req.setMethod(method);
	mHeader = req.toString();
	
	sendImpl( buffer, count );
}

void HttpClient::send( const std::string& header, uint_fast8_t* buffer, size_t count )
{
	mHeader = header;
	
	sendImpl( buffer, count );
}

void HttpClient::send( const Request& request, const ci::Buffer& data )
{
	mHeader = request.toString();
	
	uint_fast8_t* buffer;
	size_t count;
	if (data) {
		buffer = (uint_fast8_t*) data.getData();
		count = data.getDataSize();
	}
	else if (request.mData) {
		buffer = (uint_fast8_t*) request.mData.getData();
		count = request.mData.getDataSize();
	}
	else {
		buffer = 0;
		count = 0;
	}
	
	sendImpl( buffer, count );
}

void HttpClient::onSend( const string& message, const boost::system::error_code& error, size_t bytesTransferred )
{
	if (error) {
		throw ExcHttpError(error.message());
	}
	else if (bytesTransferred == 0) {
		throw ExcHttpError("HTTP send failed.");
	}
	
	if ( mSocket ) {
		/*
		boost::system::error_code err;
		// is this how we'll get a return??
		boost::asio::streambuf response_buffer;
		std::istream response_stream(&response_buffer);
		size_t bytes_read = boost::asio::read_until(*mSocket.get(), response_buffer, "\r\n", err);
		
		if (err) {
			throw ExcHttpError(err.message());
		}
		else if (bytes_read == 0) {
			throw ExcHttpError("HTTP recieve failed.");
		}
		 */
	}
	else {
		// auto connect???
	}
}

void HttpClient::sendImpl( uint_fast8_t* buffer, size_t count )
{
	if ( !mSocket ) return; // auto connect???
	
	mHeader += "\r\n";
	// Append header to buffer
	size_t headerSize			= mHeader.size() * sizeof( uint_fast8_t );
	size_t total				= count + headerSize;
	uint_fast8_t* headerBuffer	= new uint_fast8_t[ total ];
	memcpy( headerBuffer, &mHeader[ 0 ], headerSize );
	
	if ( buffer && count ) {
		memcpy( &headerBuffer[ headerSize ], buffer, count );
	}
	
	boost::asio::streambuf request_buffer;
	std::ostream request_stream(&request_buffer);
	request_stream << mHeader;
	boost::asio::write(*mSocket.get(), request_buffer);
//	mSocket->async_send( boost::asio::buffer( headerBuffer, total ), boost::bind(&HttpClient::onSend, this, std::_1, std::_2) );
	
	// ***TEMPORARY***
	//-----------------------------------
	boost::system::error_code err;
	boost::asio::streambuf response_buffer;
	std::istream response_stream(&response_buffer);
	boost::asio::read_until(*mSocket.get(), response_buffer, "\r\n", err);
	//-----------------------------------
	
	Response response(response_stream);
	//dispatch the response object/event
	
	delete [] headerBuffer;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

HttpClient::ExcHttpError::ExcHttpError( const string& msg ) throw()
{
	sprintf( mMessage, "Http response error %s", msg.c_str() );
}
