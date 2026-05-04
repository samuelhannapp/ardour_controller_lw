#include <wx/wx.h>
#include "main.h"
#include "OscMessage.hpp"
#include <string>
#include <fstream>
#include <iostream>
#include <filesystem>

#define PLUGIN_PARAMETER_MAX_CNT 100
#define TABLE_ROWS 4
#define TABLE_COLS 8

bool App::OnInit() {
	plugin_multiplexer_obj = new plugin_multiplexer_c;
	//init_plugin_routing();
	window = new wxFrame(NULL, wxID_ANY, "Plugin Router", wxDefaultPosition, wxSize(800, 400));
	button_layout = new wxBoxSizer(wxHORIZONTAL);
	main_layout = new wxBoxSizer(wxVERTICAL);

	rgb_mixer = new rgb_control_set(window);

	selected_cell = wxGridCellCoords(0, 0);

	plugin_name = new wxStaticText(window, wxID_ANY, "plugin_name\t\t");
	
	plugin_name->SetMinSize(wxSize(140, plugin_name->GetSize().GetHeight()));
	plugin_down = new wxButton(window, wxID_ANY, "plugin down");
	plugin_up = new wxButton(window, wxID_ANY, "plugin up");
	reset_table = new wxButton(window, wxID_ANY, "reset table");
	reset_cell = new wxButton(window, wxID_ANY, "reset_cell");
	save_plugin = new wxButton(window, wxID_ANY, "save_plugin");

	plugin_parameter_list = new wxListBox(window, wxID_ANY);
	plugin_parameter_list->SetMinSize(wxSize(200, plugin_parameter_list->GetSize().GetHeight()));

	Connect(wxEVT_COMMAND_LISTBOX_SELECTED, wxCommandEventHandler(App::plugin_parameter_selected));
	Connect(wxEVT_GRID_CELL_LEFT_CLICK, wxGridEventHandler(App::cell_selected));
	plugin_up->Bind(wxEVT_BUTTON, &App::plugin_up_function, this);
	plugin_down->Bind(wxEVT_BUTTON, &App::plugin_down_function, this);
	reset_cell->Bind(wxEVT_BUTTON, &App::reset_cell_function, this);
	reset_table->Bind(wxEVT_BUTTON, &App::reset_table_function, this);
	save_plugin->Bind(wxEVT_BUTTON, &App::save_plugin_function, this);

	rgb_mixer->set_color_button->Bind(wxEVT_BUTTON, &App::color_selected, this);

	button_layout->Add(plugin_name, 1);
	button_layout->Add(plugin_down, 1);
	button_layout->Add(plugin_up, 1);
	button_layout->Add(reset_table, 1);
	button_layout->Add(reset_cell, 1);
	button_layout->Add(save_plugin, 1);

	table = new wxGrid(window, wxID_ANY);
	table->CreateGrid(TABLE_ROWS, TABLE_COLS);
	main_layout->Add(button_layout);
	main_layout->Add(rgb_mixer->rgb_layout);
	main_layout->Add(plugin_parameter_list, 1);
	main_layout->Add(table);
	window->SetSizerAndFit(main_layout);

	ardour = new OscSenderReceiver("127.0.0.1", 9, 3819);
	//ardour_receiver_thread = new std::thread(&App::receive_ardour_data, this);
	//ardour_receiver_thread->detach();
	wx_osc_receive_thread = new wxOscReceiveThread(this, this->ardour);
	wx_osc_receive_thread->Run();
	Bind(wxEVT_THREAD, &App::OnThreadUpdate, this);
	OscMessage setup_message("/set_surface/feedback");
	setup_message.PushInt(1 << 13);
	ardour->send_data(setup_message);
	
	window->Show();
	return true;
}


void App::OnThreadUpdate(wxThreadEvent& event)
{
	OscMessage message = event.GetPayload<OscMessage>();

	if (!message.GetAddress().compare("/select/plugin/parameter/name")) {
		int position = message.get_int(0) - 1;
		std::string string(std::to_string(position + 1));
		string.push_back(' ');
		string.append(message.get_string(1));

		//if the string exist's already at the same place
		if (plugin_parameter_list->GetCount() > position)
			if (plugin_parameter_list->GetString(position).compare(string))
				return;

		//if the string is the next position
		if (plugin_parameter_list->GetCount() == position)
			plugin_parameter_list->Append(string);

		//if the position is within the list
		if (plugin_parameter_list->GetCount() > position) {
			plugin_parameter_list->Insert(string, position);
			plugin_parameter_list->Delete(position + 1);
		}
	}
	if (!message.GetAddress().compare("/select/plugin/name")) {
		reset_plugin_parameter_list();
		reset_table_function();
		std::string plugin_name_string(message.get_string(0));
		plugin_name->SetLabel(plugin_name_string);

		plugin_multiplexer_obj->setup(plugin_name_string);


		for (int r = 0; r < TABLE_ROWS; r++)
			for (int c = 0; c < TABLE_COLS; c++) {
				int index = (r * TABLE_COLS) + c;
				table->SetCellValue(wxGridCellCoords(r, c), std::to_string(plugin_multiplexer_obj->get_controller_to_plugin(index)));
				struct rgb_colour colour = plugin_multiplexer_obj->get_knob_colour(index);
				wxColour wx_colour(colour.r, colour.g, colour.b);
				table->SetCellBackgroundColour(r, c, wx_colour);
			}
	}
}

void App::reset_plugin_parameter_list()
{
	plugin_parameter_list->Clear();
	//for (int i = 0; i < PLUGIN_PARAMETER_MAX_CNT; i++)
			//plugin_parameter_list->Append("default\t\t");
}

void App::increment_seleced_cell()
{
	int nr = (selected_cell.GetRow() * TABLE_COLS) + selected_cell.GetCol();
	nr++;
	selected_cell.SetRow(nr / TABLE_COLS);
	selected_cell.SetCol(nr % TABLE_COLS);
	table->SetGridCursor(selected_cell);
}

void App::plugin_parameter_selected(wxCommandEvent& event)
{
	table->SetCellValue(selected_cell, event.GetString());
	increment_seleced_cell();
}

void App::cell_selected(wxGridEvent& event)
{
	selected_cell = wxGridCellCoords(event.GetRow(), event.GetCol());
	event.Skip();
}

void App::plugin_up_function(wxCommandEvent& event)
{
	OscMessage message("/select/plugin");
	message.PushFloat(1);
	
	ardour->send_data(message);	
}

void App::plugin_down_function(wxCommandEvent& event)
{
	OscMessage message("/select/plugin");
	message.PushFloat(- 1);
	ardour->send_data(message);	
}

void App::reset_cell_function(wxCommandEvent& event)
{
	table->SetCellValue(selected_cell, "");
}

void App::reset_table_function(wxCommandEvent& event)
{
	reset_table_function();	
}

void App::reset_table_function()
{
	for(int r = 0; r < TABLE_ROWS; r++)
		for(int c = 0; c < TABLE_COLS; c++)
			table->SetCellValue(wxGridCellCoords(r, c), "");
}

void App::save_plugin_function(wxCommandEvent& event)
{
	std::fstream file;
	#ifdef __linux__
	std::string file_location("plugin_data"); 
	file_location.append("/");
	#endif
	#ifdef _WIN64
	std::string file_location("C:\\Users\\Samuel\\Software\\ArdourOscController_Build\\windows_build\\plugin_data"); 
	file_location.append("\\");
	#endif
	file_location.append(plugin_name->GetLabelText());
	file_location.append(".txt");
	file.open(file_location, std::ios::out);
	std::string plugin_data;
	for(int r = 0; r < TABLE_ROWS; r++)
		for (int c = 0; c < TABLE_COLS; c++) {

			std::string content(table->GetCellValue(wxGridCellCoords(r, c)).c_str());

			if (content.size())
				plugin_data.append(content);
			else
				plugin_data.push_back('0');
			
			int data[3];
			//this->rgb_mixer->get_rgb(data);
			wxColour cell_colour = table->GetCellBackgroundColour(r, c);
			plugin_data.push_back(' ');
			plugin_data.append(std::to_string(cell_colour.Red()));
			plugin_data.push_back(' ');
			plugin_data.append(std::to_string(cell_colour.Green()));
			plugin_data.push_back(' ');
			plugin_data.append(std::to_string(cell_colour.Blue()));

			plugin_data.push_back('\n');
		}
	file << plugin_data;

	file.close();
	this->plugin_multiplexer_obj->initialize_plugin_multiplexer();
}

void App::color_selected(wxCommandEvent& event)
{
	table->SetCellBackgroundColour(this->selected_cell.GetRow(), this->selected_cell.GetCol(),  this->rgb_mixer->set_color_button->GetBackgroundColour());
	increment_seleced_cell();
}

//we need a version of this function wich initializes a specific plugin, after it was saved...
//another version would be, every time a new plugin is called we look it up on the disc...
/*
void App::init_plugin_routing()
{
	if(plugin_routing_list.size())
		plugin_routing_list.clear();

	#ifdef __linux__
	std::string path("/home/samuel/Documents/plugin_data"); 
	#endif
	#ifdef _WIN64
	std::string path("plugin_data"); 
	#endif
	std::vector<std::string> file_names;
	for (std::filesystem::directory_entry entry : std::filesystem::directory_iterator(path))
		file_names.push_back(entry.path().string());

	std::string filename_extension(".txt");

	for (std::string directory : file_names) {
		struct plugin_routing temp;
		#ifdef _WIN64
		temp.plugin_name = directory.substr(directory.find_last_of('\\') + 1);
		#endif
		#ifdef __linux__
		temp.plugin_name = directory.substr(directory.find_last_of('/') + 1);
		#endif
		temp.plugin_name.erase(temp.plugin_name.size() - filename_extension.size(), filename_extension.size());
		plugin_routing_list.push_back(temp);
	}
	
	int ctr = 0;
	for (std::string directory : file_names) {
		std::ifstream file(directory);
		std::string line;
		
		for (int i = 0; i < 32; i++) {
			std::getline(file, line);
			plugin_routing_list.at(ctr).routing_list[i] = line;
		}
		ctr++;
	}
	return;
}
*/
