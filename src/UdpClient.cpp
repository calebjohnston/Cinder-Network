#include "UdpClient.h"

#include "cinder/Utilities.h"
#include <iostream>
#include <istream>
#include <ostream>
#include <boost/bind.hpp>

using namespace ci;
using namespace std;
using boost::asio::ip::udp;

UdpClientRef UdpClient::create( boost::asio::io_service& io )
{
	return UdpClientRef( new UdpClient( io ) );
}

UdpClient::UdpClient( boost::asio::io_service& io )
: Client( io )
{
	mSocket = UdpSocketRef( new udp::socket( mIoService ) );
}

void UdpClient::connect( const string& host, uint16_t port )
{
	connect( host, toString( port ) );
}

void UdpClient::connect( const string& host, const string& protocol )
{
	udp::resolver::query query( host, protocol );
	udp::resolver resolver( mIoService );
	resolver.async_resolve( query, boost::bind( &UdpClient::onResolve, this,
											   boost::asio::placeholders::error,
											   boost::asio::placeholders::iterator ) );
}

void UdpClient::read()
{
	/*mSocket->async_receive( mResponse, boost::asio::transfer_at_least( 1 ),
							boost::bind( &UdpClient::onRead, this,
										boost::asio::placeholders::error,
										boost::asio::placeholders::bytes_transferred ) );*/
}

void UdpClient::write( const Buffer& buffer )
{
	ostream stream( &mRequest );
	stream.write( (const char*)buffer.getData(), buffer.getDataSize() );
	/*mSocket->async_send( mRequest, boost::bind( &UdpClient::onWrite, this,
	 boost::asio::placeholders::error,
	 boost::asio::placeholders::bytes_transferred ) );
	mIoService.run();
	*/
	mRequest.consume( mRequest.size() );
}

void UdpClient::onConnect( const boost::system::error_code& err )
{
	if ( err ) {
		mSignalError( err.message(), 0 );
	} else {
		mSignalConnect();
	}
}

void UdpClient::onResolve( const boost::system::error_code& err,
						  udp::resolver::iterator iter )
{
	if ( err ) {
		mSignalError( err.message(), 0 );
	} else {
		mSignalResolve();
		boost::asio::async_connect( *mSocket, iter,
								   boost::bind( &UdpClient::onConnect, this,
											   boost::asio::placeholders::error ) );
	}
}
