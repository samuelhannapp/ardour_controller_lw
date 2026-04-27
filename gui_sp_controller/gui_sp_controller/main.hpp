#pragma once
#include <wx/wx.h>
#include <wx/thread.h>
#include "wxOscReceiveThread.h"
#include "OscSenderReceiver.hpp"
#include "PluginMultiplexer.hpp"

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
	wxSlider* fader;
	int index;
	wxEvtHandler* handler;
	void OnSlider(wxCommandEvent& event);
};

#define BANK_SIZE 2
#define CONTROLLER_SIZE 12 

class MyFrame : public wxFrame
{
public:
	MyFrame(std::string udp_input_port, std::string udp_output_port);
private:
	wxBoxSizer* fader_layout;
	wxBoxSizer* main_layout;
	OscSenderReceiver* osc_sender_receiver;
	wxOscReceiveThread* receive_thread;
	void OnThreadUpdate(wxThreadEvent& event);
	instance controller[CONTROLLER_SIZE];
	void update_controller();
	plugin_multiplexer_c* plugin_multiplexer;
	std::string m_sp_controller_plugin_name;
	std::string previous_selected_plugin_name;
	std::string plugin_name_before_increment;
	std::string selected_plugin_name;
};
