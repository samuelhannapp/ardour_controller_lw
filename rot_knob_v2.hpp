#pragma once

#include <wx/wx.h>

wxDECLARE_EVENT(ROTARY_KNOB_UPDATED, wxCommandEvent);

class rotary_knob : public wxWindow
{
public:
	rotary_knob(int index, wxWindow *parent,
             wxWindowID id,
             const wxPoint& pos = wxDefaultPosition,
             const wxSize& size = wxDefaultSize,
             long style = 0,
             const wxString& name = wxASCII_STR(wxPanelNameStr)
	);

	void set_value(float value);
	void set_background_colour(wxColour);
	void set_knob_colour(wxColour);

private:
	const int max_angle = 45;
	const int min_angle = -225;
	const int center_angle = -90;
	const int range_rotation = 270;
	void OnPaint(wxPaintEvent& event);
	void OnMouseEvent(wxMouseEvent& event);

	

	wxEvtHandler* event_handler;

	int index;
	int angle;
	wxPoint previous_point;
	wxPoint new_point;
	wxColour background_colour;
	wxColour knob_colour;
	wxDECLARE_EVENT_TABLE();
};


