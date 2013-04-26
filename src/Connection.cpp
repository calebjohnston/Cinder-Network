#include "Connection.h"

#include "cinder/Utilities.h"
#include <iostream>
#include <istream>
#include <ostream>
#include <boost/bind.hpp>

using namespace ci;
using namespace std;

Connection::Connection( boost::asio::io_service& io )
: mIoService( io ), mProtocol( "" )
{
}

void Connection::removeCallback( uint32_t id )
{
	if ( mCallbacks.find( id ) != mCallbacks.end() ) {
		mCallbacks.find( id )->second->disconnect();
		mCallbacks.erase( id );
	}
}

void Connection::onWrite( const boost::system::error_code& err, size_t bytesTransferred )
{
	if ( err ) {
		mSignalError( err.message(), bytesTransferred );
	} else {
		mSignalWrite( bytesTransferred );
	}
}

void Connection::onRead( const boost::system::error_code& err, size_t bytesTransferred )
{
	if ( err ) {
		if ( err == boost::asio::error::eof ) {
			mSignalReadComplete();
			mIoService.stop();
			mIoService.poll();
		} else {
			mSignalError( err.message(), bytesTransferred );
		}
	} else {
		char data[ bytesTransferred ];
		istream stream( &mResponse );
		stream.read( data, bytesTransferred );
		mSignalRead( Buffer( data, bytesTransferred ) );
	}
	mResponse.consume( mResponse.size() );
}
