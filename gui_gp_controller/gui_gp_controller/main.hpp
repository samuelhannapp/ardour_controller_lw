#pragma once

#include <wx/wx.h>
#include <wx/thread.h>
#include "wxOscReceiveThread.h"
#include "OscSenderReceiver.hpp"
#include "PluginMultiplexer.hpp"
#include <vector>
#include "rotary_knob_modul.hpp"

#define CHANNEL_COUNT 8
#define MAX_PLUGIN_PARAMETERS 200

class MyApp : public wxApp
{
public:
	bool OnInit() override;
};

wxIMPLEMENT_APP(MyApp);


int bank = 0;

struct instance {
	//wxStaticText* name;
	//wxSlider* fader;
	rotary_knob_modul *rotary_knob;
	int index;
	wxEvtHandler* handler;
	void OnSlider(wxCommandEvent& event);
};

#define BANK_SIZE 2
#define CONTROLLER_SIZE 16

class MyFrame : public wxFrame
{
public:
	MyFrame(std::string udp_input_port, std::string udp_output_port);
	wxStaticText* plugin_name;
	wxButton* bank_up;
	wxButton* bank_down;
	wxButton* plugin_up;
	wxButton* plugin_down;
	void bank_up_function(wxCommandEvent& event);
	void bank_down_function(wxCommandEvent& event);
	void plugin_down_function(wxCommandEvent& event);
	void plugin_up_function(wxCommandEvent& event);
	wxBoxSizer* button_layout;
	wxBoxSizer* label_layout;
	wxBoxSizer* fader_layout;
	wxBoxSizer* main_layout;
	OscSenderReceiver* osc_sender_receiver;
	wxOscReceiveThread* receive_thread;
	void OnThreadUpdate(wxThreadEvent& event);
	plugin_parameter selected_plugin[MAX_PLUGIN_PARAMETERS];
	instance controller[CONTROLLER_SIZE];
	void update_controller();
	plugin_multiplexer_struct* plugin_multiplexer;
	std::string selected_plugin_name;
	std::vector<std::string> plugin_list;
	void OnRotary_Knob_Event(wxCommandEvent& event);
	wxDECLARE_EVENT_TABLE();
};
