#pragma once

#include "Client.h"

typedef std::shared_ptr<class UdpClient>				UdpClientRef;
typedef std::shared_ptr<boost::asio::ip::udp::socket>	UdpSocketRef;

class UdpClient : public Client
{
public:
	static UdpClientRef				create( boost::asio::io_service& io );
	~UdpClient();
	
	virtual void					connect( const std::string& host = "localhost", uint16_t port = 2000 );
	void							disconnect();
	
	virtual void					read();
	virtual void					read( size_t bufferSize );
	virtual void					read( const std::string& delimiter );
	virtual void					write( const ci::Buffer& buffer );
protected:
									UdpClient( boost::asio::io_service& io );
	
	boost::asio::ip::udp::endpoint	mEndpoint;
	boost::asio::io_service			mIoService;
	UdpSocketRef					mSocket;

};
