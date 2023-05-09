#ifndef ADDR_STRUCT_H
#define ADDR_STRUCT_H

#include <wsock/base_definition.h>

namespace wsock
{
	/* */
	struct addr;
	
	struct addr
	{
		friend class Socket;
		friend class udpSocket;

	private:
		SOCKADDR_STORAGE saddr;
		size_t ssize;

	public:
		addr();
		addr(const SOCKADDR* sockadd, int size);
		addr(std::string host,std::string port,int family = AF_INET);
		const SOCKADDR_STORAGE& _get_saddr();
		uint16_t _get_port();
		std::string _get_straddr();
		ADDRESS_FAMILY _get_family();
		size_t _get_size();

		static std::vector<std::string> get_available_interfaces(int family);
	};
}


#endif