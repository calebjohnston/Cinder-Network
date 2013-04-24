#pragma once

#include "Client.h"

typedef std::shared_ptr<class TcpClient>				TcpClientRef;
typedef std::shared_ptr<boost::asio::ip::tcp::socket>	TcpSocketRef;

class TcpClient : public Client
{
public:
	static TcpClientRef				create( boost::asio::io_service& io );
	~TcpClient();
	
	virtual void					connect( const std::string& host = "localhost", uint16_t port = 2000 );
	void							disconnect();
	
	TcpSocketRef					getSocket() const;
	
	virtual void					read();
	virtual void					read( size_t bufferSize );
	virtual void					read( const std::string& delimiter );
	virtual void					write( const ci::Buffer& buffer );
protected:
									TcpClient( boost::asio::io_service& io );
	
	boost::asio::ip::tcp::endpoint	mEndpoint;
	TcpSocketRef					mSocket;
};
