
#include <stream/audio_stream_out.h>

namespace stream
{
	AudioStreamOut::AudioStreamOut() : s(0), e(0), size(0)
	{
		memset(audiobuffer, 0, AUDIOBUFFOUT);
		memset(buff, 0, LISTENERBUFFER);
		initialize(2, 48000);
	}

	AudioStreamOut::~AudioStreamOut() {}

	size_t AudioStreamOut::stream_read(void* tobuffer, size_t buffersize, size_t readamount)
	{
		while (1)
		{

			mtx.lock();
			/* waiting until audiobuffer != 0 */
			if (size < readamount)
			{

				mtx.unlock();

				/* give time to CPU */
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
				continue;
			}

			/* since we have a cyclic buffer, we have to check
			   whether to reach the edge of the array and
			   whether we need to jump to the beginning of the buffer
			*/

			int readable = readamount > size ? size : readamount;
			/* if the requested data is placed without transition */
			if (AUDIOBUFFOUT - s >= readable)
			{

				memcpy(tobuffer, audiobuffer + s, readable);
				s += readable;
				size -= readable;

			}

			/* if not: need to move at the beginning and take last part of data */
			else
			{
				int toread = AUDIOBUFFOUT - s;
				memcpy(tobuffer, audiobuffer + s, toread);
				memcpy(tobuffer, audiobuffer, readable - toread);

				s = readable - toread;
				size -= readable;

			}

			/*
			   if all data is given we can reset the positions
			   and start from the beginning of the buffer
			*/

			if (!size)
			{
				e = 0;
				s = 0;
			}

			uint64_t t = std::chrono::high_resolution_clock::now().time_since_epoch().count();


			mtx.unlock();
			return readable;
		}



	}

	size_t AudioStreamOut::stream_write(const void* frombuffer, size_t writesize)
	{

		/* wait while App reading data */
		while (1)
		{
			mtx.lock();
			if (AUDIOBUFFOUT - size < writesize)
			{

				mtx.unlock();

				/* give time to CPU */
				//std::this_thread::sleep_for(std::chrono::milliseconds(10));
				continue;
			}

			break;
		}


		/*
			check can we place data without trasition at the beginning
			( look at the comment in STREAM_READER method -> )
		*/

		if ((AUDIOBUFFOUT - e) >= writesize)
		{
			memcpy(audiobuffer + e, frombuffer, writesize);
			e += writesize;
			size += writesize;

		}

		/* if not: need to move at the beginning and placing last part of data */
		else
		{
			size_t length = AUDIOBUFFOUT - e;

			memcpy(audiobuffer + e, frombuffer, length);
			memcpy(audiobuffer, (char*)frombuffer + length, writesize - length);

			e = writesize - length;
			if (s < e) s = e;

			size += writesize;

		}

		mtx.unlock();
		return writesize;

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

		////int c = stream_read((void*)data.samples, LISTENERBUFFER, LISTENERBUFFER);
		//int c = stream_read(buff, LISTENERBUFFER, LISTENERBUFFER);
		//if (!c)
		//	return false;

		//data.samples = (sf::Int16*)buff;
		//data.sampleCount = c / 2;

		//return true;
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