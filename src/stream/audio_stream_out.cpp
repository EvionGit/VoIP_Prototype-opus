
#include <stream/audio_stream_out.h>

namespace stream
{
	AudioStreamOut::AudioStreamOut()
	{
		memset(sampbuf, 0, LISTENERBUFFER);
	}

	AudioStreamOut::~AudioStreamOut()
	{
		_stop();
	}

	int64_t AudioStreamOut::stream_read(void* tobuffer, int64_t buffersize, int64_t readamount)
	{
		/* stream read implemented in onGetData */
		return 0;
	}

	int64_t AudioStreamOut::stream_write(const void* frombuffer, int64_t writesize)
	{
		/* stream write implemented in onGetData */
		return 0;
	}

	bool AudioStreamOut::onGetData(Chunk& data)
	{
		if (dec)
		{
		
			int s_in_bytes;
			while ((s_in_bytes = dec->decode_to(sampbuf)) == 0 && reading) {}
			if(!s_in_bytes && !reading)
			{
				
				return false;
			}
			
				

			data.samples = sampbuf;
			data.sampleCount = s_in_bytes / 2;
			return true;
		}
		return false;
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

	void AudioStreamOut::_play()
	{
		reading = true;
		this->play();
	}

	void AudioStreamOut::_stop()
	{
		reading = false;
		this->stop();
	}

	void AudioStreamOut::set_volume(int volume)
	{
		setVolume((float)volume);
	}
}