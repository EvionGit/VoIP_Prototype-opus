#include <stream/net_stream_audio_in.h>

namespace stream
{
	NetStreamAudioIn::NetStreamAudioIn(): jb(0)
	{
		
	}

	int64_t NetStreamAudioIn::stream_read(void* tobuffer, int64_t buffersize, int64_t readamount)
	{
		
		if (jb)
		{
			
			AudioPacket ap;
			int stat = 0;
			if((stat = jb->pop(ap)) == JSUCCESS)
			{
			
				memcpy(tobuffer, ap.data, ap.size);
				return ap.size;
			}
			else if (stat == JLOSTPACKET)
			{
				/* lost packet mark for opus-decoder*/
				return -2;
			}
			else if (stat == JBUFFERING ||  stat == JNODATA)
			{
				/* buffering wait mark for non-blocking data reader*/
				return -3;
			}
	
		}
		
		return 0;

	}

	int64_t NetStreamAudioIn::stream_write(const void* frombuffer, int64_t writesize)
	{
		
		if (jb)
		{
			AudioPacket ap;
			ap.unpack((char*)frombuffer,(int)writesize);

			/* arrived time */
			Ttimepoint arr = std::chrono::high_resolution_clock::now();

			return jb->push(ap, arr);
		}

		return 0;
	 
	}

	void NetStreamAudioIn::set_jitter_buffer(jbuf::JitterBuffer* jitter_buffer)
	{
	
		if(jb)
		{
			delete jb;	
		}
		jb = jitter_buffer;
	}

	void NetStreamAudioIn::reset_jitter_buffer()
	{
		if (jb)
		{
			jb->reset_jitter_buffer();
		}
			
	}

	
}