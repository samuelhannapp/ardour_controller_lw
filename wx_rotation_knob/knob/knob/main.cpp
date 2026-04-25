#include "main.hpp"
#include <wx/wx.h>


wxBEGIN_EVENT_TABLE(MyFrame, wxFrame)
EVT_COMMAND(wxID_ANY, ROTARY_KNOB_UPDATED, MyFrame::OnRotary_Knob_Event)
wxEND_EVENT_TABLE()

bool MyApp::OnInit()
{
	MyFrame* frame;
	frame = new MyFrame();

	frame->Show();
	return true;
}

MyFrame::MyFrame()
	: wxFrame(nullptr, wxID_ANY, "rotary_knob", wxDefaultPosition, wxSize(800, 400))
{
	text_test = new wxStaticText(this, wxID_ANY, "test");
	main_layout = new wxBoxSizer(wxVERTICAL);
	knob_layout = new wxGridSizer(4, 8, wxSize(1, 1));

	for (int row = 0; row < 4; row++) 
		for(int col = 0; col < 8; col++){
			knob_test[col][row] = new rotary_knob_modul(col + row * 8, this, wxID_ANY, wxDefaultPosition, wxSize(100, 50), wxBORDER_RAISED);
			knob_layout->Add(knob_test[col][row]);
		}
	
	main_layout->Add(text_test);
	main_layout->Add(knob_layout);
	this->SetSizerAndFit(main_layout);

	this->knob_test[0][3]->set_value(0.3);
	this->knob_test[0][0]->set_text("eq high pass");
	this->knob_test[0][0]->set_text_color(wxColour(10, 140, 10));
	this->knob_test[0][0]->set_background_colour(wxColour(20, 170, 10));
	this->knob_test[0][0]->set_knob_colour(wxColour(30, 200, 40));
}

void MyFrame::OnRotary_Knob_Event(wxCommandEvent &event)
{
	int data = event.GetInt();
	int id = event.GetId();
	std::string text;
	text.append(std::to_string(id));
	text.append(" ");
	text.append(std::to_string(data));
	

	this->text_test->SetLabel(text);
}
