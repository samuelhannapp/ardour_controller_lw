#include "main.hpp"

#define MAX_14_BIT 16383

bool MyApp::OnInit()
{
	MyFrame* frame = new MyFrame();
	frame->Show();
	return true;
}


MyFrame::MyFrame()
	: wxFrame(nullptr, wxID_ANY, "Hello World")
{
	channel_1 = new Channel(this);
	main_layout = new wxBoxSizer(wxHORIZONTAL);
	main_layout->Add(channel_1);
	this->SetSizer(main_layout);
	//Bind(wxEVT_ANY, &MyFrame::state_changed, channel_1);
}


Channel::Channel(wxWindow *parent)
	: wxBoxSizer(wxVERTICAL)
{
	this->fader = new wxSlider(parent, wxID_ANY, 300, 0, MAX_14_BIT, 
		wxDefaultPosition, wxDefaultSize, wxSL_VERTICAL);
	this->record = new wxButton(parent, wxID_ANY, "Rec");
	this->solo = new wxButton(parent, wxID_ANY, "Solo");
	this->mute = new wxButton(parent, wxID_ANY, "Mute");
	this->select = new wxButton(parent, wxID_ANY, "Sel");

	this->Add(record);
	this->Add(solo);
	this->Add(mute);
	this->Add(select);
	this->Add(fader);

	this->index = 0;
	
	fader->Bind(wxEVT_SLIDER, &Channel::OnSlider, this);
	handler = parent;

}

void MyFrame::state_changed(wxCommandEvent& event)
{
		
}

void Channel::OnSlider(wxCommandEvent& event)
{
	channel_message message;
	message.type = gui_controller::channel_component::fader;
	message.value = event.GetInt();
	message.index = this->index;
	//state_changed(message);
	wxCommandEvent event_1 = wxCommandEvent(); // No specific id
	
	// Add any data; sometimes the only information needed at the destination is the arrival of the event itself

	// Then post the event
	wxQueueEvent(handler, event_1.Clone());
}

