#ifndef STREAM_ABS_HEADER_H
#define STREAM_ABS_HEADER_H

#include <stdio.h>
#include <stdint.h>
#include <thread>
#include <mutex>

namespace stream
{
	class Stream
	{

	protected:
		Stream();
		~Stream();

	public:
		/*  */
		virtual int64_t stream_read(void* tobuffer, int64_t buffersize, int64_t readamount) = 0;

		/*  */
		virtual int64_t stream_write(const void* frombuffer, int64_t writesize) = 0;


	};
}

#endif
