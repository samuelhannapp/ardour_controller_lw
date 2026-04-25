#pragma once
#include <wx/wx.h>
#include "rotary_knob_modul.hpp"

class MyApp : public wxApp
{
public:
	bool OnInit() override;
};



class MyFrame : public wxFrame
{
public:
	MyFrame();
	rotary_knob_modul *knob_test[8][4];
	wxGridSizer *knob_layout;
	wxStaticText* text_test;
	wxBoxSizer* main_layout;
	void OnSize(wxSizeEvent& event);
	void OnRotary_Knob_Event(wxCommandEvent &event);
	wxDECLARE_EVENT_TABLE();

};

wxIMPLEMENT_APP(MyApp);

