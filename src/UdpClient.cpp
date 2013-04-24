#include "UdpClient.h"

#include "boost/bind.hpp"
#include "cinder/Utilities.h"

using boost::asio::ip::udp;
using namespace ci;
using namespace std;

UdpClientRef UdpClient::create( boost::asio::io_service& io )
{
	return UdpClientRef( new UdpClient( io ) );
}

UdpClient::UdpClient( boost::asio::io_service& io )
: Client( io )
{
}

UdpClient::~UdpClient()
{
	if ( !mIoService.stopped() ) {
		mIoService.stop();
	}
	disconnect();
}

void UdpClient::connect( const string& host, uint16_t port )
{
	mHost = host;
	mPort = port;
	try {
		// Resolve host
		boost::asio::ip::udp::resolver::query query( mHost, toString( mPort ) );
		boost::asio::ip::udp::resolver resolver( mIoService );
		boost::asio::ip::udp::resolver::iterator destination = resolver.resolve( query );
		while ( destination != boost::asio::ip::udp::resolver::iterator() ) {
			mEndpoint = *destination++;
		}
		mSocket = UdpSocketRef( new udp::socket( mIoService ) );
		
		// Convert address to V4 IP
		boost::asio::ip::address_v4 ip;
		ip = boost::asio::ip::address_v4::from_string( mEndpoint.address().to_string() );
		mEndpoint.address( ip );

		// Open socket
		mSocket->open( mEndpoint.protocol() );
		mSocket->connect( mEndpoint );
		mIoService.run();
		mConnected = true;
	} catch ( const std::exception& ex ) {
		throw ExcConnectionFailed( ex.what() );
	}
}

void UdpClient::disconnect()
{
	mConnected = false;
	if ( mSocket && mSocket->is_open() ) {
		mSocket->close();
	}
}

void UdpClient::read()
{
}

void UdpClient::write( const ci::Buffer& buffer )
{
}
