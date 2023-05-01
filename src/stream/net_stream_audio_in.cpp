#include <stream/net_stream_audio_in.h>

namespace stream
{
	NetStreamAudioIn::NetStreamAudioIn(jbuf::JitterBuffer* jitter_buffer)
	{
		jb = jitter_buffer;
	}

	size_t NetStreamAudioIn::stream_read(void* tobuffer, size_t buffersize, size_t readamount) 
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

	
		memcpy(tobuffer,ap.data, ap.size);
		return ap.size;

	}
	size_t NetStreamAudioIn::stream_write(const void* frombuffer, size_t writesize)
	{
		AudioPacket* ap = (AudioPacket*)frombuffer;
			
		char* payload = (char*)frombuffer + sizeof(AudioPacket);
		ap->data = payload;
	
		Ttimepoint arr = std::chrono::high_resolution_clock::now();
		
		return jb->push(*ap,arr);

		 
	}
}