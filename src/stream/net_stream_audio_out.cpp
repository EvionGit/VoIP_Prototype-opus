
#include <stream/net_stream_audio_out.h>

namespace stream
{
	NetStreamAudioOut::NetStreamAudioOut(wsock::udpSocket& local_addr, wsock::addr remote_addr, uint32_t rate)
		: local(local_addr), remote(remote_addr), jb(0), sample_rate(rate)
	{
		packet_headers.id = 0;
		packet_headers.timestamp = 0;

	}

	NetStreamAudioOut::~NetStreamAudioOut() {}

	int64_t NetStreamAudioOut::stream_read(void* tobuffer, int64_t buffersize, int64_t readamount)
	{
		pack::AudioPacket ap;
		if (jb->pop(ap) == JSUCCESS)
		{
			ap.timestamp = std::chrono::high_resolution_clock::now().time_since_epoch().count();
			memcpy(sendbuffer, &ap, sizeof(ap));
			memcpy(sendbuffer + sizeof(ap), ap.data, ap.size);

			//printf("SEND %lli\n", (ap.timestamp-last) / 1000000);
			last = ap.timestamp;
			local._sendto(remote, sendbuffer, sizeof(ap) + ap.size, 0);
		}


		return 1;
	}

	int64_t NetStreamAudioOut::stream_write(const void* frombuffer, int64_t writesize)
	{
		packet_headers.packet_type = AUDIO_PACKET_TYPE;
		packet_headers.id++;
		packet_headers.size = writesize;
		packet_headers.data_in_ms = 20;
		packet_headers.timestamp = 0;
		packet_headers.data = (char*)frombuffer;

		if (jb)
		{


			while (jb->push(packet_headers, std::chrono::high_resolution_clock::now()) == JFULLSTACK)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(20));
			}
		}
		else
		{
			packet_headers.timestamp = std::chrono::high_resolution_clock::now().time_since_epoch().count();
			memcpy(sendbuffer, &packet_headers, sizeof(packet_headers));
			memcpy(sendbuffer + sizeof(packet_headers), packet_headers.data, packet_headers.size);

			/*printf("SEND %lli\n", (packet_headers.timestamp - last) / 1000000);
			last = packet_headers.timestamp;*/
			//printf("remote: %s\n",remote._get_straddr().c_str());
			local._sendto(remote, sendbuffer, sizeof(packet_headers) + packet_headers.size, 0);
		}

		return 1;


	}

	void NetStreamAudioOut::set_jitter_buffer(jbuf::JitterBuffer* jbuffer)
	{
		jb = jbuffer;
	}
}