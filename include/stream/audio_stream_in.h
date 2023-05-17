#ifndef AUDIO_STREAM_IN_HEADER_H
#define AUDIO_STREAM_IN_HEADER_H

#include "stream.h"
#include <SFML/Audio.hpp>

/*  default buf size for several raw audio packets with stereo 16-bit 48kHz splitted into 20ms */
#define CHUNKCOUNT 3
#define AUDIOBUFF 1920*2*2*CHUNKCOUNT

namespace stream
{
	/* collect raw audio data from recorder */
	class AudioStreamIn : public Stream, public sf::SoundRecorder
	{
	private:
		std::mutex mtx;
		bool end_of_file = true;
		/* in this case the buffer is cycle*/
		char audiobuffer[AUDIOBUFF];
		uint32_t s, e; // start and end point
		uint32_t size;

	public:
		AudioStreamIn();
		~AudioStreamIn();
	

	public:
		/* Encoder reads data from the buffer */
		virtual int64_t stream_read(void* tobuffer, int64_t buffersize, int64_t readamount) override;

		/* App writes data to the buffer */
		virtual int64_t stream_write(const void* frombuffer, int64_t writesize) override;

	private:
		/* sound recorder override. Calls when need data to playing*/
		virtual bool onProcessSamples(const sf::Int16* samples, size_t sampleCount) override;

		/* before start recording - unset EoF flag */
		virtual bool onStart() override;

		/* after stop recording - set EoF flag */
		virtual void onStop() override;

	};
}

#endif