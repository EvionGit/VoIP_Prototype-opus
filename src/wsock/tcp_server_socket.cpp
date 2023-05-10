#include <wsock/tcp_server_socket.h>

namespace wsock
{
	tcpServerSocket::tcpServerSocket(const char* local_interface, const char* port, int family)
	{
		/* Returning code of getaddrinfo */
		INT r;

		/* Pointer to addrinfo struct */
		PADDRINFOA addrinfo, ptr_addrinfo;

		/* Hints for TCP passive socket */
		ADDRINFOA hints;
		memset(&hints, 0, sizeof(hints));
		hints.ai_flags = AI_PASSIVE;
		hints.ai_family = family;
		hints.ai_socktype = SOCK_STREAM;



		/* Get all of the appropriate interfaces with HINTS */
		if ((r = getaddrinfo((PCSTR)local_interface, (PCSTR)port, &hints, &addrinfo)) != 0)
		{
			lasterror = r;
			throw std::runtime_error("<tcpServer initialize error> (getaddrinfo error)");
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
					if (_listen(TCP_QUEUE_SIZE) != SOCKET_ERROR)
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
			throw std::runtime_error("<tcpServer initialize error> (suitable interface not found)");
		}
	}

	int tcpServerSocket::_listen(int queue_size)
	{
		if (listen(sock, queue_size) == SOCKET_ERROR)
		{
			lasterror = WSAGetLastError();
			return SOCKET_ERROR;
		}

		return 0;
	}

	tcpSocket tcpServerSocket::_accept()
	{
		SOCKADDR_STORAGE r_addr;
		int len_addr = 0;
		SOCKET s;
		
		if((s = accept(sock, (SA*)&r_addr, &len_addr)) == INVALID_SOCKET)
		{
			lasterror = WSAGetLastError();
		}

		addr remote_a((SA*)&r_addr, len_addr);
		return tcpSocket(s, remote_a);
	}
}