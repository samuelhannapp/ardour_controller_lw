#include "main.hpp"
#include "OscMessage.hpp"
#include "Defines.hpp"
#include "MackieSenderReceiverUdp.hpp"

#define MAX_14_BIT 16383.0

bool MyApp::OnInit()
{
	MyFrame* frame = new MyFrame();
	frame->Show();
	return true;
}


MyFrame::MyFrame()
	: wxFrame(nullptr, wxID_ANY, "Hello World")
{

	main_layout = new wxBoxSizer(wxHORIZONTAL);

	channel[0] = new Channel(this, 0, std::string("show buses"));
	channel[1] = new Channel(this, 1, std::string("show vcas"));
	channel[2] = new Channel(this, 2, std::string("page down"));
	channel[3] = new Channel(this, 3, std::string("page up"));
	channel[4] = new Channel(this, 4, std::string("mode"));
	channel[5] = new Channel(this, 5, std::string("plugin down"));
	channel[6] = new Channel(this, 6, std::string("plugin up"));
	channel[7] = new Channel(this, 7, std::string("spill"));

	for (int i = 0; i < CHANNEL_COUNT; i++) {
		main_layout->Add(channel[i]);
	}
	this->SetSizerAndFit(main_layout);
	Bind(wxEVT_THREAD, &MyFrame::state_changed, this);

	this->mackie_sender_receiver = new MackieSenderReceiver("127.0.0.1", 14, 13);
	//this->mackie_sender_receiver->initialize_midi(1, 1);
}

Channel::Channel(wxWindow *parent, int index_input, std::string button_function)
	: wxBoxSizer(wxVERTICAL)
{
	this->fader = new wxSlider(parent, wxID_ANY, 300, 0, MAX_14_BIT, 
		wxDefaultPosition, wxDefaultSize, wxSL_VERTICAL);
	this->function_button = new wxButton(parent, wxID_ANY, button_function);
	this->function_button->SetMinSize(wxSize(100, 20));
	this->record = new wxButton(parent, wxID_ANY, "Rec", wxDefaultPosition);
	this->record->SetMinSize(wxSize(100, 20));
	this->solo = new wxButton(parent, wxID_ANY, "Solo", wxDefaultPosition, wxSize(100, 20));
	this->solo->SetMinSize(wxSize(100, 20));
	this->mute = new wxButton(parent, wxID_ANY, "Mute", wxDefaultPosition, wxSize(100, 20));
	this->mute->SetMinSize(wxSize(100, 20));
	this->select = new wxButton(parent, wxID_ANY, "Sel", wxDefaultPosition, wxSize(100, 20));
	this->select->SetMinSize(wxSize(100, 20));

	this->index = index_input;
	
	this->Add(function_button);
	this->Add(record);
	this->Add(solo);
	this->Add(mute);
	this->Add(select);
	this->Add(fader);
	
	fader->Bind(wxEVT_SLIDER, &Channel::OnSlider, this);
	this->function_button->Bind(wxEVT_BUTTON, &Channel::OnButton, this);
	this->record->Bind(wxEVT_BUTTON, &Channel::OnButton, this);
	this->solo->Bind(wxEVT_BUTTON, &Channel::OnButton, this);
	this->mute->Bind(wxEVT_BUTTON, &Channel::OnButton, this);
	this->select->Bind(wxEVT_BUTTON, &Channel::OnButton, this);
	handler = (wxEvtHandler*)parent;
}

void MyFrame::state_changed(wxThreadEvent& event)
{
	channel_message message;
	message = event.GetPayload<channel_message>();
	this->mackie_sender_receiver->send_data(message.type, message.index + 1, message.value);
	return;
}

void Channel::OnSlider(wxCommandEvent& event)
{
	channel_message message;
	message.type = controller::STRIP_VOLUME;
	message.value = ((float(event.GetInt()) / MAX_14_BIT) - 1) * - 1;
	message.index = this->index;
	//state_changed(message);
	wxThreadEvent event_1 = wxThreadEvent(wxEVT_THREAD); // No specific id
	event_1.SetPayload(message);
	
	// Add any data; sometimes the only information needed at the destination is the arrival of the event itself

	// Then post the event
	wxQueueEvent(handler, event_1.Clone());
}

void Channel::OnButton(wxCommandEvent& event)
{
	channel_message message;
	message.value = 1;
	message.index = this->index;
	wxString label(this->function_button->GetLabel());
	if (event.GetId() == this->function_button->GetId()) {
		message.type = controller::KNOB_PUSH;
	}
	if (event.GetId() == this->record->GetId()) {
		message.type = controller::REC_ENABLE;
	}
	if (event.GetId() == this->solo->GetId()) {
		message.type = controller::SOLO;
	}
	if (event.GetId() == this->mute->GetId()) {
		message.type = controller::MUTE;
	}
	if (event.GetId() == this->select->GetId()) {
		message.type = controller::SELECT;
	}

	wxThreadEvent event_1 = wxThreadEvent(wxEVT_THREAD);
	event_1.SetPayload(message);
	wxQueueEvent(handler, event_1.Clone());
	return;
}

