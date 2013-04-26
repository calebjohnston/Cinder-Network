#pragma once

#include "Client.h"

typedef std::shared_ptr<class TcpClient>				TcpClientRef;
typedef std::shared_ptr<boost::asio::ip::tcp::socket>	TcpSocketRef;

class TcpClient : public Client
{
public:
	static TcpClientRef	create( boost::asio::io_service& io );
	~TcpClient();
	
	void				connect( const std::string& host, uint16_t port );
	void				connect( const std::string& host, const std::string& protocol );
	void				disconnect();
	
	void				read();
	void				read( const std::string& until );
	void				read( size_t bufferSize );
	
	void				write( const ci::Buffer& buffer );
protected:
	TcpClient( boost::asio::io_service& io );
	
	void				onConnect( const boost::system::error_code& err );
	void				onResolve( const boost::system::error_code& err,
								  boost::asio::ip::tcp::resolver::iterator iter );
	
	TcpSocketRef		mSocket;
};
