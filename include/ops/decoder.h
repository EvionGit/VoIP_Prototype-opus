#ifndef OPUS_DECODER_HEADER_H
#define OPUS_DECODER_HEADER_H

#include "opus_base.h"


namespace ops
{

	class Decoder
	{
	private:
		int errorcode;

		OpusDecoder* dec;

		Stream* input;
		Stream* output;

		int32_t irate;
		int32_t ichannels;
		unsigned char* ichunk;
		int32_t ichunk_size;

		int32_t orate;
		int32_t ochannels;
		int16_t* ochunk;
		int32_t oms;
		int32_t ochunk_size;


	
	public:
		Decoder();
		~Decoder();

	public:
		int decode();
		void set_input_stream(Stream* in, OPUS_SAMPLES_RATE rate, int32_t channels);
		void set_output_stream(Stream* out, OPUS_SAMPLES_RATE rate, int32_t channels,int32_t ms);


	};
}

#endif 

