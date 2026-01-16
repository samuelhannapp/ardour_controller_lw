#pragma once
#include <wx/wx.h>
#include <wx/thread.h>
#include "MackieSenderReceiverUdp.hpp"
#include "wxOscReceiveThread.h"

#define CHANNEL_COUNT 8

class MyApp : public wxApp
{
public:
	bool OnInit() override;
};

wxIMPLEMENT_APP(MyApp);




class Channel : public wxBoxSizer
{
public:
	Channel(wxWindow* parent, int index_input, std::string button_function);
private:
	int index;
	void OnSlider(wxCommandEvent& event);
	void OnButton(wxCommandEvent& event);
	wxEvtHandler* handler;
public:
	wxStaticText* display;
	wxSlider* fader;
	wxButton* function_button;
	wxButton* record;
	wxButton* solo;
	wxButton* mute;
	wxButton* select;
};

class MyFrame : public wxFrame
{
public:
	MyFrame();
	wxBoxSizer* main_layout;
	Channel* channel[CHANNEL_COUNT];
	void state_changed(wxThreadEvent& event);
	MackieSenderReceiver *mackie_sender_receiver;
	wxOscReceiveThread* receive_thread;
	void OnThreadUpdate(wxThreadEvent& event);
};

