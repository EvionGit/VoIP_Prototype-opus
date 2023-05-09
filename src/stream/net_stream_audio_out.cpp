
#include <stream/net_stream_audio_out.h>

namespace stream
{
	NetStreamAudioOut::NetStreamAudioOut(wsock::udpSocket& local_addr, wsock::addr remote_addr, uint32_t rate)
		: local(local_addr), remote(remote_addr)
	{
		p_id = 0;
	}

	NetStreamAudioOut::~NetStreamAudioOut() {}

	int64_t NetStreamAudioOut::stream_read(void* tobuffer, int64_t buffersize, int64_t readamount)
	{
		return 0;
	}

	int64_t NetStreamAudioOut::stream_write(const void* frombuffer, int64_t writesize)
	{
		packet_headers.packet_type = AUDIO_PACKET_TYPE;
		packet_headers.id = ++p_id;
		packet_headers.size = (uint16_t)writesize;
		packet_headers.data_in_ms = 20;
		packet_headers.timestamp = std::chrono::high_resolution_clock::now().time_since_epoch().count();
		packet_headers.data = (char*)frombuffer;

		
		
		int packed = packet_headers.pack(sendbuffer, sizeof(sendbuffer));
		local._sendto(remote, sendbuffer,packed, 0);
		

		return packed;


	}

	
}