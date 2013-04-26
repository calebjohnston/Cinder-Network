#pragma once

#include "Client.h"

typedef std::shared_ptr<class UdpClient>				UdpClientRef;
typedef std::shared_ptr<boost::asio::ip::udp::socket>	UdpSocketRef;

class UdpClient : public Client
{
public:
	static UdpClientRef	create( boost::asio::io_service& io );
	
	void				connect( const std::string& host, uint16_t port );
	void				connect( const std::string& host, const std::string& protocol );
	void				disconnect();
	
	void				read();
	
	void				write( const ci::Buffer& buffer );
protected:
	UdpClient( boost::asio::io_service& io );
	
	void				onConnect( const boost::system::error_code& err );
	void				onResolve( const boost::system::error_code& err,
								  boost::asio::ip::udp::resolver::iterator iter );
	
	UdpSocketRef		mSocket;
};
