#include "TcpClient.h"

#include "boost/bind.hpp"
#include "cinder/Utilities.h"

using boost::asio::ip::tcp;
using namespace ci;
using namespace ci::app;
using namespace std;

TcpClientRef TcpClient::create( boost::asio::io_service& io )
{
	return TcpClientRef( new TcpClient( io ) );
}

TcpClient::TcpClient( boost::asio::io_service& io )
: Client( io )
{
}

TcpClient::~TcpClient()
{
	disconnect();
}

void TcpClient::connect( const string& host, uint16_t port )
{
	mHost = host;
	mPort = port;
	try {
		// Resolve host
		boost::asio::ip::tcp::resolver::query query( mHost, toString( mPort ) );
		boost::asio::ip::tcp::resolver resolver( mIoService );
		boost::asio::ip::tcp::resolver::iterator destination = resolver.resolve( query );
		while ( destination != boost::asio::ip::tcp::resolver::iterator() ) {
			mEndpoint = *destination++;
		}
		
		// Convert address to V4 IP and set endpoint
		boost::asio::ip::address_v4 ip;
		ip = boost::asio::ip::address_v4::from_string( mEndpoint.address().to_string() );
		mEndpoint.address( ip );
		
		// Open socket
		mSocket = TcpSocketRef( new tcp::socket( mIoService ) );
		mSocket->open( mEndpoint.protocol() );
		mSocket->connect( mEndpoint );
		
		mConnected = true;
	} catch ( const std::exception& ex ) {
		throw ExcConnectionFailed( ex.what() );
	}
}

void TcpClient::disconnect()
{
	mConnected = false;
	if ( mSocket && mSocket->is_open() ) {
		mSocket->close();
	}
}

void TcpClient::read()
{
	if ( mSocket && mConnected ) {
		boost::asio::async_read( *mSocket.get(), mResponse, boost::bind( &TcpClient::onRead, this,
																		boost::asio::placeholders::error,
																		boost::asio::placeholders::bytes_transferred ) );
	}
}

void TcpClient::read( size_t bufferSize )
{
	if ( mSocket && mConnected ) {
		mSocket->async_read_some( mResponse.prepare( bufferSize ), boost::bind( &TcpClient::onRead, this,
																			   boost::asio::placeholders::error,
																			   boost::asio::placeholders::bytes_transferred ) );
	}
}

void TcpClient::read( const std::string &delimiter )
{
	if ( mSocket && mConnected ) {
		boost::asio::async_read_until( *mSocket.get(), mResponse, delimiter.c_str(), boost::bind( &TcpClient::onRead, this,
																								 boost::asio::placeholders::error,
																								 boost::asio::placeholders::bytes_transferred ) );
	}
}

void TcpClient::write( const ci::Buffer& buffer )
{
	ostream stream( &mRequest );
	stream.write( (const char*)buffer.getData(), buffer.getDataSize() );
	if ( mSocket && mConnected ) {
		boost::asio::async_write( *mSocket.get(), mRequest.data(), boost::bind( &TcpClient::onWrite, this,
																		boost::asio::placeholders::error,
																		boost::asio::placeholders::bytes_transferred ) );
	}
}

TcpSocketRef TcpClient::getSocket() const
{
	return mSocket;
}
