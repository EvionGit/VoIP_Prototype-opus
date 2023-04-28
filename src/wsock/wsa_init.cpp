#include "../include/wsock/wsa_init.h"

namespace wsock
{
	WSA_INIT::WSA_INIT()
	{
		WSAData wsaData;
		if (WSAStartup(MAKEWORD(2, 1), &wsaData) != 0)
		{
			throw std::runtime_error("WSAStartup error");
		}
	}

	WSA_INIT::~WSA_INIT()
	{
		WSACleanup();
	}
}
