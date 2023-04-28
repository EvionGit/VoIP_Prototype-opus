#include <stream/net_stream_audio_out.h>

namespace stream
{


	NetStreamAudioOut::NetStreamAudioOut(wsock::udpSocket& local, wsock::addr& remote_addr, int32_t ms)
		: sock(local), remote(remote_addr)
	{
		memset(sendbuf, 0, sizeof(sendbuf));
		packet.id = 0;
		packet.timestamp = 0;
		packet.data_in_ms = ms;

	}

	NetStreamAudioOut::~NetStreamAudioOut(){}

	size_t NetStreamAudioOut::stream_write(const void* frombuffer, size_t writesize)
	{
		
		packet.id++;
		packet.size = writesize;
		packet.timestamp = std::chrono::high_resolution_clock::now().time_since_epoch().count();
		packet.data = (char*)frombuffer;

		memcpy(sendbuf, &packet, sizeof(packet));
		memcpy(sendbuf + sizeof(packet), packet.data, packet.size);

		sock._sendto(remote, sendbuf, sizeof(packet) + packet.size, 0);

		return 1;


	}

	size_t NetStreamAudioOut::stream_read(void* tobuffer, size_t buffersize, size_t readamount) { return 0; };
}