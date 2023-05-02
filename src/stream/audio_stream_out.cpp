
#include <stream/audio_stream_out.h>

namespace stream
{
	AudioStreamOut::AudioStreamOut()
	{
		memset(sampbuf, 0, LISTENERBUFFER);
	}

	AudioStreamOut::~AudioStreamOut() {}

	size_t AudioStreamOut::stream_read(void* tobuffer, size_t buffersize, size_t readamount)
	{
		return 0;
	}

	size_t AudioStreamOut::stream_write(const void* frombuffer, size_t writesize)
	{
		return 0;
	}

	bool AudioStreamOut::onGetData(Chunk& data)
	{
		if (dec)
		{
			int s_in_bytes = dec->decode_to(sampbuf);
			data.samples = sampbuf;
			data.sampleCount = s_in_bytes / 2;
			return true;
		}
	}

	void AudioStreamOut::onSeek(sf::Time timeOffset)
	{

	}

	void AudioStreamOut::set_listener_conf(uint32_t rate, uint8_t channels)
	{
		initialize(channels, rate);
	}

	void AudioStreamOut::set_decoder(ops::Decoder* dec)
	{
		this->dec = dec;
	}
}