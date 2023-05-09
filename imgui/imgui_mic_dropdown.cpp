#include "imgui_mic_dropdown.h"

namespace ImGui
{
	MicDropDown::MicDropDown(sf::SoundRecorder* recorder) : rec(recorder),devs_c(0), selected(0)
	{
		
	}

	MicDropDown::~MicDropDown()
	{
		
		if(devs_c)
		{
			for (int i = 0; i < devs.size(); i++)
			{
				delete[] devs_c[i];
			}

			
			delete[] devs_c;
			devs.clear();

		}

	}

	std::string MicDropDown::get_current()
	{
		return devs[selected];
	}

	void MicDropDown::get_devices()
	{
		if (devs_c)
		{
			for (int i = 0; i < devs.size(); i++)
			{
				delete[] devs_c[i];
			}

			delete[] devs_c;
			devs.clear();

		}

		devs = rec->getAvailableDevices();

		/* check if current dev not found */
		bool isNotFound = true;
		for (auto it = devs.begin(); it != devs.end(); it++)
		{
			if (rec->getDevice() == *it)
			{
				isNotFound = false;
				break;
			}
				
		}
		if (isNotFound)
			rec->setDevice(rec->getDefaultDevice());
		
		/* clear array of ptrs*/
		devs_c = new char*[devs.size()];

		/* fills devs map <full_name,short_name> */
		int selected_size = 0;
		for (int i = 0; i < devs.size(); i++)
		{
			
			int s = (int)(devs[i].find('(') + 1);
			int e = (int)(devs[i].find(')'));

			if (devs[i] == rec->getDevice())
			{
				selected_size = e - s + 1;
				selected = i;
			}
				


			devs_c[i] = new char[e - s + 1];
			memcpy(devs_c[i], devs[i].substr(s, e - s).c_str(), e - s + 1);


		}
		
	}

	bool MicDropDown::render()
	{
		int last = selected;
		if(ImGui::Combo("##devs_combo", &selected, devs_c, (int)devs.size()))
		{
			if(selected != last)
			{
				return 1;
			}
		}

		return 0;
		
		
	}
}