#ifndef IMGUI_EXTENSION_DROPDOWN_HEADER_H
#define IMGUI_EXTENSION_DROPDOWN_HEADER_H

#include "imgui.h"
#include "imgui-SFML.h"
#include <SFML/Audio/SoundRecorder.hpp>
#include <vector>
#include <map>

namespace ImGui
{
	class MicDropDown
	{
	private:
		std::vector<std::string> devs;
		char** devs_c;
		int selected;
		sf::SoundRecorder* rec;
		
	public:
		MicDropDown(sf::SoundRecorder* recorder);
		~MicDropDown();

	public:
		bool render();
		void get_devices();
		std::string get_current();

	};
}

#endif

