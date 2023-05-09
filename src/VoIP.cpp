#include "VoIP.h"

VoIP::VoIP(wsock::udpSocket& local, int samples_rate, int channels) : sock(local), current_process_time(0)
{
	/* set local configs to ConfPacket */
	local_conf.channels = channels;
	local_conf.data_size_ms = 20;
	local_conf.key = AUDIO_KEY;

	switch (samples_rate)
	{
	case 8000: local_conf.samples_rate = ops::kHz8; break;
	case 12000: local_conf.samples_rate = ops::kHz12; break;
	case 16000: local_conf.samples_rate = ops::kHz16; break;
	case 24000: local_conf.samples_rate = ops::kHz24; break;
	case 48000: local_conf.samples_rate = ops::kHz48; break;
	default: local_conf.samples_rate = ops::kHz48; break;
		
	}
	

	bitrate = atoi(bps[cur_bit]);
	jbuffer = new jbuf::JitterBuffer(40);
	recorder = new stream::AudioStreamIn;
	listener = new stream::AudioStreamOut;
	receiver = new stream::NetStreamAudioIn;
	mic_dropdown = new ImGui::MicDropDown(recorder);


	enc = new ops::Encoder(ops::VOIP);
	dec = new ops::Decoder;

	receiver->set_jitter_buffer(jbuffer);
	listener->set_decoder(dec);


	set_ui();

	/* start thread for receiving data */
	std::thread recv_data(&VoIP::multiplex, this);
	recv_data.detach();
}

VoIP::~VoIP()
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
	if (mic_dropdown)
		delete mic_dropdown;
}

void VoIP::multiplex()
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
					if (inProcessing || isIncoming || isCalling)
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

				/* if received ABORT type*/
				else if (conf->packet_type == CONF_ABORTING_TYPE)
				{
					if ((isIncoming || inProcessing || isCalling) && from._get_straddr() == remote->_get_straddr())
					{
						printf("ABORTING\n");
						abort_to();
					}

				}
				/* if received ACCEPT type*/
				else if (conf->packet_type == CONF_ACCEPT_TYPE)
				{
					if (isCalling && from._get_straddr() == remote->_get_straddr())
					{
						remote_conf = *conf;
						accept_to();


					}
				}
				/* if received BUSY type*/
				else if (conf->packet_type == CONF_BUSY_TYPE)
				{
					printf("BUSY NOW\n");
					isCalling = false;
				}
			}

		}

	}
}

int VoIP::accept_to()
{
	std::lock_guard<std::mutex> lock(mtx);

	if (sender)
		delete sender;
	sender = new stream::NetStreamAudioOut(sock, *remote);


	isIncoming = false;
	inProcessing = true;
	start_process = std::chrono::high_resolution_clock::now();

	/* start record and listen */
	if (!isMuting)
		start_record_process();
	if (isSpeaking)
		start_listen_process();


	return 1;
}

int VoIP::call_to()
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

		sender = new stream::NetStreamAudioOut(sock, *remote);

		local_conf.packet_type = CONF_ACCEPT_TYPE;

		isIncoming = false;
		inProcessing = true;
		start_process = std::chrono::high_resolution_clock::now();

		/* start record and listen */
		if (!isMuting)
			start_record_process();
		if (isSpeaking)
			start_listen_process();

	}


	callref = call + 1;
	sock._sendto(*remote, &local_conf, sizeof(local_conf));

	return 1;
}

int VoIP::abort_to()
{
	std::lock_guard<std::mutex> lock(mtx);

	/* stop record and listen */
	if (inProcessing)
	{
		stop_record_process();
		stop_listen_process();
	}

	/* refresh states */
	isCalling = false;
	inProcessing = false;
	isIncoming = false;

	/* send ABORT type msg to remote */
	local_conf.packet_type = CONF_ABORTING_TYPE;
	sock._sendto(*remote, &local_conf, sizeof(local_conf));

	/* change ABORT-img to CALL-img  */
	callref = call;
	return 1;
}

void VoIP::set_ui()
{
	window_pos = ImVec2(0, 0);
	window_size = ImVec2(800, 800);

	wdropdown |= ImGuiWindowFlags_NoResize;
	wdropdown |= ImGuiWindowFlags_NoMove;


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

	settings[0].loadFromFile("..\\img\\settings.png");
	settings[1].loadFromFile("..\\img\\settings_exit.png");
	settingsref = settings;

	process.loadFromFile("..\\img\\process.png");

	isCalling = false;
	inProcessing = false;
	isIncoming = false;
	isMuting = false;
	isSpeaking = true;
	isSettingUp = false;
}

void VoIP::load_settings()
{
	/* loads microphones */
	mic_dropdown->get_devices();
}

void VoIP::controller()
{
	if (isIncoming && ImGui::Begin("incoming_win", 0, wcallto))
	{
		ImGui::SetWindowPos("incoming_win", window_pos);
		ImGui::SetWindowSize("incoming_win", window_size);

		ImGui::SetWindowFontScale(3);
		ImGui::SetCursorPos(ImVec2(200, 250));
		ImGui::LabelText("##label_income", "Incoming call from: ");

		ImGui::SetWindowFontScale(5);

		mtx.lock();
		ImGui::SetCursorPos(ImVec2((800 - ImGui::CalcTextSize(remote->_get_straddr().c_str()).x) / 2, 350));
		ImGui::LabelText("##label_remote_ip", "%s", remote->_get_straddr().c_str());
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
		if (inProcessing && ImGui::Begin("process_win", 0, wcallto))
		{
			check_microphone();

			current_process_time = (std::chrono::high_resolution_clock::now().time_since_epoch().count()
				- start_process.time_since_epoch().count()) / 1000000000;

			ImGui::SetWindowPos("process_win", window_pos);
			ImGui::SetWindowSize("process_win", window_size);


			ImGui::SetCursorPos(ImVec2(200, 150));
			ImGui::Image(process, ImVec2(400, 400));

			ImGui::SetWindowFontScale(2);
			mtx.lock();
			ImGui::SetCursorPos(ImVec2((800 - ImGui::CalcTextSize(remote->_get_straddr().c_str()).x) / 2, 600));
			ImGui::LabelText("##label_remote_ip", "%s", remote->_get_straddr().c_str());

			ImGui::SetCursorPos(ImVec2(352, 650));
			ImGui::LabelText("##label_clock", "%s", get_clock(current_process_time).c_str());


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
			ImGui::LabelText("##label_ip_welcome", "IPv4 address of your companion:");

			/* ip field */
			ImGui::SetWindowFontScale(3);
			ImGui::SetCursorPos(ImVec2(100, 200));
			ImGui::SetNextItemWidth(600);
			if (ImGui::InputInt4("##input4_ip", octets, remote_ip_flags))
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
			ImGui::LabelText("##label_port", "Port:");

			/* port field */
			ImGui::SetWindowFontScale(3);
			ImGui::SetCursorPos(ImVec2(100, 350));
			ImGui::SetNextItemWidth(600);
			if (ImGui::InputInt("##input_port", &port, 1, remote_ip_flags))
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
			ImGui::SetCursorPos(ImVec2(280, 250));
			ImGui::LabelText("##label_calling", "Calling to..");


			ImGui::SetWindowFontScale(5);
			mtx.lock();
			ImGui::SetCursorPos(ImVec2((800 - ImGui::CalcTextSize(remote->_get_straddr().c_str()).x) / 2, 350));
			ImGui::LabelText("##label_remote_ip", "%s", remote->_get_straddr().c_str());
			mtx.unlock();

			ImGui::End();
		}

		/* settings window */
		if (isSettingUp && ImGui::Begin("settitng_win", 0, wcallto))
		{
			ImGui::SetWindowPos("settitng_win", window_pos);
			ImGui::SetWindowSize("settitng_win", window_size);


			ImGui::SetWindowFontScale(2);
			ImGui::SetCursorPos(ImVec2(ImGui::GetWindowPos().x + 150, ImGui::GetWindowPos().x + 70));

			ImGui::LabelText("##label_input_device", "Input device:");

			ImGui::SetWindowFontScale(2);
			ImGui::SetCursorPos(ImVec2(ImGui::GetWindowPos().x + 150, ImGui::GetWindowPos().x + 120));
			if (mic_dropdown->render())
			{
				stop_record_process();
				recorder->setDevice(mic_dropdown->get_current());
				if (inProcessing && !isMuting)
					start_record_process();
			}

			ImGui::SetCursorPos(ImVec2(ImGui::GetWindowPos().x + 150, ImGui::GetCursorPosY() + 30));
			ImGui::LabelText("##label_bitrate", "Bitrate (bps):");
			ImGui::SetCursorPos(ImVec2(ImGui::GetWindowPos().x + 150, ImGui::GetCursorPosY()));
			int last_bit = cur_bit;
			if (ImGui::Combo("##combo_bitrate", &cur_bit, bps, 7))
			{
				if (cur_bit != last_bit)
				{
					stop_record_process();
					bitrate = atoi(bps[cur_bit]);
					if (inProcessing && !isMuting)
						start_record_process();
				}
			}



			ImGui::End();
		}

		/*if(ImGui::IsKeyDown(ImGuiKey_M))
		{
			printf("M_down\n");
		}*/

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
				{
					micref = mic;
					start_record_process();
				}

				else
				{
					micref = mic + 1;
					stop_record_process();
				}

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
					stop_listen_process();
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

			/* settings */
			ImGui::SetCursorPos(ImVec2(ImGui::GetWindowPos().x + 500, ImGui::GetWindowPos().x + 70));
			if (ImGui::ImageButton(*settingsref, ImVec2(60, 60)))
			{
				if (!isSettingUp)
				{
					load_settings();
					settingsref = settings + 1;
				}
				else
				{
					settingsref = settings;
				}


				isSettingUp = !isSettingUp;

			}


			ImGui::End();
		}

	}
}

void VoIP::check_microphone()
{
	std::vector<std::string> devs = recorder->getAvailableDevices();
	for (auto it = devs.begin(); it != devs.end(); it++)
	{
		if (recorder->getDevice() == *it)
			return;
	}

	stop_record_process();
	recorder->setDevice(recorder->getDefaultDevice());
	start_record_process();
}

void VoIP::start_record_process()
{

	enc->set_input_stream(recorder, local_conf.samples_rate, local_conf.channels, local_conf.data_size_ms);
	enc->set_bitrate(bitrate);
	enc->set_output_stream(sender);
	recorder->setChannelCount(local_conf.channels);
	recorder->start(local_conf.samples_rate);
	enc->thread_encode();

}

void VoIP::stop_record_process()
{
	recorder->stop();
}

void VoIP::start_listen_process()
{

	receiver->reset_jitter_buffer();
	dec->set_input_stream(receiver);
	listener->set_listener_conf(remote_conf.samples_rate, (uint8_t)remote_conf.channels);
	dec->set_output_stream(listener, remote_conf.samples_rate, remote_conf.channels, remote_conf.data_size_ms);

	listener->_play();

}

void VoIP::stop_listen_process()
{
	listener->_stop();
}

std::string VoIP::get_clock(long long time_in_sec)
{
	uint8_t h = 0, m = 0, s = 0;
	std::string hs, ms, ss;

	h = (uint8_t)(std::floor(time_in_sec / 3600));
	time_in_sec %= 3600;

	m = (uint8_t)(std::floor(time_in_sec / 60));
	time_in_sec %= 60;

	s = (uint8_t)time_in_sec;


	hs = h < 10 ? "0" + std::to_string(h) : std::to_string(h);
	ms = m < 10 ? "0" + std::to_string(m) : std::to_string(m);
	ss = s < 10 ? "0" + std::to_string(s) : std::to_string(s);

	return hs + ":" + ms + ":" + ss;
}