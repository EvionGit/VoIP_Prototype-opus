#ifndef AUDIO_STREAM_OUT_HEADER_H
#define AUDIO_STREAM_OUT_HEADER_H

#include "stream.h"
#include <SFML/Audio.hpp>

/*  default buf size for 10 raw audio packets with monophonic 16-bit 44100hz splitted into 30-40ms */
#define AUDIOBUFFOUT 32768
#define LISTENERBUFFER 2048
namespace stream
{
	class AudioStreamOut : public Stream, public sf::SoundStream
	{
	private:
		std::mutex mtx;

		char buff[LISTENERBUFFER];
		/* in this case the buffer is cyclical */
		char audiobuffer[AUDIOBUFFOUT];
		uint32_t s, e; // start and end point
		uint32_t size;

		

	public:
		AudioStreamOut();
		~AudioStreamOut();

	public:
		/* App reads data from the buffer */
		virtual size_t stream_read(void* tobuffer, size_t buffersize, size_t readamount) override;

		/* Decoder writes data to the buffer */
		virtual size_t stream_write(const void* frombuffer, size_t writesize) override;

		/**/
		virtual bool onGetData(Chunk& data) override;

		/**/
		virtual void onSeek(sf::Time timeOffset) override;

		void set_listener_conf(uint32_t rate, uint8_t channels);


	};
}

#endif