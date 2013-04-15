#pragma once

#include "TcpClient.h"

typedef std::map<std::string, std::string> KeyValueMap;
typedef std::pair<std::string, std::string> KeyValuePair;
typedef std::map<std::string, std::string>::iterator KeyValueMapIter;
typedef std::map<std::string, std::string>::const_iterator KeyValueMapConstIter;
typedef std::shared_ptr<class HttpClient> HttpClientRef;

typedef enum {
	HTTP_1_0, HTTP_1_1, HTTP_2_0
} HttpVersion;

class HttpRequest {
public:
	HttpRequest( const std::string& rawHeaders = "" );
	~HttpRequest();
	
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
	
	void setFollowRedirect( bool value = true );
	void setCacheResponse( bool value = true );
	void setUserAgent( const std::string& value );
	void setTimeout( const uint32_t value );
	void setConnection();
	void setAcceptType( const std::string& value );
	void setAcceptCharset( const std::string& value );
	void setAcceptLanguage( const std::string& value );
	void setAcceptEncoding( const std::string& value );
	void setAuthorization( const std::string& username, const std::string& password );
	void setData( const ci::Buffer& data, const std::string& contentType ); // for POST requests
	
	const std::string& toString() const;
	
protected:	
	std::string concatenateHeader() const;
	
	mutable bool mRawHeaderInvalidated;
	mutable std::string mRawHeaders;
	
	HttpVersion mHttpVersion;
	KeyValueMap	mHeaders;
	std::string	mMethod;
	std::string	mPath;
	ci::Buffer mData; // for POST requests
};

class HttpResponse {
public:	
	uint16_t getStatusCode() const;
	uint32_t getContentLength() const;
	const std::string& getConnection() const;
	const std::string& getCacheControl() const;
	const std::string& getContentType() const;
	const std::string& getDate() const;
	
	inline ci::Buffer& getContent() { return mData; }
	inline const ci::Buffer& getContent() const { return mData; }
	
protected:
	std::string mRawHeaders;
	KeyValueMap	mHeaders;
	ci::Buffer mData;
};

class HttpClient : public TcpClient {
public:
	static HttpClientRef create();
	virtual ~HttpClient();
	
	virtual void			connect( const std::string& host = "localhost", const uint16_t port = 80 );

	virtual void			send( const std::string& path, const std::string& method = "GET",
								 uint_fast8_t* buffer = 0, size_t count = 0 );
								
	virtual void			send( const HttpRequest& request );	
						
	virtual void			send( const std::string& header );
	
	const std::string&		getHeader() const { return mHeader; }
	
protected:
	HttpClient();
	
	virtual void			sendImpl( uint_fast8_t* buffer, size_t count );
	
	std::string				mHeader; // needed ?
	
public:

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	//! Exception expressing absence of requested header field.
	class ExcHeaderNotFound : public Client::Exception {
	public:
		ExcHeaderNotFound( const std::string &msg ) throw();
		virtual const char* what() const throw()
		{
			return mMessage;
		}
		
	private:
		char mMessage[ 2048 ];
	};
};
