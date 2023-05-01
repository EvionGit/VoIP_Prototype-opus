#define NOMINMAX

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



void recv_data(stream::NetStreamAudioIn& stream)
{
	wsock::WSA_INIT wsa;
	wsock::udpSocket sock("0.0.0.0", "5555");
	wsock::addr remote;


	uint64_t start = std::chrono::high_resolution_clock::now().time_since_epoch().count() / 1000000;
	int32_t jitter = 0;
	int32_t d = 0;

	int32_t last_received = 0;
	int32_t last_packet_timestamp = 0;
	int32_t avg = 0;
	int32_t count = 0;

	char buff[2048];

	long long last = 0;
	size_t m;
	while (1)
	{
		if ((m = sock._recvfrom(remote, &buff, sizeof(buff), 0)) > 0)
		{
			pack::AudioPacket* ap = (pack::AudioPacket*)buff;
			
			//printf("%u: %lli\n",ap->id, (ap->timestamp - last) / 1000000);
			//last = ap->timestamp;

			//stream.stream_write(buff+sizeof(pack::AudioPacket), ap->size);

			stream.stream_write(buff, m);
		}

	}
	printf("END\n");

}


//int main()
//{
//	stream::AudioStreamOut aout;
//	jbuf::JitterBuffer jb(40, 0);
//	stream::NetStreamAudioIn ain(&jb);
//
//
//	AudioListener listener(aout);
//
//	ops::Decoder decoder;
//	decoder.set_input_stream(&ain, ops::kHz48, 2);
//	decoder.set_output_stream(&aout, ops::kHz48, 2, 20);
//
//	
//
//	std::thread net_reader(recv_data, std::ref(ain));
//
//	listener.play();
//
//	decoder.decode();
//
//
//
//
//
//	return 0;
//}

class VoIP
{
private:
	std::mutex mtx;
	wsock::udpSocket& sock;
	wsock::addr* remote;

	jbuf::JitterBuffer* jbuffer;
	ops::Decoder* dec;
	ops::Encoder* enc;
	stream::AudioStreamIn* recorder;
	stream::AudioStreamOut* listener;
	stream::NetStreamAudioIn* receiver;
	stream::NetStreamAudioOut* sender;

	pack::AudioConfigPacket aconf;

private:
	ImVec2 wcallto_pos;
	ImVec2 wcallto_size;

	ImGuiWindowFlags wcallto;
	ImGuiInputTextFlags remote_ip_flags;

	int octets[4];
	int port;

	/* btns set */
	ImVec2 wbtns_pos;
	ImVec2 wbtns_size;

	ImGuiWindowFlags wbtns;


	sf::Texture mic[2];
	sf::Texture* micref;

	sf::Texture speak[2];
	sf::Texture* speakref;

	sf::Texture call[2];
	sf::Texture* callref;

	/* main window state */
	bool isCalling;
	bool inProcessing;
	bool isIncoming;

	/* btns state */
	bool isMuting;
	bool isSpeaking;

public:
	VoIP(wsock::udpSocket& local) : sock(local)
	{
		
		jbuffer = new jbuf::JitterBuffer(40, 0, 100, true);
		recorder = new stream::AudioStreamIn;
		listener = new stream::AudioStreamOut;
		receiver = new stream::NetStreamAudioIn(jbuffer);
		
		
		enc = new ops::Encoder(ops::VOIP);
		enc->set_input_stream(recorder, ops::kHz48, 2, 20);
		
		// enc->set_output - when connected
		enc->set_bitrate(128000);

		dec = new ops::Decoder;
		
		// dec->set_input - when connected
		dec->set_output_stream(listener, ops::kHz48, 2, 20);
		set_ui();

		std::thread recv_data(&VoIP::multiplex,this);
		recv_data.detach();

	};

	~VoIP()
	{
		if (enc)
			delete enc;
		if (dec)
			delete dec;
		if (receiver)
			delete receiver;
		if (sender)
			delete sender;
		if (recorder)
			delete recorder;
		if (listener)
			delete listener;
		if (jbuffer)
			delete jbuffer;
		
	}

	void multiplex()
	{
		char buff[1024];
		wsock::addr from;
		pack::AudioConfigPacket* conf;
		size_t m;
		while (1)
		{
			if ((m = sock._recvfrom(from, buff, sizeof(buff), 0)) > 0)
			{
				
				if (*(uint8_t*)buff == AUDIO_PACKET_TYPE)
				{
					if (inProcessing)
						receiver->stream_write(buff, m);
				}
				else
				{
				

					conf = (pack::AudioConfigPacket*)buff;

					/* incoming */
					if (conf->packet_type == CONF_CONNECTION_TYPE)
					{
						std::lock_guard<std::mutex> lock(mtx);

						/* if busy now */
						if(inProcessing || isIncoming || isCalling)
						{
							conf->packet_type = CONF_BUSY_TYPE;
							sock._sendto(from, conf, sizeof(pack::AudioConfigPacket));
						}
						/* if free */
						else
						{
							if (remote)
								*remote = from;
							else
								remote = new wsock::addr(from);

							isIncoming = true;
							aconf = *conf;
						}
					
					}

					else if(conf->packet_type == CONF_ABORTING_TYPE)
					{
						if((isIncoming || inProcessing || isCalling) && from._get_straddr() == remote->_get_straddr())
						{
							abort_to();
						}
						
					}
					else if(conf->packet_type = CONF_ACCEPT_TYPE)
					{
						if (isCalling && from._get_straddr() == remote->_get_straddr())
						{
							aconf = *conf;
							accept_to();
							
						}
					}
					else if(conf->packet_type == CONF_BUSY_TYPE)
					{
						printf("BUSY NOW\n");
						isCalling = false;
					}

					
				}

				
			}

		}
	}

	int call_to()
	{
		std::lock_guard<std::mutex> lock(mtx);

		/* init outcoming call */
		if (!isIncoming)
		{
			aconf.samples_rate = 48000;
			aconf.data_size_ms = 20;
			aconf.channels = 2;
			aconf.key = AUDIO_KEY;
			aconf.packet_type = CONF_CONNECTION_TYPE;

			if (remote)
				delete remote;

			std::string remote_ip = std::to_string(octets[0]) + "." +
				std::to_string(octets[1]) + "." +
				std::to_string(octets[2]) + "." +
				std::to_string(octets[3]);

			remote = new wsock::addr(remote_ip, std::to_string(port));
			

			isCalling = true;
			printf("INIT CALL\n");
			
		}
		/* accept incoming call */
		else
		{
			printf("ACCEPT CALL\n");
			switch (aconf.samples_rate)
			{
			case 8000:
				dec->set_input_stream(receiver, ops::kHz8, aconf.channels);
				listener->set_listener_conf(8000, aconf.channels);
				break;
			case 12000:
				dec->set_input_stream(receiver, ops::kHz12, aconf.channels);
				break;
			case 24000:
				dec->set_input_stream(receiver, ops::kHz24, aconf.channels);
				break;
			case 48000:
				dec->set_input_stream(receiver, ops::kHz48, aconf.channels);
				break;
			default:
				dec->set_input_stream(receiver, ops::kHz48, aconf.channels);
				break;
			}

			if (sender)
				delete sender;
			sender = new stream::NetStreamAudioOut(sock, *remote, 20);

			enc->set_output_stream(sender, ops::kHz48, 2);

			
	
			aconf.samples_rate = 48000;
			aconf.data_size_ms = 20;
			aconf.channels = 2;
			aconf.key = AUDIO_KEY;
			aconf.packet_type = CONF_ACCEPT_TYPE;

			isIncoming = false;
			inProcessing = true;

		}
		
		
		callref = call+1;
		sock._sendto(*remote, &aconf, sizeof(aconf));
		
		return 1;
	}

	int accept_to()
	{
		std::lock_guard<std::mutex> lock(mtx);

		switch (aconf.samples_rate)
		{
		case 8000:
			dec->set_input_stream(receiver, ops::kHz8, aconf.channels);
			listener->set_listener_conf(8000, aconf.channels);
			break;
		case 12000:
			dec->set_input_stream(receiver, ops::kHz12, aconf.channels);
			break;
		case 24000:
			dec->set_input_stream(receiver, ops::kHz24, aconf.channels);
			break;
		case 48000:
			dec->set_input_stream(receiver, ops::kHz48, aconf.channels);
			break;
		default:
			dec->set_input_stream(receiver, ops::kHz48, aconf.channels);
			break;
		}

		if (sender)
			delete sender;
		sender = new stream::NetStreamAudioOut(sock, *remote, 20);

		enc->set_output_stream(sender, ops::kHz48, 2);


		isIncoming = false;
		inProcessing = true;

		return 1;
	}

	int abort_to()
	{
		std::lock_guard<std::mutex> lock(mtx);

		isCalling = false;
		inProcessing = false;
		isIncoming = false;

		aconf.packet_type = CONF_ABORTING_TYPE;

		sock._sendto(*remote, &aconf, sizeof(aconf));
		delete remote;
		remote = 0;


		callref = call;
		printf("ABORT\n");
		return 1;
	}


	
	void set_ui()
	{
		wcallto_pos = ImVec2(0, 0);
		wcallto_size = ImVec2(800, 800);


		wcallto |= ImGuiWindowFlags_NoResize;
		wcallto |= ImGuiWindowFlags_NoTitleBar;
		wcallto |= ImGuiWindowFlags_NoMove;

		remote_ip_flags |= ImGuiInputTextFlags_NoHorizontalScroll;
		remote_ip_flags |= ImGuiInputTextFlags_CharsDecimal;

		memset(octets, 0, sizeof(octets));
		port = 5555;

		/* btns set */
		wbtns_pos = ImVec2(0, 800);
		wbtns_size = ImVec2(800, 200);

		wbtns |= ImGuiWindowFlags_NoResize;
		wbtns |= ImGuiWindowFlags_NoTitleBar;
		wbtns |= ImGuiWindowFlags_NoMove;

		mic[0].loadFromFile("..\\img\\micon.png");
		mic[1].loadFromFile("..\\img\\micoff.png");
		micref = mic;

	
		speak[0].loadFromFile("..\\img\\speakeron.png");
		speak[1].loadFromFile("..\\img\\speakeroff.png");
		speakref = speak;

	
		call[0].loadFromFile("..\\img\\begincall.png");
		call[1].loadFromFile("..\\img\\endcall.png");
		callref = call;

		isCalling = false;
		inProcessing = false;
		isIncoming = false;
		isMuting = false;
		isSpeaking = true;
	}

	void controller()
	{
		if (isIncoming && ImGui::Begin("incoming_win",0, wcallto))
		{
			ImGui::SetWindowPos("incoming_win", wcallto_pos);
			ImGui::SetWindowSize("incoming_win", wcallto_size);

			ImGui::SetWindowFontScale(3);
			ImGui::SetCursorPos(ImVec2(200, 250));
			ImGui::LabelText("##", "Incoming call from: ");

			ImGui::SetWindowFontScale(5);

			mtx.lock();
			ImGui::SetCursorPos(ImVec2((800 - ImGui::CalcTextSize(remote->_get_straddr().c_str()).x) / 2, 350));
			ImGui::LabelText("##", "%s",remote->_get_straddr().c_str());
			mtx.unlock();

			/* btns window */
			if (ImGui::Begin("incoming_btns_win", 0, wbtns))
			{
				ImGui::SetWindowPos("incoming_btns_win", wbtns_pos);
				ImGui::SetWindowSize("incoming_btns_win", wbtns_size);

				/* accept call */
				ImGui::SetCursorPos(ImVec2(ImGui::GetWindowPos().x + 300, ImGui::GetWindowPos().x + 70));
				if (ImGui::ImageButton(call[0], ImVec2(60, 60)))
				{
					call_to();
					
				}

				/* cancel call */
				ImGui::SetCursorPos(ImVec2(ImGui::GetWindowPos().x + 400, ImGui::GetWindowPos().x + 70));
				if (ImGui::ImageButton(call[1], ImVec2(60, 60)))
				{
					abort_to();
			
				}

				ImGui::End();
			}

			ImGui::End();
		}
		else
		{
			/* call_to window */
			if (!isCalling && ImGui::Begin("callto_win", 0, wcallto))
			{

				ImGui::SetWindowPos("callto_win", wcallto_pos);
				ImGui::SetWindowSize("callto_win", wcallto_size);

				/* ip label */
				ImGui::SetWindowFontScale(2);
				ImGui::SetCursorPos(ImVec2(100, 150));
				ImGui::LabelText("##", "IPv4 address of your companion:");

				/* ip field */
				ImGui::SetWindowFontScale(3);
				ImGui::SetCursorPos(ImVec2(100, 200));
				ImGui::SetNextItemWidth(600);
				if (ImGui::InputInt4("##", octets, remote_ip_flags))
				{

					for (int i = 0; i < 4; i++)
					{
						if (octets[i] > 255)
							octets[i] = 255;
						else if (octets[i] < 0)
							octets[i] = 0;
					}
				}

				/* port label */
				ImGui::SetWindowFontScale(2);
				ImGui::SetCursorPos(ImVec2(100, 300));
				ImGui::LabelText("##", "Port:");

				/* port field */
				ImGui::SetWindowFontScale(3);
				ImGui::SetCursorPos(ImVec2(100, 350));
				ImGui::SetNextItemWidth(600);
				if (ImGui::InputInt("##", &port, 1, remote_ip_flags))
				{
					if (port < 1025)
						port = 1025;
					else if (port > 65535)
						port = 65535;
				}


				ImGui::End();
			}

			else if (isCalling && ImGui::Begin("calling_win", 0, wcallto))
			{
				ImGui::SetWindowPos("calling_win", wcallto_pos);
				ImGui::SetWindowSize("calling_win", wcallto_size);


				ImGui::SetWindowFontScale(3);
				ImGui::SetCursorPos(ImVec2(200, 250));
				ImGui::LabelText("##", "Calling to..");


				ImGui::SetWindowFontScale(5);
				mtx.lock();
				ImGui::SetCursorPos(ImVec2((800- ImGui::CalcTextSize(remote->_get_straddr().c_str()).x) / 2, 350));
				ImGui::LabelText("##", "%s", remote->_get_straddr().c_str());
				mtx.unlock();

				ImGui::End();
			}

			/* btns window */
			if (ImGui::Begin("btns_win", 0, wbtns))
			{
				ImGui::SetWindowPos("btns_win", wbtns_pos);
				ImGui::SetWindowSize("btns_win", wbtns_size);

				/* micro */
				ImGui::SetCursorPos(ImVec2(ImGui::GetWindowPos().x + 200, ImGui::GetWindowPos().x + 70));
				if (ImGui::ImageButton(*micref, ImVec2(60, 60)))
				{
					if (isMuting)
						micref = mic;
					else
						micref = mic + 1;


					isMuting = !isMuting;
				}

				/* speaker */
				ImGui::SetCursorPos(ImVec2(ImGui::GetWindowPos().x + 300, ImGui::GetWindowPos().x + 70));
				if (ImGui::ImageButton(*speakref, ImVec2(60, 60)))
				{
					if (!isSpeaking)
						speakref = speak;
					else
						speakref = speak + 1;


					isSpeaking = !isSpeaking;
				}


				/* call */
				ImGui::SetCursorPos(ImVec2(ImGui::GetWindowPos().x + 400, ImGui::GetWindowPos().x + 70));
				if (ImGui::ImageButton(*callref, ImVec2(60, 60)))
				{
					if (isCalling || inProcessing)
					{
						abort_to();
					}
					else
					{
						call_to();
					}
					
				
				}


				ImGui::End();
			}

		}

	}
};

#define VOIP_APP 1
#define STREAM_APP 2

int main()
{
	wsock::WSA_INIT wsa;
	wsock::udpSocket sock("0.0.0.0", "5555");
	VoIP voip(sock);

	sf::RenderWindow window(sf::VideoMode(800, 1000), "VoIP - prototype");
	window.setFramerateLimit(60);
	ImGui::SFML::Init(window);
	
	

	uint8_t app = VOIP_APP;
	/************************************/

	sf::Clock deltaClock;
	while (window.isOpen()) {
		sf::Event event;
		while (window.pollEvent(event)) {
			ImGui::SFML::ProcessEvent(window, event);

			if (event.type == sf::Event::Closed) {
				window.close();
			}
		}

		ImGui::SFML::Update(window, deltaClock.restart());

		if (app == VOIP_APP)
			voip.controller();


		window.clear();
		ImGui::SFML::Render(window);
		window.display();
	}

	ImGui::SFML::Shutdown();

	return 0;
}