#include "Client.h"

using namespace ci;
using namespace std;

Client::Client( boost::asio::io_service& io )
: Connection( io ), mHost( "" )
{
}

const string& Client::getHost() const
{
	return mHost;
}

