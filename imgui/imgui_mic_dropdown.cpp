#include "imgui_mic_dropdown.h"

namespace ImGui
{
	MicDropDown::MicDropDown(sf::SoundRecorder* recorder) : rec(recorder), text_label_flags(0),
		isOpen(false), devs_c_style(0), selected(0)
	{
		text_label_flags |= ImGuiInputTextFlags_ReadOnly;
	}

	MicDropDown::~MicDropDown()
	{
		
		if (!devs.empty())
		{
			for (auto it = devs.begin(); it != devs.end(); it++)
			{
				delete[] it->second;
			}
		}
		devs.clear();

		if (devs_c_style)
			delete[] devs_c_style;

	}

	std::string MicDropDown::get_current()
	{
		for (auto it = devs.begin();it != devs.end();it++)
		{
			if (it->second == devs_c_style[selected])
				return it->first;
		}
		return rec->getDefaultDevice();
	}

	void MicDropDown::get_devices()
	{
		/* clear prev devs map */
		if (!devs.empty())
		{
			for (auto it = devs.begin(); it != devs.end(); it++)
			{
				delete[] it->second;
			}
		}
		devs.clear();

		if (devs_c_style)
			delete[] devs_c_style;

		std::vector<std::string> d = rec->getAvailableDevices();

		/* check if current dev not found */
		bool isNotFound = true;
		for (auto it = d.begin(); it != d.end(); it++)
		{
			if (rec->getDevice() == *it)
			{
				isNotFound = false;
				break;
			}
				
		}
		if (isNotFound)
			rec->setDevice(rec->getDefaultDevice());
		
		/* clear array of ptrs to short_names */
		devs_c_style = new char*[d.size()];

		/* fills devs map <full_name,short_name> */
		int selected_size = 0;
		for (int i = 0; i < d.size(); i++)
		{
			
			int s = d[i].find('(') + 1;
			int e = d[i].find(')');

			if (d[i] == rec->getDevice())
			{
				selected_size = e - s + 1;
				selected = i;
			}
				


			devs[d[i]] = new char[e - s + 1];
			memcpy(devs[d[i]], d[i].substr(s, e - s).c_str(), e - s + 1);

			devs_c_style[i] = devs[d[i]];

		}

		/* set current uses audio interface */
		memcpy(input_device, devs[rec->getDevice()], selected_size);
		
	}

	bool MicDropDown::render()
	{
		int field_size = (ImGui::GetWindowSize().x - (ImGui::GetCursorPosX()) * 2) - 50;
		ImVec2 cur_pos(ImGui::GetCursorPosX(), ImGui::GetCursorPosY());

		ImGui::SetNextItemWidth(field_size);
		
		ImGui::InputText("##",input_device, rec->getDevice().size(),text_label_flags);
		ImVec2 size_last_element = ImGui::GetItemRectSize();
		

		ImGui::SetCursorPos(ImVec2(cur_pos.x + field_size, cur_pos.y));

		if(ImGui::ArrowButton("dropdown_btn", isOpen ? 2 : 3))
		{
			isOpen = !isOpen;
		}

		if(isOpen)
		{
			ImGui::SetCursorPos(ImVec2(cur_pos.x, cur_pos.y+size_last_element.y));
			ImGui::SetNextItemWidth(field_size);
			int sel = selected;
			if(ImGui::ListBox("##", &sel, devs_c_style, devs.size()))
			{
				if(sel != selected)
				{
					/* set current uses audio interface */
					memcpy(input_device, devs_c_style[sel], 100);
					selected = sel;
					isOpen = false;
					return 1;
				}
				
			}
		}

		return 0;
		
		
	}
}