#pragma once

#include "Client.h"

typedef std::shared_ptr<class FtpClient>	FtpClientRef;

class FtpClient : public Client
{
public:
	static FtpClientRef	create();
	virtual ~FtpClient();
	
	virtual void			connect( const std::string& host = "localhost", const uint16_t port = 21 );
protected:
	FtpClient();
	
	virtual void			sendImpl( uint_fast8_t* buffer, size_t count );
};
