#include "rotary_knob_modul.hpp"
#include "rot_knob_v2.hpp"


rotary_knob_modul::rotary_knob_modul(int index, wxWindow* parent,
    wxWindowID id,
    const wxPoint& pos,
    const wxSize& size,
    long style,
    const wxString& name) : wxBoxSizer(wxVERTICAL)
{
    this->knob = new rotary_knob(index, parent, id, pos, size, style, name);
    wxSize panel_size = size;
    panel_size.y = 20;
    this->panel = new wxPanel(parent, wxID_ANY, wxDefaultPosition, panel_size, wxBORDER_SUNKEN);
    this->text = new wxStaticText(this->panel, wxID_ANY, "test", wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER);
    this->Add(knob);
    this->Add(panel);

}

void rotary_knob_modul::set_value(float value)
{
    this->knob->set_value(value);
}

void rotary_knob_modul::set_text(std::string modul_text)
{
    this->text->SetLabel(modul_text);
}

void rotary_knob_modul::set_text_color(wxColour colour)
{
    this->panel->SetBackgroundColour(colour);
}

void rotary_knob_modul::set_knob_colour(wxColour colour)
{
    this->knob->set_knob_colour(colour);

}
void rotary_knob_modul::set_background_colour(wxColour colour)
{
    this->knob->set_background_colour(colour);
}
