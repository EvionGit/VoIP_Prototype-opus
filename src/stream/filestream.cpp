#include <stream/filestream.h>


namespace stream
{
	FileStream::FileStream(const char* from, const char* mode)
	{
		errno_t code = 0; // check error code

		if ((code = fopen_s(&f, from, mode)) != 0)
		{
			printf("FILE IN ERROR: %i", code);
			//throw
		};
	
	};

	FileStream::~FileStream()
	{
		if (f)
			fclose(f);
	};

	size_t FileStream::stream_read(void* tobuffer, size_t buffersize, size_t readamount)
	{
	
		return fread_s(tobuffer, buffersize, 1, readamount, f);
	}

	size_t FileStream::stream_write(const void* frombuffer, size_t writesize)
	{
		return fwrite(frombuffer, 1, writesize, f);
	}

	int64_t FileStream::stream_tell()
	{
		return ftell(f);
	}

	void FileStream::add_header(const void* header, uint16_t header_size)
	{
		stream_seek_begin(0);
		fwrite(header, 1, header_size, f);
	}

	int32_t FileStream::stream_seek_begin(int32_t offset)
	{
		return fseek(f, offset, SEEK_SET);	
	}

	
	int32_t FileStream::stream_seek_end(int32_t offset)
	{
		return fseek(f, offset, SEEK_END);
	}

	
	int32_t FileStream::strema_seek(int32_t offset)
	{
		return fseek(f, offset, SEEK_CUR);
	}
}