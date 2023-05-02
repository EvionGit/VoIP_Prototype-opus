#include <ops/decoder.h>

namespace ops
{
	Decoder::Decoder()
		:input(0), output(0),dec(0), errorcode(0), 
		irate(0), ichannels(0), ichunk(0), ichunk_size(0),
		orate(0), ochannels(0), ochunk(0), ochunk_size(0),oms(0)
	{

	}

	Decoder::~Decoder()
	{
		if (ichunk)
			delete[] ichunk;
		if (ochunk)
			delete[] ochunk;
		if (dec)
			opus_decoder_destroy(dec);
	}

	int Decoder::decode_to(int16_t* samples)
	{
		if (!input)
			return OPS_INPUT_STREAM_NOT_FOUND;
		if (!output)
			return OPS_OUTPUT_STREAM_NOT_FOUND;
		if (errorcode != 0)
			return errorcode;

		int read = input->stream_read(ichunk, 4000, 4000);

		if (read == OPS_LOST_PACKET)
		{
			int r = opus_decode(dec, 0, read, ochunk, ochunk_size / 2, 0);
			memcpy(samples, ochunk, r * ochannels * 2);
			return r * ochannels * 2;
			
		}
		else if(read > 0)
		{
			int r = opus_decode(dec, ichunk, read, ochunk, ochunk_size / 2, 0);

			memcpy(samples, ochunk, r * ochannels * 2);
			return r * ochannels * 2;

		}

		return 0;
	}

	int Decoder::decode()
	{
		if (!input)
			return OPS_INPUT_STREAM_NOT_FOUND;
		if (!output)
			return OPS_OUTPUT_STREAM_NOT_FOUND;
		if (errorcode != 0)
			return errorcode;

		int read = 0, r = 0;
READER:
		while((read = input->stream_read(ichunk,4000,4000)) > 0)
		{
			r = opus_decode(dec, ichunk, read, ochunk, ochunk_size / 2, 0);
			
			
			output->stream_write(ochunk, r * ochannels*2);
		}

		if (read == OPS_LOST_PACKET)
		{
			r = opus_decode(dec, 0, read, ochunk, ochunk_size / 2, 0);
			output->stream_write(ochunk, r * ochannels*2);
			goto READER;
		}

		return 0;
	}

	void Decoder::set_input_stream(Stream* in)
	{
		if (ichunk)
			delete[] ichunk;

		ichunk = new unsigned char[4000];

		if (dec)
			opus_decoder_destroy(dec);


		dec = opus_decoder_create(irate, ichannels, &errorcode);
	}

	void Decoder::set_output_stream(Stream* out, OPUS_SAMPLES_RATE rate, int32_t channels, int32_t ms)
	{
		output = out;
		orate = rate == DEFAULT ? 48000 : rate;
		ochannels = channels;
		oms = ms;
		ochunk_size = (int32_t)((oms / 1000.f) * orate) * ochannels;

		if (ochunk)
			delete[] ochunk;

		ochunk = new int16_t[ochunk_size];
	}
}