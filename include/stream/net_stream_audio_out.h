
#ifndef NET_STREAM_AUDIO_OUT_HEADER
#define NET_STREAM_AUDIO_OUT_HEADER

#include "stream.h"
#include <wsock/udp_socket.h>
#include <pack/audio_packet.h>


namespace stream
{
	class NetStreamAudioOut : public Stream
	{
	private:
		pack::AudioPacket packet_headers;
		uint32_t p_id;

		wsock::udpSocket& local;
		wsock::addr remote;
		std::mutex mtx;
		char sendbuffer[2048];
		



	public:
		NetStreamAudioOut(wsock::udpSocket& local_addr, wsock::addr remote_addr);
		~NetStreamAudioOut();

	public:
		virtual int64_t stream_write(const void* frombuffer, int64_t writesize) override;
		
	private:
		virtual int64_t stream_read(void* tobuffer, int64_t buffersize, int64_t readamount) override;



	};
}

#endif