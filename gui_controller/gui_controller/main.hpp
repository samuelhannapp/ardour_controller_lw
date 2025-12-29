#pragma once
#include <wx/wx.h>

//wxDEFINE_EVENT(CHANNEL_EVENT, wxCommandEvent);

#define CHANNEL_COUNT 8

class MyApp : public wxApp
{
public:
	bool OnInit() override;
};

wxIMPLEMENT_APP(MyApp);

namespace gui_controller {
	enum channel_component {
		fader, record, solo, mute, select
	};
}

struct channel_message {
	enum gui_controller::channel_component type;
	int value;
	int index;
};

class Channel : public wxBoxSizer
{
public:
	Channel(wxWindow* parent, int index_input);
private:
	int index;
	wxSlider* fader;
	wxButton* record;
	wxButton* solo;
	wxButton* mute;
	wxButton* select;
	void OnSlider(wxCommandEvent& event);
	wxEvtHandler* handler;
};

class MyFrame : public wxFrame
{
public:
	MyFrame();
	wxBoxSizer* main_layout;
	Channel* channel[CHANNEL_COUNT];
	void state_changed(wxThreadEvent& event);
};


/*
wxBEGIN_EVENT_TABLE(MyFrame, wxFrame)
EVT_COMMAND(wxID_ANY, CHANNEL_EVENT, MyFrame::state_changed)
wxEND_EVENT_TABLE()
*/
