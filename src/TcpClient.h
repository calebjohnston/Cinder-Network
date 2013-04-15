#pragma once

#include "Client.h"

typedef std::shared_ptr<class TcpClient>	TcpClientRef;

class TcpClient : public Client
{
public:
	static TcpClientRef				create();
	virtual ~TcpClient();
	
	virtual void					connect( const std::string& host = "localhost", const uint16_t port = 2000 );
protected:
	typedef std::shared_ptr<boost::asio::ip::tcp::socket>	TcpSocketRef;
	
	TcpClient();
	virtual void					onSend( const std::string& message,
										   const boost::system::error_code& error,
										   std::size_t bytesTransferred );

	virtual void					sendImpl( uint_fast8_t* buffer, size_t count );
	
	std::string						mBuffer;
	boost::asio::ip::tcp::endpoint	mEndpoint;
	boost::asio::io_service			mIoService;
	TcpSocketRef					mSocket;
};
