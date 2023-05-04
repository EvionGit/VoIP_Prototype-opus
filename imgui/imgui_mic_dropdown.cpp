#include "imgui_mic_dropdown.h"

namespace ImGui
{
	MicDropDown::MicDropDown(sf::SoundRecorder* recorder) : rec(recorder), text_label_flags(0)
	{
		text_label_flags |= ImGuiInputTextFlags_ReadOnly;
		memcpy(input_device, rec->getDevice().c_str(), rec->getDevice().size());
	}

	void MicDropDown::get_devices()
	{

	}

	void MicDropDown::render()
	{
		
		memcpy(input_device, rec->getDevice().c_str(), rec->getDevice().size());
		
		ImGui::InputText("##",input_device, rec->getDevice().size());
		
	}
}