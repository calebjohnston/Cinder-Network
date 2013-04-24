#pragma once

#ifdef WIN32
	#include "sdkddkver.h"
#endif
#include "boost/asio.hpp"
#include "boost/signals2.hpp"
#include "cinder/Buffer.h"
#include "cinder/Exception.h"

class Connection
{
public:
	//! Disconnects socket.
	virtual void			disconnect() = 0;
	//! Returns true if connected to socket.
	bool					isConnected() const;
	
	//! Returns port number.
	uint16_t				getPort() const;
	
	//! Receives data and dispatches a ci::Buffer to read callbacks.
	virtual void			read() = 0;
	//! Read \a bufferSize bytes.
	virtual void			read( size_t bufferSize ) = 0;
	//! Read until \a delimiter is reached.
	virtual void			read( const std::string& delimiter ) = 0;
	
	//! Sends \a buffer.
	virtual void			write( const ci::Buffer& buffer ) = 0;
	
	/*! Adds error callback with signature void( std::string, size_t ) and returns ID.
	 ci::Buffer in callback contains server response. */
	template<typename T, typename Y>
	inline uint32_t			addErrorCallback( T callback, Y *callbackObject )
	{
		uint32_t id = mCallbacks.empty() ? 0 : mCallbacks.rbegin()->first + 1;
		mCallbacks.insert( std::make_pair( id, CallbackRef( new Callback( mSignalError.connect( std::bind( callback, callbackObject, std::placeholders::_1, std::placeholders::_2 ) ) ) ) ) );
		return id;
	}
	/*! Adds read callback with signature void( ci::Buffer ) and returns ID.
	 ci::Buffer in callback contains server response. */
	template<typename T, typename Y>
	inline uint32_t			addReadCallback( T callback, Y *callbackObject )
	{
		uint32_t id = mCallbacks.empty() ? 0 : mCallbacks.rbegin()->first + 1;
		mCallbacks.insert( std::make_pair( id, CallbackRef( new Callback( mSignalRead.connect( std::bind( callback, callbackObject, std::placeholders::_1 ) ) ) ) ) );
		return id;
	}
	/*! Adds write callback with signature void( std::string, size_t ) and
	 returns ID. The callback contains error message, and number
	 of bytes transferred. */
	template<typename T, typename Y>
	inline uint32_t			addWriteCallback( T callback, Y *callbackObject )
	{
		uint32_t id = mCallbacks.empty() ? 0 : mCallbacks.rbegin()->first + 1;
		mCallbacks.insert( std::make_pair( id, CallbackRef( new Callback( mSignalWrite.connect( std::bind( callback, callbackObject, std::placeholders::_1 ) ) ) ) ) );
		return id;
	}
protected:
	typedef boost::signals2::connection		Callback;
	typedef std::shared_ptr<Callback>		CallbackRef;
	typedef std::map<uint32_t, CallbackRef>	CallbackList;
	
	explicit					Connection( boost::asio::io_service& io );

	CallbackList											mCallbacks;
	boost::signals2::signal<void ( std::string, size_t )>	mSignalError;
	boost::signals2::signal<void ( ci::Buffer )>			mSignalRead;
	boost::signals2::signal<void ( ci::Buffer )>			mSignalWrite;
	
	bool						mConnected;
	uint16_t					mPort;

	ci::Buffer					mBufferRequest;
	ci::Buffer					mBufferResponse;
	boost::asio::io_service&	mIoService;
	boost::asio::streambuf		mRequest;
	boost::asio::streambuf		mResponse;
	char*						readBuffer( boost::asio::streambuf& buffer, size_t count );
	
	virtual void				onRead( const boost::system::error_code& error,
									   size_t bytesTransferred );
	virtual void				onWrite( const boost::system::error_code& error,
									   size_t bytesTransferred );
public:
	//! Exception expressing connection failure.
	class ExcConnectionFailed : public ci::Exception
	{
	public:
		ExcConnectionFailed( const std::string &msg ) throw();
		virtual const char* what() const throw()
		{
			return mMessage;
		}
		
	private:
		char mMessage[ 2048 ];
	};
};
