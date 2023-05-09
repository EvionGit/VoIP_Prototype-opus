
#include "../include/wsock/addr_struct.h"

namespace wsock
{
	addr::addr()
	{	
		ssize = sizeof(saddr);
		memset(&saddr, 0, sizeof(saddr));
	}

	addr::addr(const SOCKADDR* sockadd, int size)
	{
		memcpy(&saddr, sockadd, size);
		ssize = size;

	}

	addr::addr(std::string host, std::string port, int family)
	{
		
		/* Returning code of getaddrinfo */
		INT r;

		/* Pointer to addrinfo struct */
		PADDRINFOA addrinfo;

		/* Hints for UDP passive socket */
		ADDRINFOA hints;
		memset(&hints, 0, sizeof(hints));
		hints.ai_family = family;


		/* Get all of the appropriate interfaces with HINTS */
		if ((r = getaddrinfo((PCSTR)host.c_str(), (PCSTR)port.c_str(), &hints, &addrinfo)) != 0)
		{
			lasterror = r;
			throw std::exception("<addr initialize error> (getaddrinfo error)");
		}

		if (addrinfo)
		{
			memcpy(&saddr, addrinfo->ai_addr, addrinfo->ai_addrlen);
			ssize = addrinfo->ai_addrlen;
			
		}
		else
		{
			freeaddrinfo(addrinfo);
			throw std::runtime_error("<No addresses> (DNS error)");
		}

		/* Clear the addrinfo list */
		freeaddrinfo(addrinfo);

	
	}

	const SOCKADDR_STORAGE& addr::_get_saddr()
	{
		return saddr;
	}

	uint16_t addr::_get_port()
	{
		return ntohs(*rcast(u_short*,rcast(SOCKADDR*, &saddr)->sa_data));
	}

	std::string addr::_get_straddr()
	{
		char sbuf[46];
		memset(sbuf, 0, 46);
		switch(saddr.ss_family)
		{
		case AF_INET:
			inet_ntop(saddr.ss_family, &((SOCKADDR_IN*)&saddr)->sin_addr, (PSTR)sbuf, 46);
			break;
		case AF_INET6:
			inet_ntop(saddr.ss_family, &((SOCKADDR_IN6*)&saddr)->sin6_addr, (PSTR)sbuf, 46);
			break;
		default:
			break;

		}
		
		return std::string(sbuf);
	}

	size_t addr::_get_size()
	{
		return ssize;
	}

	ADDRESS_FAMILY addr::_get_family()
	{
		return saddr.ss_family;
	}

	/* static */
	std::vector<std::string> addr::get_available_interfaces(int family)
	{
		/* Returning code of getaddrinfo */
		INT r;
		std::vector<std::string> interfaces;
		interfaces.push_back("0.0.0.0");
		interfaces.push_back("localhost");
		
		/* Pointer to addrinfo struct */
		PADDRINFOA addrinfo,ptrinfo;

		/* Hints for UDP passive socket */
		ADDRINFOA hints;
		memset(&hints, 0, sizeof(hints));
		hints.ai_family = family;


		/* Get all of the appropriate interfaces with HINTS */
		if ((r = getaddrinfo((PCSTR)"", (PCSTR)"5555", &hints, &addrinfo)) != 0)
		{
			lasterror = r;
			throw std::exception("<addr initialize error> (getaddrinfo error)");
		}

		if (addrinfo)
		{
			int offset = family == AF_INET ? 2 : 6;
			ptrinfo = addrinfo;
			while(ptrinfo)
			{
				char addr_buf[46];
				inet_ntop(family, ptrinfo->ai_addr->sa_data+offset, addr_buf, 46);
				interfaces.push_back(addr_buf);
				ptrinfo = ptrinfo->ai_next;
			}

		}
		else
		{
			freeaddrinfo(addrinfo);
			throw std::runtime_error("<No addresses> (DNS error)");
		}

		/* Clear the addrinfo list */
		freeaddrinfo(addrinfo);

		return interfaces;
	}
}