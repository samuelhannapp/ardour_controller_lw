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
	//init_plugin_routing();
	window = new wxFrame(NULL, wxID_ANY, "Plugin Router", wxDefaultPosition, wxSize(600, 400));
	button_layout = new wxBoxSizer(wxHORIZONTAL);
	main_layout = new wxBoxSizer(wxVERTICAL);

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

	button_layout->Add(plugin_name, 1);
	button_layout->Add(plugin_down, 1);
	button_layout->Add(plugin_up, 1);
	button_layout->Add(reset_table, 1);
	button_layout->Add(reset_cell, 1);
	button_layout->Add(save_plugin, 1);

	table = new wxGrid(window, wxID_ANY);
	table->CreateGrid(TABLE_ROWS, TABLE_COLS);
	main_layout->Add(button_layout);
	main_layout->Add(plugin_parameter_list, 1);
	main_layout->Add(table);
	window->SetSizerAndFit(main_layout);

	ardour = new OscSenderReceiver("127.0.0.1", 9, 3819);
	ardour_receiver_thread = new std::thread(&App::receive_ardour_data, this);
	ardour_receiver_thread->detach();
	OscMessage setup_message("/set_surface/feedback");
	setup_message.PushInt(1 << 13);
	ardour->send_data(setup_message);
	
	window->Show();
	return true;
}

void App::reset_plugin_parameter_list()
{
	plugin_parameter_list->Clear();
	//for (int i = 0; i < PLUGIN_PARAMETER_MAX_CNT; i++)
			//plugin_parameter_list->Append("default\t\t");
}

void App::plugin_parameter_selected(wxCommandEvent& event)
{
	table->SetCellValue(selected_cell, event.GetString());
	int nr = (selected_cell.GetRow() * TABLE_COLS) + selected_cell.GetCol();
	nr++;
	selected_cell.SetRow(nr / TABLE_COLS);
	selected_cell.SetCol(nr % TABLE_COLS);
	table->SetGridCursor(selected_cell);
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
	for(int r = 0; r < TABLE_ROWS; r++)
		for(int c = 0; c < TABLE_COLS; c++)
			table->SetCellValue(wxGridCellCoords(r, c), "");
}

void App::save_plugin_function(wxCommandEvent& event)
{
	std::fstream file;
	#ifdef __linux__
	std::string file_location("home/samuel/Documents/plugin_data"); 
	#endif
	#ifdef _WIN64
	std::string file_location("C:\\Users\\Samuel\\Documents\\plugin_data"); 
	#endif
	file_location.append("\\");
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
			plugin_data.push_back('\n');
		}
	file << plugin_data;
}

void App::receive_ardour_data()
{
	char buffer[1024];
	int size = 0;
	while (1) {
		OscMessage message = this->ardour->receive_data();
		if (!message.GetAddress().compare("/select/plugin/parameter/name")) {
			int position = message.get_int(0) - 1;
			std::string string(std::to_string(position + 1));
			string.push_back(' ');
			string.append(message.get_string(1));

			//if the string exist's already at the same place
			if (plugin_parameter_list->GetCount() > position)
				if (plugin_parameter_list->GetString(position).compare(string))
					continue;

			//if the string is the next position
			if (plugin_parameter_list->GetCount() == position)
				plugin_parameter_list->Append(string);

			//if the position is within the list
			if (plugin_parameter_list->GetCount() > position) {
				plugin_parameter_list->Insert(string, position);
				plugin_parameter_list->Delete(position + 1);
			}
		}

		int ctr = 0;

		if (!message.GetAddress().compare("/select/plugin/name")) {
			reset_plugin_parameter_list();
			std::string string(message.get_string(0));
			plugin_name->SetLabel(string);
				
			for (plugin_routing plugin : plugin_routing_list)
				if (!string.compare(plugin.plugin_name)) {
					break;
				}
				else
					ctr++;

			if (ctr != plugin_routing_list.size())
				for (int r = 0; r < TABLE_ROWS; r++)
					for (int c = 0; c < TABLE_COLS; c++)
						table->SetCellValue(wxGridCellCoords(r, c), plugin_routing_list.at(ctr).routing_list[(r * TABLE_COLS) + c]);
		}
	}
}


//we need a version of this function wich initializes a specific plugin, after it was saved...
//another version would be, every time a new plugin is called we look it up on the disc...
void App::init_plugin_routing()
{
	#ifdef __linux__
	std::string path("home/samuel/Documents/plugin_data"); 
	#endif
	#ifdef _WIN64
	std::string path("C:\\Users\\Samuel\\Documents\\plugin_data"); 
	#endif
	std::vector<std::string> file_names;
	for (std::filesystem::directory_entry entry : std::filesystem::directory_iterator(path))
		file_names.push_back(entry.path().string());

	std::string filename_extension(".txt");

	for (std::string directory : file_names) {
		struct plugin_routing temp;
		temp.plugin_name = directory.substr(directory.find_last_of('\\') + 1);
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
