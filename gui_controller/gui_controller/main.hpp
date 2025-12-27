#pragma once
#include <wx/wx.h>

class MyApp : public wxApp
{
public:
	bool OnInit() override;
};

wxIMPLEMENT_APP(MyApp);

enum channel_component {
	fader, record, solo, mute, select
};

struct channel_message {
	enum channel_component type;
	int value;
	int index;
};

class Channel : public wxBoxSizer
{
public:
	Channel(wxWindow *parent);
private:
	int index;
	wxSlider* fader;
	wxButton* record;
	wxButton* solo;
	wxButton* mute;
	wxButton* select;
	void OnSlider(wxCommandEvent& event);
};

class MyFrame : public wxFrame
{
public:
	MyFrame();
	wxBoxSizer* main_layout;
	Channel* channel_1;
	void state_changed(channel_message message);
};
