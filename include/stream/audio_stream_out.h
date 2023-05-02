#ifndef AUDIO_STREAM_OUT_HEADER_H
#define AUDIO_STREAM_OUT_HEADER_H

#include "stream.h"
#include <SFML/Audio.hpp>
#include <ops/decoder.h>


#define LISTENERBUFFER 2048

namespace stream
{
	class AudioStreamOut : public Stream, public sf::SoundStream
	{
	private:

		ops::Decoder* dec;
		int16_t sampbuf[LISTENERBUFFER];


	public:
		AudioStreamOut();
		~AudioStreamOut();

	public:
		/* stream plug */
		virtual size_t stream_read(void* tobuffer, size_t buffersize, size_t readamount) override;

		/* stream plug */
		virtual size_t stream_write(const void* frombuffer, size_t writesize) override;

		/**/
		virtual bool onGetData(Chunk& data) override;

		/**/
		virtual void onSeek(sf::Time timeOffset) override;

		void set_listener_conf(uint32_t rate, uint8_t channels);

		void set_decoder(ops::Decoder* dec);


	};
}

#endif