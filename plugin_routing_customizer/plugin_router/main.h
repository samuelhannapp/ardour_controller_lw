#pragma once
#include <wx/wx.h>
#include <wx/grid.h>
#include "OscSenderReceiver.hpp"
#include <thread>
#include <vector>
#include "wxOscReceiveThread.h"
#include "PluginMultiplexer.hpp"
#include <wx/spinctrl.h>
/*
struct plugin_routing {
	std::string plugin_name;
	std::string routing_list[32];
};
*/

class rgb_control {
public:
	rgb_control(wxFrame *parent, wxString name)
	{
		this->label = new wxStaticText(parent, wxID_ANY, name);
		this->number = new wxSpinCtrl(parent, wxID_ANY, "5", wxDefaultPosition, wxDefaultSize, 16384L, 0, 255, 5);
		this->slider = new wxSlider(parent, wxID_ANY, 20, 0, 255);
		this->layout = new wxBoxSizer(wxHORIZONTAL);
		this->layout->Add(label);
		this->layout->Add(number);
		this->layout->Add(slider);
		this->slider->Bind(wxEVT_SLIDER, &rgb_control::OnSlider, this);
	}

	wxBoxSizer* layout;
	wxSlider* slider;

private:	
	wxStaticText* label;
	wxSpinCtrl* number;

	void OnSlider(wxCommandEvent& event)
	{
		this->number->SetValue(event.GetInt());
	}
};

class rgb_control_set {
public:
	rgb_control_set(wxFrame* parent)
	{
		this->mixer[0] = new rgb_control(parent, "R");
		this->mixer[1] = new rgb_control(parent, "G");
		this->mixer[2] = new rgb_control(parent, "B");

		this->mixer[0]->slider->Bind(wxEVT_SLIDER, &rgb_control_set::OnSlider, this);
		this->mixer[1]->slider->Bind(wxEVT_SLIDER, &rgb_control_set::OnSlider, this);
		this->mixer[2]->slider->Bind(wxEVT_SLIDER, &rgb_control_set::OnSlider, this);

		set_color_button = new wxButton(parent, wxID_ANY, "set colour");

		rgb_layout = new wxBoxSizer(wxVERTICAL);
		rgb_layout->Add(this->mixer[0]->layout);
		rgb_layout->Add(this->mixer[1]->layout);
		rgb_layout->Add(this->mixer[2]->layout);
		rgb_layout->Add(set_color_button);
	}

	wxBoxSizer* rgb_layout;
	wxButton* set_color_button;
	void get_rgb(int *data )
	{
		data[0] = this->mixer[0]->slider->GetValue();
		data[1] = this->mixer[1]->slider->GetValue();
		data[2] = this->mixer[2]->slider->GetValue();
	}
private:
	rgb_control* mixer[3];

	void OnSlider(wxCommandEvent& event) {
		int r = this->mixer[0]->slider->GetValue();
		int g = this->mixer[1]->slider->GetValue();
		int b = this->mixer[2]->slider->GetValue();
		this->set_color_button->SetBackgroundColour(wxColor(r, g, b));
	}
};

class App : public wxApp {
public:
	bool OnInit();
	wxFrame* window;
	wxBoxSizer* button_layout;
	wxBoxSizer* rgb_layout;
	wxBoxSizer* main_layout;
	wxStaticText* plugin_name;
	wxButton* plugin_down;
	wxButton* plugin_up;
	wxButton* reset_table;
	wxButton* reset_cell;
	wxButton* save_plugin;
	wxListBox* plugin_parameter_list;
	wxGrid* table;

	rgb_control_set* rgb_mixer;

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
	void color_selected(wxCommandEvent& event);

	struct plugin_multiplexer_c *plugin_multiplexer_obj;

	//std::vector<plugin_routing> plugin_routing_list;
	//void init_plugin_routing();
	void reset_plugin_parameter_list();
};

wxIMPLEMENT_APP(App);
