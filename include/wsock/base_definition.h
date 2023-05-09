#ifndef BASE_DEFINITION_H
#define BASE_DEFINITION_H

#include <WinSock2.h> // win.socket API
#include <ws2tcpip.h>
#include <string> 
#include <vector>
#include <stdio.h>
#include <stdexcept>
#include <wsock/wsa_init.h>

#pragma comment(lib,"ws2_32.lib")

#define DYNAMIC_PORT ""
#define DEFAULT_PORT "5555"
#define DEFAULT_IP4_ADDRESS "0.0.0.0"
#define DEFAULT_IP6_ADDRESS "::"
#define MAXBUFSIZE 1280

#define SA SOCKADDR

#define rcast(type,a) reinterpret_cast<type>(a)
#define scast(type,a) static_cast<type>(a)

static int lasterror = 0;




#endif