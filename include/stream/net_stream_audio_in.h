#ifndef NET_STREAM_AUDIO_IN_HEADER_H
#define NET_STREAM_AUDIO_IN_HEADER_H

#include "stream.h"
#include <jbuf/jitter_buffer.h>



namespace stream
{
	
	class NetStreamAudioIn : public Stream
	{
	private:
		
		jbuf::JitterBuffer* jb;

	public:
		NetStreamAudioIn(jbuf::JitterBuffer* jitter_buffer);


	public:
		/* Decoder reads data from the buffer */
		virtual size_t stream_read(void* tobuffer, size_t buffersize, size_t readamount) override;

		/* Protocol writes data to the buffer */
		virtual size_t stream_write(const void* frombuffer, size_t writesize) override;

	};
}

#endif