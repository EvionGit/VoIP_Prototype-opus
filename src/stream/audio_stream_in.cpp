
#include <stream/audio_stream_in.h>

namespace stream
{
	AudioStreamIn::AudioStreamIn() : s(0),e(0),size(0)
	{
		sf::Time t = sf::milliseconds(20);
		setProcessingInterval(t);
		memset(audiobuffer, 0, AUDIOBUFF);
	}

	AudioStreamIn::~AudioStreamIn()
	{
		
	}

	int64_t AudioStreamIn::stream_read(void* tobuffer, int64_t buffersize, int64_t readamount)
	{
		while(1)
		{
			mtx.lock();


			if(end_of_file)
			{
				mtx.unlock();
				return 0;
			}
			
			
	
			/* waiting until audiobuffer < 4096 bytes */
			if (size < readamount)
			{
				mtx.unlock();
			
				/* give a time to CPU */
				//std::this_thread::sleep_for(std::chrono::milliseconds(20));
				continue;
			}

			if (size < readamount)
			{
				size_t rs = size;
				memcpy(tobuffer, audiobuffer + s, size);
				s += size;
				size -= size;

				
				e = 0;
				s = 0;
				

				mtx.unlock();
				return rs;
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

	int64_t AudioStreamIn::stream_write(const void* frombuffer, int64_t writesize)
	{
		std::lock_guard<std::mutex> lock(mtx);
		//printf("GET: %zu - ",writesize);
		//printf("%zu\n", std::chrono::high_resolution_clock::now().time_since_epoch().count() / 1000000);
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
		
		return writesize;

	}

	bool AudioStreamIn::onProcessSamples(const sf::Int16* samples, size_t sampleCount)
	{
		printf("RECORDED\n");
		/*printf("%lli\n", (std::chrono::high_resolution_clock::now().time_since_epoch().count() - last) / 1000000);
		last = std::chrono::high_resolution_clock::now().time_since_epoch().count();*/
		stream_write(samples, sampleCount * 2);
		return true;
	};

	void AudioStreamIn::onStop()
	{
		end_of_file = true;
	}

	bool AudioStreamIn::onStart()
	{
		s = e = size = 0;
		end_of_file = false;
		return true;
	}
}