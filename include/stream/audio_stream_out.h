#ifndef AUDIO_STREAM_OUT_HEADER_H
#define AUDIO_STREAM_OUT_HEADER_H

#include "stream.h"
#include <SFML/Audio.hpp>
#include <ops/decoder.h>


#define LISTENERBUFFER 2048

namespace stream
{
	class AudioStreamOut : public Stream, protected sf::SoundStream
	{
	private:
		bool reading;
		ops::Decoder* dec;
		int16_t sampbuf[LISTENERBUFFER];


	public:
		AudioStreamOut();
		~AudioStreamOut();

	public:
		/* stream plug */
		virtual int64_t stream_read(void* tobuffer, int64_t buffersize, int64_t readamount) override;

		/* stream plug */
		virtual int64_t stream_write(const void* frombuffer, int64_t writesize) override;

		void set_listener_conf(uint32_t rate, uint8_t channels);

		void set_decoder(ops::Decoder* dec);

		void _play();

		void _stop();

		void set_volume(int volume);

	private:
		/**/
		virtual bool onGetData(Chunk& data) override;

		/**/
		virtual void onSeek(sf::Time timeOffset) override;

	


	};
}

#endif