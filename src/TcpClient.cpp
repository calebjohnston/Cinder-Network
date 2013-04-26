#include "TcpClient.h"

#include "cinder/Utilities.h"

using namespace ci;
using namespace std;
using boost::asio::ip::tcp;

TcpClientRef TcpClient::create( boost::asio::io_service& io )
{
	return TcpClientRef( new TcpClient( io ) );
}

TcpClient::TcpClient( boost::asio::io_service& io )
: Client( io )
{
	mSocket = TcpSocketRef( new tcp::socket( mIoService ) );
}

TcpClient::~TcpClient()
{
	disconnect();
	
	mIoService.stop();
}

void TcpClient::connect( const string& host, uint16_t port )
{
	connect( host, toString( port ) );
}

void TcpClient::connect( const string& host, const string& protocol )
{
	mHost		= host;
	mProtocol	= protocol;
	
	tcp::resolver::query query( mHost, mProtocol );
	tcp::resolver resolver( mIoService );
	resolver.async_resolve( query, boost::bind( &TcpClient::onResolve, this,
											   boost::asio::placeholders::error,
											   boost::asio::placeholders::iterator ) );
	mIoService.reset();
	mIoService.run();
}

void TcpClient::disconnect()
{
	if ( mSocket && mSocket->is_open() ) {
		mSocket->close();
	}
}

void TcpClient::read()
{
	boost::asio::async_read( *mSocket, mResponse, boost::asio::transfer_at_least( 1 ),
							boost::bind( &TcpClient::onRead, this,
										boost::asio::placeholders::error,
										boost::asio::placeholders::bytes_transferred ) );
}

void TcpClient::read( const std::string& until )
{
	boost::asio::async_read_until( *mSocket, mResponse, until,
							boost::bind( &TcpClient::onRead, this,
										boost::asio::placeholders::error,
										boost::asio::placeholders::bytes_transferred ) );
}

void TcpClient::read( size_t bufferSize )
{
	mSocket->async_read_some( mResponse.prepare( bufferSize ), boost::bind( &TcpClient::onRead, this,
																		   boost::asio::placeholders::error,
																		   boost::asio::placeholders::bytes_transferred ) );
}

void TcpClient::write( const Buffer& buffer )
{
	ostream stream( &mRequest );
	stream.write( (const char*)buffer.getData(), buffer.getDataSize() );
	boost::asio::async_write( *mSocket, mRequest,
							 boost::bind( &TcpClient::onWrite, this,
										 boost::asio::placeholders::error,
										 boost::asio::placeholders::bytes_transferred ) );
	mRequest.consume( mRequest.size() );
}

void TcpClient::onConnect( const boost::system::error_code& err )
{
	if ( err ) {
		mSignalError( err.message(), 0 );
	} else {
		mSignalConnect();
	}
}

void TcpClient::onResolve( const boost::system::error_code& err,
						  tcp::resolver::iterator iter )
{
	if ( err ) {
		mSignalError( err.message(), 0 );
	} else {
		mSignalResolve();
		boost::asio::async_connect( *mSocket, iter, boost::bind( &TcpClient::onConnect, this,
																boost::asio::placeholders::error ) );
	}
}
