#pragma once
#include <wx/wx.h>
#include <wx/grid.h>
#include "OscSenderReceiver.hpp"
#include <thread>
#include <vector>
#include "wxOscReceiveThread.h"
#include "PluginMultiplexer.hpp"
/*
struct plugin_routing {
	std::string plugin_name;
	std::string routing_list[32];
};
*/

class App : public wxApp {
public:
	bool OnInit();
	wxFrame* window;
	wxBoxSizer* button_layout;
	wxBoxSizer* main_layout;
	wxStaticText* plugin_name;
	wxButton* plugin_down;
	wxButton* plugin_up;
	wxButton* reset_table;
	wxButton* reset_cell;
	wxButton* save_plugin;
	wxListBox* plugin_parameter_list;
	wxGrid* table;
	void plugin_parameter_selected(wxCommandEvent& event);
	void cell_selected(wxGridEvent& event);
	wxGridCellCoords selected_cell;

	OscSenderReceiver *ardour;
	//std::thread *ardour_receiver_thread;
	wxOscReceiveThread* wx_osc_receive_thread;
	void receive_ardour_data();
	void OnThreadUpdate(wxThreadEvent& event);
	void plugin_up_function(wxCommandEvent& event);
	void plugin_down_function(wxCommandEvent& event);
	void reset_cell_function(wxCommandEvent& event);
	void reset_table_function(wxCommandEvent& event);
	void reset_table_function();
	void save_plugin_function(wxCommandEvent& event);

	struct plugin_multiplexer_c *plugin_multiplexer_obj;

	//std::vector<plugin_routing> plugin_routing_list;
	//void init_plugin_routing();
	void reset_plugin_parameter_list();
};

wxIMPLEMENT_APP(App);
