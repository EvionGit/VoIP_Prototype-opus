#ifndef NET_STREAM_AUDIO_IN_HEADER_H
#define NET_STREAM_AUDIO_IN_HEADER_H

#include "stream.h"
#include <jbuf/jitter_buffer.h>



namespace stream
{
	/* */
	class NetStreamAudioIn : public Stream
	{
	private:
		std::mutex mtx;
		jbuf::JitterBuffer* jb;

	public:
		NetStreamAudioIn();


	public:
		/* Decoder reads data from the buffer */
		virtual int64_t stream_read(void* tobuffer, int64_t buffersize, int64_t readamount) override;

		/* Protocol writes data to the buffer */
		virtual int64_t stream_write(const void* frombuffer, int64_t writesize) override;

		/* set pointer to jitter_buffer */
		void set_jitter_buffer(jbuf::JitterBuffer* jitter_buffer);

		/* reset data from buffer */
		void reset_jitter_buffer();

	};
}

#endif