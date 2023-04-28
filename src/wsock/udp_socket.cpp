#include "../include/wsock/udp_socket.h"


namespace wsock
{
	udpSocket::udpSocket(const char* local_interface, const char* port, int family)
	{

		/* Returning code of getaddrinfo */
		INT r;

		/* Pointer to addrinfo struct */
		PADDRINFOA addrinfo, ptr_addrinfo;

		/* Hints for UDP passive socket */
		ADDRINFOA hints;
		memset(&hints, 0, sizeof(hints));
		hints.ai_family = family;
		hints.ai_socktype = SOCK_DGRAM;



		/* Get all of the appropriate interfaces with HINTS */
		if ((r = getaddrinfo((PCSTR)local_interface, (PCSTR)port, &hints, &addrinfo)) != 0)
		{
			lasterror = r;
			throw std::runtime_error("<udpServer initialize error> (getaddrinfo error)");
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
				self_addr = addr(ptr_addrinfo->ai_addr, scast(int,ptr_addrinfo->ai_addrlen));
				
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
			throw std::runtime_error("<udpServer initialize error> (suitable interface not found)");
		}

	}

	int udpSocket::_sendto(const addr& remote, const void* frombuf, int bufsize, int flag)
	{
		int r;
		if ((r = sendto(
			sock,
			rcast(const char*, frombuf),
			bufsize,
			flag,
			(SA*)&remote.saddr,
			scast(int,remote.ssize))) == SOCKET_ERROR)
		{
			lasterror = WSAGetLastError();
			return SOCKET_ERROR;
		}

		return r;
	}

	int udpSocket::_recvfrom(addr& remote, void* tobuf, int bufsize, int flag)
	{
		int r;
		

		if ((r = recvfrom(
			sock,
			rcast(char*, tobuf),
			bufsize,
			flag,
			(SA*)&remote.saddr,
			rcast(int*,&(remote.ssize)))) == SOCKET_ERROR)
		{
			lasterror = WSAGetLastError();
			return SOCKET_ERROR;
		}

		return r;
	}



	
}


