#include <stream/net_stream_audio_in.h>

namespace stream
{
	NetStreamAudioIn::NetStreamAudioIn(): jb(0)
	{
		
	}

	size_t NetStreamAudioIn::stream_read(void* tobuffer, size_t buffersize, size_t readamount) 
	{
		if (jb)
		{
			AudioPacket ap;
			int stat = 0;
			while ((stat = jb->pop(ap)) != JSUCCESS)
			{
				if (stat == JLOSTPACKET)
				{
					//printf("--LOSTPACKET\n");
					return -2;
				}

				//std::this_thread::sleep_for(std::chrono::milliseconds(20));
			}


			memcpy(tobuffer, ap.data, ap.size);
			return ap.size;
		}
		
		return 0;

	}
	size_t NetStreamAudioIn::stream_write(const void* frombuffer, size_t writesize)
	{

		if (jb)
		{
			AudioPacket* ap = (AudioPacket*)frombuffer;

			char* payload = (char*)frombuffer + sizeof(AudioPacket);
			ap->data = payload;

			Ttimepoint arr = std::chrono::high_resolution_clock::now();

			return jb->push(*ap, arr);
		}

		return 0;
	 
	}

	void NetStreamAudioIn::set_jitter_buffer(jbuf::JitterBuffer* jitter_buffer)
	{
		if(jb)
		{
			delete jb;
			jb = jitter_buffer;
		}
	}

	void NetStreamAudioIn::reset_jitter_buffer()
	{
		if (jb)
		{
			jb->reset_jitter_buffer();
		}
			
	}

	
}