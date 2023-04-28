#include "../include/wsock/socket.h"

namespace wsock
{
	
	Socket::Socket() : sock(0)
	{
		//static sckt_wsa::WSA_INIT wsa_init;
	}

	int Socket::_bind()
	{
		if (bind(sock, (SA*)&self_addr.saddr, scast(int, self_addr.ssize)) == SOCKET_ERROR)
		{		
			lasterror = WSAGetLastError();
			return SOCKET_ERROR;
		}

		getsockname(sock, (SA*)&self_addr.saddr, rcast(int*, &self_addr.ssize));
		return 0;
	}

	SOCKET Socket::_get_socket()
	{
		return sock;
	}

	Socket::~Socket()
	{
		closesocket(sock);
		
	}

	addr Socket::_get_self_addr()
	{
		return self_addr;
	}

	DWORD Socket::_get_receive_buf()
	{
		DWORD val;
		int sval = sizeof(val);
		if (getsockopt(sock, SOL_SOCKET, SO_RCVBUF, rcast(char*, &val), &sval) == SOCKET_ERROR)
			return SOCKET_ERROR;
		return val;
	}
	int Socket::_set_receive_buf(DWORD bufsize)
	{
		int sval = sizeof(bufsize);
		return setsockopt(sock, SOL_SOCKET, SO_RCVBUF, rcast(const char*, &bufsize), sval);
			
	}
	DWORD Socket::_get_send_buf()
	{
		DWORD val;
		int sval = sizeof(val);
		if (getsockopt(sock, SOL_SOCKET, SO_SNDBUF, rcast(char*, &val), &sval) == SOCKET_ERROR)
			return SOCKET_ERROR;
		return val;
	}
	int Socket::_set_send_buf(DWORD bufsize)
	{
		int sval = sizeof(bufsize);
		return setsockopt(sock, SOL_SOCKET, SO_SNDBUF, rcast(const char*, &bufsize), sval);
			 
	}

	DWORD Socket::_get_receive_timeout()
	{
		DWORD val;
		int sval = sizeof(val);
		if (getsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, rcast(char*, &val), &sval) == SOCKET_ERROR)
			return SOCKET_ERROR;
		return val;
	}
	int Socket::_set_recive_timeout(DWORD millisec)
	{
		int sval = sizeof(millisec);
		return setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, rcast(const char*, &millisec), sval);
			 
	}
	DWORD Socket::_get_send_timeout()
	{
		DWORD val;
		int sval = sizeof(val);
		if (getsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, rcast(char*, &val), &sval) == SOCKET_ERROR)
			return SOCKET_ERROR;
		return val;
	}
	int Socket::_set_send_timeout(DWORD millisec)
	{
		int sval = sizeof(millisec);
		return setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, rcast(const char*, &millisec), sval);
			 
	}

	int Socket::_set_nonblock()
	{
		u_long s = 1;
		return ioctlsocket(sock, FIONBIO, &s);
	}
	int Socket::_set_block()
	{
		u_long s = 0;
		return ioctlsocket(sock, FIONBIO, &s);
	}

	int Socket::_set_sockopt(int level, int optname, const char* optval, int optlen)
	{
		return setsockopt(sock, level, optname, optval, optlen);
	}

	int Socket::_get_sockopt(int level, int optname, char* optval, int* optlen)
	{
		return getsockopt(sock, level, optname, optval, optlen);
	}
}

