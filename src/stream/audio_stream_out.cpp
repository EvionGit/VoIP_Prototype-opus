
#include <stream/audio_stream_out.h>

namespace stream
{
	AudioStreamOut::AudioStreamOut() : s(0), e(0), size(0)
	{
		memset(audiobuffer, 0, AUDIOBUFF);
	}

	AudioStreamOut::~AudioStreamOut() {}

	size_t AudioStreamOut::stream_read(void* tobuffer, size_t buffersize, size_t readamount)
	{
		while (1)
		{
			mtx.lock();
			/* waiting until audiobuffer != 0 */
			if (!size)
			{
				
				mtx.unlock();

				/* give time to CPU */
				std::this_thread::sleep_for(std::chrono::milliseconds(20));
				continue;
			}

			/* since we have a cyclic buffer, we have to check
			   whether to reach the edge of the array and
			   whether we need to jump to the beginning of the buffer
			*/

			/* if the requested data is placed without transition */
			else if (AUDIOBUFF - s >= readamount)
			{

				memcpy(tobuffer, audiobuffer + s, readamount);
				s += readamount;
				size -= readamount;

			}

			/* if not: need to move at the beginning and take last part of data */
			else
			{
				int toread = AUDIOBUFF - s;
				memcpy(tobuffer, audiobuffer + s, toread);
				memcpy(tobuffer, audiobuffer, readamount - toread);

				s = readamount - toread;
				size -= readamount;

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
			mtx.unlock();
			return readamount;
		}



	}

	size_t AudioStreamOut::stream_write(const void* frombuffer, size_t writesize)
	{

		/* wait while App reading data */
		while (1)
		{
			mtx.lock();
			if (AUDIOBUFF - size < writesize)
			{
				mtx.unlock();

				/* give time to CPU */
				std::this_thread::sleep_for(std::chrono::milliseconds(50));
				continue;
			}
			
			break;
		}
		

		/*
			check can we place data without trasition at the beginning
			( look at the comment in STREAM_READER method -> )
		*/

		if ((AUDIOBUFF - e) >= writesize)
		{
			memcpy(audiobuffer + e, frombuffer, writesize);
			e += writesize;
			size += writesize;

		}

		/* if not: need to move at the beginning and placing last part of data */
		else
		{
			size_t length = AUDIOBUFF - e;

			memcpy(audiobuffer + e, frombuffer, length);
			memcpy(audiobuffer, (char*)frombuffer + length, writesize - length);

			e = writesize - length;
			if (s < e) s = e;

			size += writesize;

		}

		mtx.unlock();
		return writesize;

	}
}