#include <wsock/tcp_socket.h>

namespace wsock
{
	tcpSocket::tcpSocket(const char* local_interface, const char* port, int family) : isConnected(false)
	{
		/* Returning code of getaddrinfo */
		INT r;

		/* Pointer to addrinfo struct */
		PADDRINFOA addrinfo, ptr_addrinfo;

		/* Hints for TCP passive socket */
		ADDRINFOA hints;
		memset(&hints, 0, sizeof(hints));
		hints.ai_family = family;
		hints.ai_socktype = SOCK_STREAM;



		/* Get all of the appropriate interfaces with HINTS */
		if ((r = getaddrinfo((PCSTR)local_interface, (PCSTR)port, &hints, &addrinfo)) != 0)
		{
			lasterror = r;
			throw std::runtime_error("<tcp initialize error> (getaddrinfo error)");
		}

		/* Choose the first suitable interface */
		ptr_addrinfo = addrinfo;
		while (ptr_addrinfo)
		{
			if ((sock = socket(ptr_addrinfo->ai_family,
				ptr_addrinfo->ai_socktype,
				ptr_addrinfo->ai_protocol)) != INVALID_SOCKET)
			{

				/* Fill udpServer addr struct */
				self_addr = addr(ptr_addrinfo->ai_addr, scast(int, ptr_addrinfo->ai_addrlen));

				if (_bind() != SOCKET_ERROR)
				{
					break;
				}

			};
			closesocket(sock);
			sock = 0;
			ptr_addrinfo = ptr_addrinfo->ai_next;


		}

		/* Clear the addrinfo list */
		freeaddrinfo(addrinfo);
		if (!sock)
		{
			throw std::runtime_error("<tcp initialize error> (suitable interface not found)");
		}
	}

	tcpSocket::tcpSocket(SOCKET s,addr& remote_addr)
	{
		sock = s;
		self_addr = remote_addr;
		isConnected = true;
	}

	int tcpSocket::_connect(const addr& remote)
	{
		int r;
		if((r = connect(sock, (SA*)&remote.saddr, (int)remote.ssize)) == SOCKET_ERROR)
		{
			lasterror = WSAGetLastError();
			return SOCKET_ERROR;
		};

		isConnected = true;
		return 0;
	}

	int tcpSocket::_send(const void* frombuf, int bufsize, int flag)
	{
		if (!isConnected)
			return WSAENOTCONN;

		int r;
		if ((r = send(
			sock,
			rcast(const char*, frombuf),
			bufsize,
			flag
			)) == SOCKET_ERROR)
		{
			lasterror = WSAGetLastError();
			return SOCKET_ERROR;
		}

		return r;
	}

	int tcpSocket::_recv(void* tobuf, int bufsize, int flag)
	{
		if (!isConnected)
			return WSAENOTCONN;

		int r;

		if ((r = recv(
			sock,
			rcast(char*, tobuf),
			bufsize,
			flag
			)) == SOCKET_ERROR)
		{
			lasterror = WSAGetLastError();
			return SOCKET_ERROR;
		}

		return r;
	}
}