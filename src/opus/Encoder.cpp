#include <ops/Encoder.h>

namespace ops
{
	Encoder::Encoder(OPUS_TYPE opus_app_type)
		:
		  input(0), output(0), tick_rate(0), bitrate(48000), send_event(0),
		  enc(0),errorcode(0),opus_type(OPUS_TYPE::AUDIO),
		  irate(0), ichannels(0), ims(0),ichunk(0),ichunk_size(0),
		  orate(0), ochannels(0), ochunk(0), ochunk_size(0)
	{
		switch(opus_app_type)
		{
		case OPUS_TYPE::AUDIO:
			opus_type = OPUS_TYPE::AUDIO;
			break;

		case OPUS_TYPE::VOIP:
			opus_type = OPUS_TYPE::VOIP;
			break;

		case OPUS_TYPE::LOWDELAY:
			opus_type = OPUS_TYPE::LOWDELAY;
			break;

		default:
			errorcode = OPS_BAD_APPLICATION_TYPE;
			break;
		}

		



	}

	void Encoder::set_input_stream(Stream* in, OPUS_SAMPLES_RATE rate, int32_t channels, int32_t ms)
	{
		input = in;
		irate = rate == DEFAULT ? 48000 : rate;
		ichannels = channels;
		ims = ms;
		ichunk_size = (int32_t)((ims / 1000.f) * irate) * ichannels;

		if (ichunk)
			delete[] ichunk;

		ichunk = new int16_t[ichunk_size];

		if (enc)
			opus_encoder_destroy(enc);

		enc = opus_encoder_create(irate, ichannels, opus_type, &errorcode);
		
	}

	void Encoder::set_output_stream(Stream* out)
	{
		output = out;

		if (ochunk)
			delete[] ochunk;

		ochunk = new unsigned char[4000];

		
		
	}

	void Encoder::set_tick_rate(uint32_t tick_ms)
	{
		if (tick_ms >= 0)
			tick_rate = tick_ms;

	}

	void Encoder::thread_encode()
	{
		std::thread t1(&Encoder::encode, this);
		t1.detach();
		
	}

	void Encoder::set_bitrate(int32_t bitrate)
	{

		if (!bitrate || (bitrate >= 500 && bitrate <= 512000))
		{
			this->bitrate = bitrate;
		}

	}

	

	int Encoder::encode()
	{
		if (!input)
			return OPS_INPUT_STREAM_NOT_FOUND;
		if (!output)
			return OPS_OUTPUT_STREAM_NOT_FOUND;
		if (errorcode != 0)
			return errorcode;
		
		
		MMRESULT mm_id = 0;

		int read = 0;
		int i = 1;
		long long last = 0;
		while((read = input->stream_read(ichunk,ichunk_size*2,ichunk_size*2)) > 0)
		{
			//printf("READ: %i  - %lli\n",read, (std::chrono::high_resolution_clock::now().time_since_epoch().count() - last) / 1000000);
			last = std::chrono::high_resolution_clock::now().time_since_epoch().count();
			ochunk_size = opus_encode(enc, ichunk, ichunk_size / ichannels, ochunk, 4000);
			
			if (tick_rate)
			{
				if (!send_event)
					send_event = CreateEvent(0, TRUE, FALSE, (LPCWSTR)"TICK_SEND");

				mm_id = timeSetEvent(tick_rate-3, 0, (LPTIMECALLBACK)tick_send, (DWORD_PTR)this, TIME_ONESHOT);
				WaitForSingleObject(send_event, INFINITE);
				timeKillEvent(mm_id);
				ResetEvent(send_event);
			}
			else
			{
				output->stream_write(ochunk, ochunk_size);
			}

		}


		CloseHandle(send_event);
		return 0;

		
	}

	void CALLBACK Encoder::tick_send(UINT uTimerId, UINT uMsg, DWORD_PTR dwUser, DWORD dw1, DWORD dw2)
	{
		
		Encoder* obj = (Encoder*)dwUser;
		obj->output->stream_write(obj->ochunk, obj->ochunk_size);
		SetEvent(obj->send_event);
		

	}

	Encoder::~Encoder()
	{
		if (ichunk)
			delete[] ichunk;
		if (ochunk)
			delete[] ochunk;
		if (enc)
			opus_encoder_destroy(enc);

	}
}