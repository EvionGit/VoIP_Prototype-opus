#ifndef AUDIO_STREAM_IN_HEADER_H
#define AUDIO_STREAM_IN_HEADER_H

#include "stream.h"

/*  default buf size for 10 raw audio packets with monophonic 16-bit 44100hz splitted into 30-40ms */
#define AUDIOBUFF 1920*2*6

namespace stream
{
	/* collect raw audio data in self inside buffer */
	class AudioStreamIn : public Stream
	{
	private:
		std::mutex mtx;

		/* in this case the buffer is cycle*/
		char audiobuffer[AUDIOBUFF];
		uint32_t s, e; // start and end point
		uint32_t size;

	public:
		AudioStreamIn();
		~AudioStreamIn();

	public:
		/* Encoder reads data from the buffer */
		virtual size_t stream_read(void* tobuffer, size_t buffersize, size_t readamount) override;

		/* App writes data to the buffer */
		virtual size_t stream_write(const void* frombuffer, size_t writesize) override;

	};
}

#endif