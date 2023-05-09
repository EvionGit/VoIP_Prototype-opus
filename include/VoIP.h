#ifndef VOIP_APP_PROTO_HEADER_H
#define VOIP_APP_PROTO_HEADER_H



#include <wsock/wsa_init.h>
#include <wsock/udp_socket.h>

#include <pack/config_packet.h>

#include <stream/audio_stream_in.h>
#include <stream/audio_stream_out.h>
#include <stream/net_stream_audio_in.h>
#include <stream/net_stream_audio_out.h>

#include <SFML/Audio.hpp>
#include <ops/decoder.h>
#include <ops/Encoder.h>
#include <math.h>


#include <SFML/Window/Event.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/System/Clock.hpp>

#include "imgui.h"
#include "imgui-SFML.h"
#include "imgui_mic_dropdown.h"

class VoIP
{
private:

	std::mutex mtx;

	/* local metadata */
	wsock::udpSocket& sock;
	pack::AudioConfigPacket local_conf;
	int32_t bitrate;

	/* companion`s metadata */
	wsock::addr* remote;
	pack::AudioConfigPacket remote_conf;

	/* base streaming and decoding pointers */
	jbuf::JitterBuffer* jbuffer;
	ops::Decoder* dec;
	ops::Encoder* enc;
	stream::AudioStreamIn* recorder;
	stream::AudioStreamOut* listener;
	stream::NetStreamAudioIn* receiver;
	stream::NetStreamAudioOut* sender;

	/* conversation timer */
	std::chrono::high_resolution_clock::time_point start_process;
	long long current_process_time;

private:
	/* dropdown to choice a microphone */
	ImGui::MicDropDown* mic_dropdown;
	ImGuiWindowFlags wdropdown;

	/* base windows parameters */
	ImVec2 window_pos;
	ImVec2 window_size;
	ImGuiWindowFlags wcallto;
	ImGuiInputTextFlags remote_ip_flags;

	/* ip and port fields */
	int octets[4];
	int port;

	/* buttuns window parameters */
	ImVec2 wbtns_pos;
	ImVec2 wbtns_size;
	ImGuiWindowFlags wbtns;

	/* btns images */
	sf::Texture mic[2];
	sf::Texture* micref;

	sf::Texture speak[2];
	sf::Texture* speakref;

	sf::Texture call[2];
	sf::Texture* callref;

	sf::Texture settings[2];
	sf::Texture* settingsref;

	sf::Texture process;

	/* bitrate variation */
	const char* bps[7]{ "8000","32000","64000","128000","196000","256000","320000" };
	int cur_bit = 3;

	/* main window state */
	bool isCalling;
	bool inProcessing;
	bool isIncoming;

	/* btns state */
	bool isMuting;
	bool isSpeaking;
	bool isSettingUp;


public:
	/* main render ImGui function */
	void controller();

	VoIP(wsock::udpSocket& local, int samples_rate, int channels);
	~VoIP();

private:

	/* initialize UI */
	void set_ui();

	/* packets receiver, need to multiplex packet types*/
	void multiplex();

	/* get ACCEPTION from remote host */
	int accept_to();

	/* event for CALL button */
	int call_to();

	/* event for ABORT button */
	int abort_to();

	/* loads setting before SETTING button was pressed*/
	void load_settings();

	/* Active checks for using microphone availability. If not availability - choose a default mic */
	void check_microphone();

	/* inits record process (starts recorder) */
	void start_record_process();

	/* stops record process (stops recorder and clears rec-buffer) */
	void stop_record_process();

	/* inits listen process (starts recorder) */
	void start_listen_process();

	/* stops listen process (stops listener and clears listen-buffer) */
	void stop_listen_process();


	/* active conversation timer to string converter */
	std::string get_clock(long long time_in_sec);


};
#endif // !VOIP_APP_PROTO_HEADER_H
