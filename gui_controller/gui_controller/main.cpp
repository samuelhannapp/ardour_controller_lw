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
	
	fader->Bind(wxEVT_SLIDER, &Channel::OnSlider, parent);
}

void MyFrame::state_changed(channel_message message)
{
		
}

void Channel::OnSlider(wxCommandEvent& event)
{
	channel_message message;
	message.type = channel_component::fader;
	message.value = event.GetInt();
	message.index = this->index;
	state_changed(message);
}

