#pragma once

#include "Client.h"

typedef std::shared_ptr<class SmtpClient>	SmtpClientRef;

class SmtpClient : public Client
{
public:
	static SmtpClientRef	create();
	virtual ~SmtpClient();
	
	virtual void			connect( const std::string& host = "localhost", const uint16_t port = 25 );
protected:
	SmtpClient();
	
	virtual void			sendImpl( uint_fast8_t* buffer, size_t count );
};
