#pragma once

#include <boost/asio/buffer.hpp>

#include "TcpClient.h"

typedef std::shared_ptr<class HttpClient> HttpClientRef;

class HttpClient : public TcpClient {
public:
	typedef std::map<std::string, std::string> KeyValueMap;
	typedef std::pair<std::string, std::string> KeyValuePair;
	typedef std::map<std::string, std::string>::iterator KeyValueMapIter;
	typedef std::map<std::string, std::string>::const_iterator KeyValueMapConstIter;
	
	typedef enum {
		HTTP_1_0, HTTP_1_1, HTTP_2_0
	} HttpVersion;
	
	class Request;
	class Response;
	
public:
	static HttpClientRef create();
	virtual ~HttpClient();
	
	virtual void			connect( const std::string& host = "localhost", const uint16_t port = 80 );

	virtual void			send( const std::string& path, const std::string& method,
								 uint_fast8_t* buffer = 0, size_t count = 0 );
	
	virtual void			send( const std::string& header, uint_fast8_t* buffer = 0, size_t count = 0 );
				
	virtual void			send( const Request& request, const ci::Buffer& data = ci::Buffer() );
	
	const std::string&		getHeader() const { return mHeader; }
	
protected:
	HttpClient();
	
	virtual void			sendImpl( uint_fast8_t* buffer, size_t count );
	
	virtual void			onSend(const std::string& message,
								   const boost::system::error_code& error,
								   std::size_t bytesTransferred);
	
	std::string				mHeader;
	
public:	
	class Request {
	public:
		Request( const std::string& rawHeaders = "" ) : mRawHeaders(rawHeaders), mRawHeaderInvalidated(false) {}
		~Request() { mHeaders.clear(); }
		
		/*! Finds and returns the value for header field named \a key. */
		std::string				getHeaderField( const std::string& key ) const;
		/*! Assigns header field \a key to \a value. A new field is created if \a key is not found. */
		void					setHeaderField( const std::string& key, const std::string& value );
		/*! Erases header field named \a key. */
		void					eraseHeaderField( const std::string& key );
		
		inline HttpVersion		getHttpVersion() const { return mHttpVersion; }
		inline void				setHttpVersion( const HttpVersion v ) { mHttpVersion = v; }
		
		const std::string&		getMethod() const { return mMethod; }
		inline void				setMethod( const std::string& m ) { mMethod = m; }
		
		const std::string&		getPath() const { return mPath; }
		inline void				setPath( const std::string& p ) { mPath = p; }
		
		void setAuthField( const std::string& username, const std::string& password );
		
		//! for POST requests
		void setData( const ci::Buffer& data, const std::string& contentType = "" );
		
		const std::string& toString() const;
		
	protected:
		std::string concatenateHeader() const;
		
		mutable bool mRawHeaderInvalidated;
		mutable std::string mRawHeaders;
		
		HttpVersion mHttpVersion;
		KeyValueMap	mHeaders;
		std::string	mMethod;
		std::string	mPath;
		ci::Buffer mData; //!< for POST requests
		
		friend class HttpClient;
		friend class WebSocketClient;
	};
	
	class Response {
	public:
		~Response() { mHeaders.clear(); }
		
		inline uint16_t getStatusCode() const { return mStatusCode; }
		inline uint32_t getContentLength() const { return mContentLength; }
		
		inline ci::Buffer& getContent() { return mData; }
		inline const ci::Buffer& getContent() const { return mData; }
		
	protected:
		Response( std::istream& response );
		Response( std::string& content );
		
		std::string mRawHeaders;
		KeyValueMap	mHeaders;
		ci::Buffer mData;
		uint16_t mStatusCode;
		uint32_t mContentLength;
		
		friend class HttpClient;
		friend class WebSocketClient;
	};
	
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	//! Exception expressing absence of requested header field.
	class ExcHttpError : public Client::Exception {
	public:
		ExcHttpError( const std::string &msg ) throw();
		virtual const char* what() const throw()
		{
			return mMessage;
		}
		
	private:
		char mMessage[ 2048 ];
	};
};
