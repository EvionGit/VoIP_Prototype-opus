#ifndef ACONFIG_PACKET_H
#define ACONFIG_PACKET_H

#include <stdint.h>
#include <string.h>

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
		uint32_t samples_rate;
		uint16_t channels;
		uint16_t data_size_ms;
		

	};
}

#endif