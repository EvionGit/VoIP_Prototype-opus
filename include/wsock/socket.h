#ifndef SOCKET_H
#define SOCKET_H

#include <wsock/base_definition.h>
#include <wsock/addr_struct.h>

/* Base socket class */
namespace wsock
{
	class Socket;

	class Socket
	{

	public:
		/* Return the socket descriptor */
		SOCKET _get_socket();

		/* Return the size of the receive buffer */
		DWORD _get_receive_buf();

		/* Set the size of the receive buffer */
		int _set_receive_buf(DWORD bufsize);

		/* Return the size of the send buffer */
		DWORD _get_send_buf();

		/* Set the size of the send buffer */
		int _set_send_buf(DWORD bufsize);

		/* Return the amount of timeout to receive function */
		DWORD _get_receive_timeout();

		/* Set the amount of timeout to receive function */
		int _set_recive_timeout(DWORD millisec);

		/* Return the amount of timeout to send function */
		DWORD _get_send_timeout();

		/* Set the amount of timeout to send function */
		int _set_send_timeout(DWORD millisec);

		/* Set the socket to NON-BLOCK mode */
		int _set_nonblock();

		/* Set the socket to BLOCK mode */
		int _set_block();

		/* Set additional options for the socket.
		   Check the WinSock documentation (socket-option) */
		int _set_sockopt(int level,int optname,const char* optval,int optlen);

		/* Get additional options for the socket.
		   Check the WinSock documentation (socket-option)  */
		int _get_sockopt(int level, int optname, char* optval, int* optlen);

		/* Return the address structure of the local interface bound of the socket */
		addr _get_self_addr();

		/* shutdown channels of socket */
		int _shutdown(int how = SD_BOTH);

		/* close socket */
		int _close();
		
	protected:

		/* Bind the address structure with socket */
		int _bind();
		
	protected:
		Socket();
		~Socket();
		
	protected:
		SOCKET sock;
		addr self_addr;

	};
}



#endif
