#ifndef TCP_CLIENT_SOCKET_HEADER_H
#define TCP_CLIENT_SOCKET_HEADER_H

#include <wsock/base_definition.h>
#include <wsock/addr_struct.h>
#include <wsock/socket.h>

/* Base class of udp-socket */
namespace wsock
{
	class tcpSocket;

	class tcpSocket : public Socket
	{
		friend class tcpServerSocket;

	private:
		bool isConnected;

	private:
		tcpSocket(SOCKET s,addr& remote_addr);

	public:
		/* Initialize UDP socket on the LOCAL_INTERFACE.
		By default, the universal address and dynamic port
		of the AF_INET family is selected.

		You can get the IP and PORT after by calling the _get_self_addr() function.*/
		tcpSocket(const char* local_interface = DEFAULT_IP4_ADDRESS,
			const char* port = DYNAMIC_PORT,
			int family = AF_INET);
	public:

		/* connect to remote tcp-socket*/
		int _connect(const addr& remote);

		/* Send BUFSIZE bytes from FROMBUF to REMOTE socket.
			You may also add the additional options with FLAG.*/
		int _send(const void* frombuf, int bufsize, int flag = 0);

		/* Receive BUFSIZE bytes to TOBUF from socket and fills
		 the REMOTE with the address structure of the sender.
			You may also add the additional options with FLAG.*/
		int _recv(void* tobuf, int bufsize, int flag = 0);



	};
}



#endif