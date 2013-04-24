#include "Connection.h"

#include "cinder/Buffer.h"

using namespace ci;
using namespace std;

Connection::Connection( boost::asio::io_service& io )
: mConnected( false ), mIoService( io ), mPort( 0 )
{
}

bool Connection::isConnected() const
{
	return mConnected;
}

uint16_t Connection::getPort() const
{
	return mPort;
}

void Connection::onRead( const boost::system::error_code& error, size_t bytesTransferred )
{
	if ( error ) {
		mSignalError( error.message(), bytesTransferred );
	} else {
		mSignalRead( readBuffer( mResponse, bytesTransferred ) );
	}
	mResponse.commit( bytesTransferred );
}

void Connection::onWrite( const boost::system::error_code& error, size_t bytesTransferred )
{
	if ( error ) {
		mSignalError( error.message(), bytesTransferred );
	} else {
		mSignalWrite( readBuffer( mRequest, bytesTransferred ) );
	}
	mRequest.consume( bytesTransferred );
}

Buffer Connection::readBuffer( boost::asio::streambuf& buffer, size_t count )
{
	istream stream( &buffer );
	char data[ count ];
	stream.read( data, count );
	return Buffer( data, count );
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

Connection::ExcConnectionFailed::ExcConnectionFailed( const string& msg ) throw()
{
	sprintf( mMessage, "Unable to connect: %s", msg.c_str() );
}
 