#include "rot_knob_v2.hpp"
#include <cmath>

wxDEFINE_EVENT(ROTARY_KNOB_UPDATED, wxCommandEvent);

wxBEGIN_EVENT_TABLE(rotary_knob, wxWindow)
EVT_PAINT(rotary_knob::OnPaint)
EVT_MOUSE_EVENTS(rotary_knob::OnMouseEvent)
wxEND_EVENT_TABLE()

rotary_knob::rotary_knob(int index, wxWindow* parent,
    wxWindowID id,
    const wxPoint& pos,
    const wxSize& size,
    long style,
    const wxString& name) : wxWindow(parent, id, pos, size, style, name)
{
    this->index = index;
    this->previous_point = wxPoint(0, 0);
    this->new_point = wxPoint(0, 0);
    this->angle = center_angle;
    this->event_handler = parent;
    this->background_colour = wxColor(255, 255, 255);
    this->knob_colour = wxColor(255, 255, 255);
    this->mouse_down = false;
}

void rotary_knob::OnPaint(wxPaintEvent& event)
{
    this->SetBackgroundColour(this->background_colour);
    wxPaintDC dc(this);
    wxColour colour(0, 0, 0);
    wxPen pen(colour, 3);
    pen.SetWidth(2);
    dc.SetPen(pen);
    dc.SetBrush(this->knob_colour);
    wxSize sz = GetClientSize();
    wxCoord w = 100, h = 50;
    int x = wxMax(0, (sz.x - w) / 2);
    int y = wxMax(0, (sz.y - h) / 2);
    wxRect recToDraw(x, y, w, h);
    wxPoint center(20, 20);
    int radius = 20;

    int difference = new_point.y - previous_point.y;
    int new_angle = angle - difference;
    angle = new_angle;
    if (angle < min_angle)
        angle = min_angle;
    if (angle > max_angle)
        angle = max_angle;
    double sine = sin(angle * M_PI / 180);
    double cosine = cos(angle * M_PI / 180);
    
    int y_gegenkathete = sine * radius + center.y;
    int x_ankathete = cosine * radius + center.x;
    wxPoint point_2(x_ankathete, y_gegenkathete);

    dc.DrawCircle(center, radius);
    dc.DrawLine(center, point_2);
    

    wxCommandEvent command_event(ROTARY_KNOB_UPDATED);
    command_event.SetInt(angle);
    command_event.SetId(this->index);

    wxPostEvent(this->event_handler, command_event);
}

void rotary_knob::OnMouseEvent(wxMouseEvent& event)
{
	wxPoint ptM(event.GetPosition());
    if (event.LeftDown()) {
            CaptureMouse();
            this->mouse_down = true;
    }
    if (event.LeftUp()) {
            ReleaseMouse();
            this->mouse_down = false;
    }

    if (event.LeftIsDown())
    {
        previous_point = new_point;
        new_point = ptM;
            
        this->Refresh();
    }
    else {
        previous_point = ptM;
        new_point = ptM;
    }

	if (event.RightIsDown())
	{
	}
}

void rotary_knob::set_value(float value)
{
    if (this->mouse_down == true)
        return;
    float temp_angle = range_rotation - (value * this->range_rotation);
    temp_angle = this->max_angle - temp_angle;
    this->angle = temp_angle;
    this->Refresh();
}

void rotary_knob::set_background_colour(wxColour colour)
{
    this->background_colour = colour;
}
void rotary_knob::set_knob_colour(wxColour colour)
{
    this->knob_colour = colour;
}







