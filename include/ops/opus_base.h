#ifndef OPUS_BASE_DEFINITION_HEADER_H
#define OPUS_BASE_DEFINITION_HEADER_H


#define OPS_BAD_APPLICATION_TYPE -100
#define OPS_UNSUPPORTED_BITRATE -200
#define OPS_BAD_FRAMESIZE -300
#define OPS_INPUT_STREAM_NOT_FOUND -400
#define OPS_OUTPUT_STREAM_NOT_FOUND -401
#define OPS_LOST_PACKET -2

#include <opus/opus.h>
#include <stream/stream.h>
#include <mutex>
#include <thread>
#include <windows.h>
#pragma comment(lib, "winmm.lib")

using stream::Stream;

namespace ops
{
	enum OPUS_TYPE {
		AUDIO = OPUS_APPLICATION_AUDIO,
		VOIP = OPUS_APPLICATION_VOIP,
		LOWDELAY = OPUS_APPLICATION_RESTRICTED_LOWDELAY
	};

	enum OPUS_SAMPLES_RATE {
		kHz8 = 8000,
		kHz12 = 12000,
		kHz24 = 24000,
		kHz48 = 48000,
		DEFAULT = 0
	};



}

#endif 




