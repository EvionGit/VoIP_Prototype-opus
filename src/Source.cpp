#include "VoIP.h"



#define VOIP_APP 1
#define STREAM_APP 2

int main()
{
	wsock::WSA_INIT wsa;
	wsock::udpSocket* sock = 0;
	VoIP* voip = 0;


	sf::RenderWindow window(sf::VideoMode(800, 1000), "VoIP - prototype");
	window.setFramerateLimit(60);
	ImGui::SFML::Init(window);

	/* main window parameters */
	ImGuiWindowFlags main_flags = 0;
	main_flags |= ImGuiWindowFlags_NoResize;
	main_flags |= ImGuiWindowFlags_NoTitleBar;
	main_flags |= ImGuiWindowFlags_NoMove;

	ImVec2 mwindow_pos = ImVec2(0, 0);
	ImVec2 mwindow_size = ImVec2(800, 1000);

	/**************************/

	/* combos data */

	/* get all available interfaces */
	std::vector<std::string> interfaces_s= wsock::addr::get_available_interfaces(AF_INET);
	const char** interfaces_c = new const char*[interfaces_s.size()];
	for (int i = 0; i < interfaces_s.size();i++)
	{
		interfaces_c[i] = interfaces_s[i].data();
	}

	int interface_selected = 0;

	/* local port */
	int port = 5555;
	ImGuiInputTextFlags local_port_flags = 0;
	local_port_flags |= ImGuiInputTextFlags_NoHorizontalScroll;
	local_port_flags |= ImGuiInputTextFlags_CharsDecimal;


	/* samples rate combo */
	const char* samples_rate[5]{ "8000","12000","16000","24000","48000"};
	int cur_sample_rate = 4;

	/* channels */
	int channels = 2;

	

	uint8_t app = 0;
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
			voip->controller();

		else if (app == STREAM_APP)
			printf("dont implemented\n");

		else if(ImGui::Begin("main_window",0,main_flags))
		{
			ImGui::SetWindowPos("main_window", mwindow_pos);
			ImGui::SetWindowSize("main_window", mwindow_size);
			ImGui::SetWindowFontScale(2);

			ImGui::SetCursorPos(ImVec2(150,150));
			ImGui::LabelText("##label_local_interface", "Local interface: ");
			ImGui::SetWindowFontScale(3);
			ImGui::SetCursorPos(ImVec2(150, 190));
			if(ImGui::Combo("##interface",&interface_selected,interfaces_c,interfaces_s.size()))
			{
				/* change active interface */
			}
			ImGui::SetWindowFontScale(2);
			ImGui::SetCursorPos(ImVec2(150, 250));
			ImGui::LabelText("##label_local_port", "Local port: ");
			ImGui::SetCursorPos(ImVec2(150, 290));
			ImGui::SetWindowFontScale(3);
			if (ImGui::InputInt("##local_port", &port, 1, local_port_flags))
			{
				if (port < 1025)
					port = 1025;
				else if (port > 65535)
					port = 65535;
			}

			ImGui::SetCursorPos(ImVec2(150, 450));
			ImGui::SetWindowFontScale(2);
			ImGui::LabelText("##label_samples_rate", "Samples rate (Hz): ");
			ImGui::SetCursorPos(ImVec2(150, 490));
			ImGui::SetWindowFontScale(3);
			ImGui::Combo("##samples_rate", &cur_sample_rate, samples_rate, 5);
			

			ImGui::SetCursorPos(ImVec2(150, 550));
			ImGui::SetWindowFontScale(2);
			ImGui::LabelText("##label_channels", "Channels: ");
			ImGui::SetCursorPos(ImVec2(150, 590));
			ImGui::SetWindowFontScale(3);
			ImGui::SliderInt("##slider_channels", &channels, 1, 2);


			ImGui::SetCursorPos(ImVec2(200, 800));
			ImGui::SetWindowFontScale(2);
			if(ImGui::Button("Start VoIP",ImVec2(400,150)))
			{
				app = VOIP_APP;
				if(!voip)
				{
					sock = new wsock::udpSocket(interfaces_c[interface_selected], std::to_string(port).c_str());
					voip = new VoIP(*sock, atoi(samples_rate[cur_sample_rate]), channels);
					
				}
			}

			ImGui::End();
		}

		
	


		window.clear();
		ImGui::SFML::Render(window);
		window.display();

	}

	ImGui::SFML::Shutdown();

	delete voip;
	delete sock;
	delete[] interfaces_c;

	return 0;


}