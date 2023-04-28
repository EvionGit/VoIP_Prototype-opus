#ifndef OPUS_ENCODER_HEADER_H
#define OPUS_ENCODER_HEADER_H

#include "opus_base.h"


namespace ops
{
	
	class Encoder
	{
	private:
		int errorcode;
		HANDLE send_event;
		OpusEncoder* enc;

		Stream* input;
		Stream* output;

		OPUS_TYPE opus_type;
		int32_t bitrate;
		uint32_t tick_rate;

		int32_t irate;
		int32_t ichannels;
		int32_t ims;
		int16_t* ichunk;
		int32_t ichunk_size;

		int32_t orate;
		int32_t ochannels;
		unsigned char* ochunk;
		int32_t ochunk_size;
		

	private:
		static void CALLBACK tick_send(UINT uTimerId,UINT uMsg, DWORD_PTR dwUser,DWORD dw1,DWORD dw2);

	public:
		Encoder(OPUS_TYPE opus_app_type);
		~Encoder();

	public:
		int encode();
		void set_input_stream(Stream* in, OPUS_SAMPLES_RATE rate, int32_t channels,int32_t ms);
		void set_output_stream(Stream* out, OPUS_SAMPLES_RATE rate, int32_t channels);
		void set_tick_rate(uint32_t tick_ms);
		void set_bitrate(int32_t bitrate);

	

		
	};
}

#endif 

