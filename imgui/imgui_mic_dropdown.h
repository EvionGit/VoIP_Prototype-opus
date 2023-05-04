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
		sf::SoundRecorder* rec;
		char input_device[255];
		ImGuiInputTextFlags text_label_flags;
	public:
		MicDropDown(sf::SoundRecorder* recorder);
		void render();
		void get_devices();
		std::string get_current();
		void save();

	};
}

#endif

