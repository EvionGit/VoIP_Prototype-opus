#include <wsock/wsa_init.h>
#include <wsock/udp_socket.h>
#include <stream/net_stream_audio_out.h>
#include <stream/audio_stream_in.h>
#include <stream/filestream.h>
#include <SFML/Audio.hpp>
#include <jbuf/jitter_buffer.h>
#include <ops/Encoder.h>




class AudioRecorder : public sf::SoundRecorder
{
	stream::AudioStreamIn& astream;
public:
	AudioRecorder(stream::AudioStreamIn& astream) : astream(astream)
	{
		
	}
	virtual  bool onProcessSamples(const sf::Int16* samples, size_t sampleCount) override
	{
		
		astream.stream_write(samples, sampleCount * 2);
		

		return 1;
	}
};



int main()
{
	wsock::WSA_INIT wsa;
	wsock::udpSocket sock;
	//wsock::addr remote("188.242.191.139","5555");
	wsock::addr remote("192.168.0.216", "5555");


	stream::AudioStreamIn ain;
	stream::FileStream fin("C:\\Users\\Андрей\\source\\repos\\Vorbis_Wrapper\\Vogg\\Vorbis_Wrapper\\WAV.wav", "rb");


	stream::NetStreamAudioOut aout(sock, remote,20);

	AudioRecorder recorder(ain);
	recorder.setChannelCount(2);

	ops::Encoder encoder(ops::OPUS_TYPE::AUDIO);
	encoder.set_input_stream(&fin, ops::OPUS_SAMPLES_RATE::kHz48, 2, 20);
	encoder.set_output_stream(&aout, ops::OPUS_SAMPLES_RATE::kHz48, 2);
	encoder.set_bitrate(128000);
	encoder.set_tick_rate(20);
	
	

	//recorder.start();

	printf("recording..\n");

	encoder.encode();

	return 0;
}
