#ifndef FILE_STREAM_HEADER_H
#define FILE_STREAM_HEADER_H

#include "stream.h"

namespace stream
{
	class FileStream : public Stream
	{

	private:
		FILE* f; // FILE STREAM


	public:
		virtual size_t stream_read(void* tobuffer, size_t buffersize, size_t readamount) override;
		virtual size_t stream_write(const void* frombuffer, size_t writesize) override;

	public:
		/* Returns current position in a stream */
		int64_t stream_tell();

		/* set position in a stream at start of a stream */
		int32_t stream_seek_begin(int32_t offset);

		/* set position in a stream at end of a stream */
		int32_t stream_seek_end(int32_t offset);

		/* set position in a stream at current position of a stream */
		int32_t strema_seek(int32_t offset);

		void add_header(const void* header, uint16_t header_size);

	public:
		FileStream(const char* f, const char* mode);
		~FileStream();

	
	private:
		FileStream(FileStream& fstream); // no copy
		FileStream& operator=(FileStream& fstream); // no assignment
	
	};

}

#endif