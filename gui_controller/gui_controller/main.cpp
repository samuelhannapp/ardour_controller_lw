#include "main.hpp"
#include "OscMessage.hpp"
#include "Defines.hpp"
#include "MackieSenderReceiverUdp.hpp"

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
	this->receive_thread = new wxOscReceiveThread(this, this->mackie_sender_receiver);
	receive_thread->Run();
	Bind(wxEVT_THREAD, &MyFrame::OnThreadUpdate, this);

}

void MyFrame::OnThreadUpdate(wxThreadEvent& event)
{
	// SAFE: runs on GUI thread
	thread_message message;
	message = event.GetPayload<thread_message>();
	if (message.midi_message == false)
		state_changed(event);
	
	switch (message.midi_data[0] & 0xf0) {
	case 0x90:
		switch (mackie::button_type(message.midi_data[1] / STRIPS_PER_CONTROLLER)) {
		case mackie::RECORD:
			break;
		case mackie::SOLO:
			break;
		case mackie::MUTE:
			break;
		case mackie::SELECT:
			break;
		}
		//note on
		break;
	case 0xa0:
		//aftertouch
		break;
	case 0xb0:
		//continuous controller
		break;
	case 0xc0:
		//patch change
		break;
	case 0xd0:
		//channel preassure
		break;
	case 0xe0:
	{
		int value = ((message.midi_data[1] | (message.midi_data[2] << 7)) - MAX_14_BIT) * - 1;
		this->channel[message.midi_data[0] & 0x0f]->fader->SetValue(value);
		//pitch bend
	}
		break;
	case 0xf0:
	{
		wxString string;
		long long test_data = 0;
		for (int i = 7; i < (120 / 2); i++) {
			string.append(message.midi_data[i]);
			if (((i - 7) % 8) == 7) {
				this->channel[(i - 7) / 8]->display->SetLabel(string);
				string.erase();
			}
		}
	}
		break;
	}
	return;
}

Channel::Channel(wxWindow *parent, int index_input, std::string button_function)
	: wxBoxSizer(wxVERTICAL)
{
	this->display = new wxStaticText(parent, wxID_ANY, "ddddddd\nddddddd");
	this->display->SetFont(wxFont(wxFontInfo(15)));

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
	this->Add(display);
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
	thread_message message;
	message = event.GetPayload<thread_message>();
	if (message.midi_message == true)
		return;
	this->mackie_sender_receiver->send_data(message.type, message.index + 1, message.value);
	return;
}

void Channel::OnSlider(wxCommandEvent& event)
{
	thread_message message;
	message.midi_message = false;
	message.type = controller::STRIP_VOLUME;
	message.value = ((float(event.GetInt()) / MAX_14_BIT) - 1) * - 1;
	message.index = this->index;
	//state_changed(message);
	wxThreadEvent event_1 = wxThreadEvent(wxEVT_THREAD); // No specific id
	event_1.SetPayload(message);
	wxQueueEvent(handler, event_1.Clone());
}

void Channel::OnButton(wxCommandEvent& event)
{
	thread_message message;
	message.midi_message = false;
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

