#ifndef AUDIO_PACKET_H
#define AUDIO_PACKET_H

#include <stdint.h>
#include <string.h>

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


	/*public:
		AudioPacket() : id(0),size(0),data_in_ms(0),timestamp(0),data(0) {};
		AudioPacket(const AudioPacket& from)
		{
			id = from.id;
			size = from.size;
			data_in_ms = from.data_in_ms;
			timestamp = from.timestamp;
			data = from.data;
		}
		AudioPacket(AudioPacket&& from) noexcept
		{
			id = from.id;
			size = from.size;
			data_in_ms = from.data_in_ms;
			timestamp = from.timestamp;
			data = from.data;

			from.data = 0;
		}
		AudioPacket& operator=(const AudioPacket& from)
		{
			id = from.id;
			size = from.size;
			data_in_ms = from.data_in_ms;
			timestamp = from.timestamp;
			data = from.data;
			return *this;
		}*/

	};
}

#endif