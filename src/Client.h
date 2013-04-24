#pragma once

#include "Connection.h"

class Client : public Connection
{
public:
	//! Connects socket to remote \a host on port number \a port.
	virtual void		connect( const std::string &host, uint16_t port ) = 0;
	
	//! Returns host as string.
	const std::string&	getHost() const;
protected:
	explicit			Client( boost::asio::io_service& io );
	
	std::string			mHost;
};
