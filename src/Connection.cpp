#include "Connection.h"

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
		istream stream( &mResponse );
		char data[ bytesTransferred ];
		stream.read( data, bytesTransferred );
		mSignalRead( Buffer( data, bytesTransferred ) );
	}
	mResponse.commit( bytesTransferred );
}

void Connection::onWrite( const boost::system::error_code& error, size_t bytesTransferred )
{
	if ( error ) {
		mSignalError( error.message(), bytesTransferred );
	} else {
		istream stream( &mRequest );
		char data[ bytesTransferred ];
		stream.read( data, bytesTransferred );
		mSignalWrite( Buffer( data, bytesTransferred ) );
	}
	mRequest.consume( bytesTransferred );
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

Connection::ExcConnectionFailed::ExcConnectionFailed( const string& msg ) throw()
{
	sprintf( mMessage, "Unable to connect: %s", msg.c_str() );
}
 