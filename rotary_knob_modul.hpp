#pragma once

#include <wx/wx.h>
#include "rot_knob_v2.hpp"
class rotary_knob_modul : public wxBoxSizer
{
public:
	rotary_knob_modul(int index, wxWindow *parent,
             wxWindowID id,
             const wxPoint& pos = wxDefaultPosition,
             const wxSize& size = wxDefaultSize,
             long style = 0,
             const wxString& name = wxASCII_STR(wxPanelNameStr)
	);
    void set_value(float);
    void set_text(std::string);
    void set_text_color(wxColour);
    void set_knob_colour(wxColour);
    void set_background_colour(wxColour);
private:
    rotary_knob* knob;
	wxPanel *panel;
	wxStaticText *text;

};

