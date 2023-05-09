#ifndef AUDIO_PACKET_H
#define AUDIO_PACKET_H

#include <stdint.h>
#include <string.h>
#include <wsock/wsa_init.h>

#define AUDIO_PACKET_TYPE 55

namespace pack
{
	class AudioPacket
	{
	public:

		int8_t packet_type;
		uint32_t id;
		uint16_t size;
		int32_t data_in_ms;
		int64_t timestamp;
		char* data;

		int pack(char* buffer,int len)
		{
			*(int8_t*)buffer = packet_type;
			*(uint32_t*)(buffer + 1) = htonl(id);
			*(uint16_t*)(buffer + 5) = htons(size);
			*(int32_t*)(buffer + 7) = htonl(data_in_ms);
			*(int64_t*)(buffer + 11) = htonll(timestamp);
			memcpy(buffer + 19, data, size);

			return 19 + size;
			
		}

		int unpack(char* buffer, int len)
		{
			packet_type = *(int8_t*)buffer;
			id = ntohl(*(uint32_t*)(buffer + 1));
			size = ntohs(*(uint16_t*)(buffer + 5));
			data_in_ms = ntohl(*(int32_t*)(buffer + 7));
			timestamp = ntohll(*(int64_t*)(buffer + 11));
			data = buffer + 19;

			return 19 + size;
		}

	};

}

#endif