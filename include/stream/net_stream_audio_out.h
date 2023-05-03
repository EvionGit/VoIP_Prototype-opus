
#ifndef NET_STREAM_AUDIO_OUT_HEADER
#define NET_STREAM_AUDIO_OUT_HEADER

#include "stream.h"
#include <wsock/udp_socket.h>
#include <jbuf/jitter_buffer.h>


namespace stream
{
	class NetStreamAudioOut : public Stream
	{
	private:
		pack::AudioPacket packet_headers;
		jbuf::JitterBuffer* jb;
		wsock::udpSocket& local;
		wsock::addr remote;
		std::mutex mtx;
		char sendbuffer[2048];
		uint32_t sample_rate;
		long long last = 0;



	public:
		NetStreamAudioOut(wsock::udpSocket& local_addr, wsock::addr remote_addr, uint32_t rate);
		~NetStreamAudioOut();

	public:
		virtual int64_t stream_write(const void* frombuffer, int64_t writesize) override;
		virtual int64_t stream_read(void* tobuffer, int64_t buffersize, int64_t readamount) override;

		void set_jitter_buffer(jbuf::JitterBuffer* jbuffer);



	};
}

#endif