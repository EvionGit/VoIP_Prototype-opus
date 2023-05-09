#ifndef ACONFIG_PACKET_H
#define ACONFIG_PACKET_H

#include <stdint.h>
#include <string.h>
#include <ops/opus_base.h>

#define AUDIO_KEY 3140035
#define CONF_CONNECTION_TYPE 1
#define CONF_ABORTING_TYPE 2
#define CONF_BUSY_TYPE 3
#define CONF_ACCEPT_TYPE 4
#define CONF_RECONF_TYPE 5


namespace pack
{
	class AudioConfigPacket
	{
	public:

		int8_t packet_type;
		uint32_t key;
		ops::OPUS_SAMPLES_RATE samples_rate;
		uint16_t channels;
		uint16_t data_size_ms;

		int pack(char* buffer,int len)
		{
			if (len >= 13)
			{
				*(int8_t*)buffer = packet_type;
				*(uint32_t*)(buffer + 1) = htonl(key);
				*(uint32_t*)(buffer + 5) = htonl(samples_rate);
				*(uint16_t*)(buffer + 9) = htons(channels);
				*(uint16_t*)(buffer + 11) = htons(data_size_ms);

				return 13;
			}
			

			return 0;
		}

		int unpack(char* buffer, int len)
		{
			if (len >= 13)
			{
				packet_type = *(int8_t*)buffer;
				key = ntohl(*(uint32_t*)(buffer + 1));
				samples_rate = (ops::OPUS_SAMPLES_RATE)(ntohl(*(uint32_t*)(buffer + 5)));
				channels = ntohs(*(uint16_t*)(buffer + 9));
				data_size_ms = ntohs(*(uint16_t*)(buffer + 11));

				return 13;
			}
			
			return 0;
		}
	
	};
}

#endif