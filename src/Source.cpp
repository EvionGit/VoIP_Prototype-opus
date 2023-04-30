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
	long long last = 0;
	stream::AudioStreamIn& astream;
public:
	AudioRecorder(stream::AudioStreamIn& astream) : astream(astream)
	{
		sf::Time t = sf::milliseconds(20);
		setProcessingInterval(t);
	}
	virtual  bool onProcessSamples(const sf::Int16* samples, size_t sampleCount) override
	{
		//printf("%lli\n",(std::chrono::high_resolution_clock::now().time_since_epoch().count() - last)/1000000);
		//last = std::chrono::high_resolution_clock::now().time_since_epoch().count();
		astream.stream_write(samples, sampleCount * 2);
		

		return 1;
	}
};

stream::NetStreamAudioOut* nout;
void sender()
{
	nout->stream_read(0, 0, 0);
}

#define MS 20

int main()
{
	wsock::WSA_INIT wsa;
	wsock::udpSocket sock;
	//wsock::addr remote("188.242.191.139","5555");
	wsock::addr remote("192.168.0.216", "5555");


	stream::AudioStreamIn ain;
	stream::FileStream fin("C:\\Users\\Андрей\\source\\repos\\epica.wav", "rb");
	jbuf::JitterBuffer j(60, 0, 0, false);

	stream::NetStreamAudioOut aout(sock, remote, MS);
	aout.set_jitter_buffer(&j);
	nout = &aout;
	AudioRecorder recorder(ain);
	recorder.setChannelCount(2);

	ops::Encoder encoder(ops::OPUS_TYPE::VOIP);
	encoder.set_input_stream(&ain, ops::OPUS_SAMPLES_RATE::kHz48, 2, MS);
	encoder.set_output_stream(&aout, ops::OPUS_SAMPLES_RATE::kHz48, 2);
	encoder.set_bitrate(128000);
	//encoder.set_tick_rate(MS);
	
	MMRESULT m = timeSetEvent(MS, 0, (LPTIMECALLBACK)sender, 0, TIME_PERIODIC);
	

	recorder.start(48000);

	printf("recording..\n");

	encoder.encode();
	timeKillEvent(m);
	return 0;
}
