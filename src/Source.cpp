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




class VoIP
{
private:

	
	std::mutex mtx;
	
	/* local metadata */
	wsock::udpSocket& sock;
	pack::AudioConfigPacket local_conf;
	uint32_t bitrate;

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
	long long current_process;

private:
	ImVec2 window_pos;
	ImVec2 window_size;

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

	sf::Texture process;

	/* main window state */
	bool isCalling;
	bool inProcessing;
	bool isIncoming;

	/* btns state */
	bool isMuting;
	bool isSpeaking;

public:
	VoIP(wsock::udpSocket& local) : sock(local), current_process(0),bitrate(128000)
	{
		
		jbuffer = new jbuf::JitterBuffer(40, 0, 1000, true);
		recorder = new stream::AudioStreamIn;
		listener = new stream::AudioStreamOut;
		receiver = new stream::NetStreamAudioIn;
		//sender = new stream::NetStreamAudioOut - init when calling or accepting incoming;
		
		
		
		enc = new ops::Encoder(ops::VOIP);
	
		

		dec = new ops::Decoder;
		dec->set_output_stream(listener, ops::kHz48, 2, 20);
		
		listener->set_decoder(dec);
		
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
				
				if (*(int8_t*)buff == AUDIO_PACKET_TYPE)
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
							remote_conf = *conf;
						}
					
					}

					else if(conf->packet_type == CONF_ABORTING_TYPE)
					{
						if((isIncoming || inProcessing || isCalling) && from._get_straddr() == remote->_get_straddr())
						{
							abort_to();
						}
						
					}
					else if(conf->packet_type == CONF_ACCEPT_TYPE)
					{
						if (isCalling && from._get_straddr() == remote->_get_straddr())
						{
							remote_conf = *conf;
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
			local_conf.packet_type = CONF_CONNECTION_TYPE;
			

			if (remote)
				delete remote;

			std::string remote_ip = std::to_string(octets[0]) + "." +
				std::to_string(octets[1]) + "." +
				std::to_string(octets[2]) + "." +
				std::to_string(octets[3]);

			remote = new wsock::addr(remote_ip, std::to_string(port));
			

			isCalling = true;
			
		}
		/* accept incoming call */
		else
		{
			if (sender)
				delete sender;

			sender = new stream::NetStreamAudioOut(sock, *remote, local_conf.data_size_ms);

			local_conf.packet_type = CONF_ACCEPT_TYPE;

			isIncoming = false;
			inProcessing = true;
			start_process = std::chrono::high_resolution_clock::now();

			start_listen_process();

		}
		
		
		callref = call+1;
		sock._sendto(*remote, &local_conf, sizeof(local_conf));
		
		return 1;
	}

	int accept_to()
	{
		std::lock_guard<std::mutex> lock(mtx);

		
		if (sender)
			delete sender;
		sender = new stream::NetStreamAudioOut(sock, *remote, local_conf.data_size_ms);

		


		isIncoming = false;
		inProcessing = true;
		start_process = std::chrono::high_resolution_clock::now();
		start_listen_process();
		

		return 1;
	}

	int abort_to()
	{
		std::lock_guard<std::mutex> lock(mtx);
		
		if (inProcessing)
		{
			end_listen_process();
		}
			

		isCalling = false;
		inProcessing = false;
		isIncoming = false;
		

		local_conf.packet_type = CONF_ABORTING_TYPE;

		sock._sendto(*remote, &local_conf, sizeof(local_conf));
	
		callref = call;
		return 1;
	}


	
	void set_ui()
	{
		window_pos = ImVec2(0, 0);
		window_size = ImVec2(800, 800);


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

		process.loadFromFile("..\\img\\process.png");

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
			ImGui::SetWindowPos("incoming_win", window_pos);
			ImGui::SetWindowSize("incoming_win", window_size);

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
			if (inProcessing && ImGui::Begin("process_win",0,wcallto))
			{

				current_process = (std::chrono::high_resolution_clock::now().time_since_epoch().count()
									- start_process.time_since_epoch().count()) / 1000000000;

				ImGui::SetWindowPos("process_win", window_pos);
				ImGui::SetWindowSize("process_win", window_size);

				
				ImGui::SetCursorPos(ImVec2(200, 150));
				ImGui::Image(process,ImVec2(400,400));
				
				ImGui::SetWindowFontScale(2);
				mtx.lock();
				ImGui::SetCursorPos(ImVec2((800 - ImGui::CalcTextSize(remote->_get_straddr().c_str()).x) / 2, 600));
				ImGui::LabelText("##", "%s", remote->_get_straddr().c_str());
			
				ImGui::SetCursorPos(ImVec2(352, 650));
				ImGui::LabelText("##", "%s", get_clock(current_process).c_str());
				

				mtx.unlock();

				


				ImGui::End();
			}

			/* call_to window */
			else if (!isCalling && ImGui::Begin("callto_win", 0, wcallto))
			{

				ImGui::SetWindowPos("callto_win", window_pos);
				ImGui::SetWindowSize("callto_win", window_size);

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
				ImGui::SetWindowPos("calling_win", window_pos);
				ImGui::SetWindowSize("calling_win", window_size);


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
					{
						speakref = speak;
						start_listen_process();
					}
						
					else
					{
						speakref = speak + 1;
						end_listen_process();
					}


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

	void start_record_process()
	{
		
		enc->set_bitrate(bitrate);
		enc->set_input_stream(recorder, local_conf.samples_rate, local_conf.channels, local_conf.data_size_ms);
		enc->set_output_stream(sender);
		recorder->setChannelCount(local_conf.channels);
		recorder->start(local_conf.samples_rate);
		enc->thread_encode();
		
	}

	void stop_record_process()
	{
		recorder->stop();
	}

	void start_listen_process()
	{

		receiver->reset_jitter_buffer();
		dec->set_input_stream(receiver);
		listener->set_listener_conf(remote_conf.samples_rate, remote_conf.channels);
		
		listener->play();

	}

	void end_listen_process()
	{
		listener->stop();
	}

	std::string get_clock(long long time_in_sec)
	{
		uint8_t h = 0, m = 0, s = 0;
		std::string hs, ms, ss;

		h = std::floor(time_in_sec / 3600);
		time_in_sec %= 3600;

		m = std::floor(time_in_sec / 60);
		time_in_sec %= 60;

		s = time_in_sec;

		
		hs = h < 10 ? "0" + std::to_string(h) : std::to_string(h);
		ms = m < 10 ? "0" + std::to_string(m) : std::to_string(m);
		ss = s < 10 ? "0" + std::to_string(s) : std::to_string(s);

		return hs + ":" + ms + ":" + ss;
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
	

	sf::Clock deltaClock;
	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			ImGui::SFML::ProcessEvent(window, event);

			if (event.type == sf::Event::Closed)
			{
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