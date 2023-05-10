#ifndef TCP_SERVER_SOCKET_HEADER_H
#define TCP_SERVER_SOCKET_HEADER_H

#include <wsock/base_definition.h>
#include <wsock/addr_struct.h>
#include <wsock/socket.h>
#include <wsock/tcp_socket.h>

#define TCP_QUEUE_SIZE 5

/* Base class of udp-socket */
namespace wsock
{
	class tcpServerSocket;

	class tcpServerSocket : public Socket
	{
	public:
		/* Initialize UDP socket on the LOCAL_INTERFACE.
		By default, the universal address and dynamic port
		of the AF_INET family is selected.

		You can get the IP and PORT after by calling the _get_self_addr() function.*/
		tcpServerSocket(const char* local_interface,
			const char* port,
			int family = AF_INET);
	public:
		tcpSocket _accept();
		
	private:
		/* set socket as passive */
		int _listen(int queue_size);

	};
}



#endif
