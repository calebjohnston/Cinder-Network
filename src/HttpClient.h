#pragma once

#include "TcpClient.h"

typedef std::shared_ptr<class HttpClient>	HttpClientRef;

class HttpClient : public TcpClient
{
public:
	HttpHeader
	typedef std::map<std::string, std::string> HeaderMap;
	
	enum {
		HTTP_1_0, HTTP_1_1, HTTP_2_0
	} typedef HttpVersion;
	
	static HttpClientRef	create();
	~HttpClient();
	
	virtual void			connect( const std::string& host = "localhost", uint16_t port = 80 );

	void					send( const std::string& path, const std::string& method = "GET",
								 uint_fast8_t* buffer = 0, size_t count = 0 );
	
	//! Returns complete header as a string.
	const std::string&		getHeader() const;
	//! Sets header to \a value. This operation erases all fields.
	void					setHeader( const std::string& value );
	
	/*! Erases header field named \a key. Throws ExcHeaderNotFound
		if \a key is not found. */
	void					eraseHeaderField( const std::string& key );
	/*! Finds and returns the value for header field named \a key. 
		Throws ExcHeaderNotFound if \a key is not found. */
	const std::string&		getHeaderField( const std::string& key ) const;
	/*! Assigns header field \a key to \a value. A new field is created
		if \a key is not found. */
	void					setHeaderField( const std::string& key, const std::string& value );
	
	HttpVersion				getHttpVersion() const { return mHttpVersion; }
	void					setHttpVersion( const HttpVersion v ) { mHttpVersion = v; }
	
	const std::string&		getMethod() const { return mMethod; }
	void					setMethod( const std::string& m ) { mMethod = m; }
	
	const std::string&		getPath() const { return mPath; }
	void					setPath( const std::string& p ) { mPath = p; }
	
	void					setCredentials( const std::string& username, const std::string& password );
	
protected:
	HttpClient();
	
	void					sendImpl( uint_fast8_t* buffer, size_t count );
	
	void					concatenateHeader();
	std::string				mHeader;
	HeaderMap				mHeaders;
	HttpVersion				mHttpVersion;
	std::string				mMethod;
	std::string				mPath;
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
