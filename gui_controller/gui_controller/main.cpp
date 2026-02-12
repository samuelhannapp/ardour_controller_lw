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
	: wxFrame(nullptr, wxID_ANY, "Mackie Control Gui Simulation (base_controller)")
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
	//Bind(wxEVT_THREAD, &MyFrame::state_changed, this);

	this->mackie_sender_receiver = new MackieSenderReceiver("127.0.0.1", 14, 13);
	//this->mackie_sender_receiver->initialize_midi(1, 1);
	this->receive_thread = new wxOscReceiveThread(this, this->mackie_sender_receiver);
	receive_thread->Run();
	Bind(wxEVT_THREAD, &MyFrame::OnThreadUpdate, this);

}

MidiMessage_v2 MyFrame::get_midi_data(OscMessage osc_message)
{
	MidiMessage_v2 message;
	if (osc_message.GetTypeList().at(0) == 'i') {
		message.midi_data_size = 3;
		int midi_data = osc_message.get_int(0);
		message.midi_data[0] = (unsigned char)(midi_data & 0xff);
		message.midi_data[1] = (unsigned char)((midi_data & 0xff00) >> 8);
		message.midi_data[2] = (unsigned char)((midi_data & 0xff0000) >> 16);
	}
	if (osc_message.GetTypeList().at(0) == 'h') {
		message.midi_data_size = 120;
		if (osc_message.GetTypeList().size() == 15)
			for (int i = 0; i < 15; i++){
				long long temp = osc_message.get_long_long(i);
				char char_data[8] = { 0 };
				char_data[7] = (long long)(temp & (long long)(0xff) << 56) >> 56;
				char_data[6] = (long long)(temp & (long long)(0xff) << 48) >> 48;
				char_data[5] = (long long)(temp & (long long)(0xff) << 40) >> 40;
				char_data[4] = (long long)(temp & (long long)(0xff) << 32) >> 32;
				char_data[3] = (temp & (long long)(0xff << 24)) >> 24;
				char_data[2] = (temp & (long long)(0xff << 16)) >> 16;
				char_data[1] = (temp & (long long)(0xff << 8)) >> 8;
				char_data[0] = (temp & (long long)(0xff << 0)) >> 0;
				for (int o = 0; o < 8; o++)
					message.midi_data[o + 8 * i] = char_data[o];
			}
	}
	return message;
}

void MyFrame::OnThreadUpdate(wxThreadEvent& event)
{
	// SAFE: runs on GUI thread
	OscMessage osc_message = event.GetPayload<OscMessage>();	

	if(!osc_message.GetAddress().compare("/Midi"))
		process_midi_data(event);
	else
		process_gui_controller_data(event);
	return;
}

Channel::Channel(wxWindow *parent, int index_input, std::string button_function)
	: wxBoxSizer(wxVERTICAL)
{
	this->display = new wxStaticText(parent, wxID_ANY, "ddddddd\nddddddd");
	this->display->SetFont(wxFont(wxFontInfo(15)));
	this->display->SetMinSize(wxSize(100, 50));

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

void MyFrame::process_gui_controller_data(wxThreadEvent& event)
{
	OscMessage osc_message = event.GetPayload<OscMessage>();
	if (!osc_message.GetAddress().compare("/wxSlider")) 
		this->mackie_sender_receiver->send_data(controller::STRIP_VOLUME, osc_message.get_int(0) + 1, osc_message.get_float(1));
	if (!osc_message.GetAddress().compare("/wxKnobPush")) 
		this->mackie_sender_receiver->send_data(controller::KNOB_PUSH, osc_message.get_int(0) + 1, 1);
	if(!osc_message.GetAddress().compare("/wxRecEnable"))
		this->mackie_sender_receiver->send_data(controller::REC_ENABLE, osc_message.get_int(0) + 1, 1);
	if(!osc_message.GetAddress().compare("/wxSolo"))
		this->mackie_sender_receiver->send_data(controller::SOLO, osc_message.get_int(0) + 1, 1);
	if(!osc_message.GetAddress().compare("/wxMute"))
		this->mackie_sender_receiver->send_data(controller::MUTE, osc_message.get_int(0) + 1, 1);
	if(!osc_message.GetAddress().compare("/wxSelect"))
		this->mackie_sender_receiver->send_data(controller::SELECT, osc_message.get_int(0) + 1, 1);
	return;
}

void MyFrame::process_midi_data(wxThreadEvent& event)
{
	OscMessage osc_message = event.GetPayload<OscMessage>();	
	MidiMessage_v2 message = this->get_midi_data(osc_message);
	int button_nr = message.midi_data[1] % 8;
	int value = message.midi_data[2];
	
	switch (message.midi_data[0] & 0xf0) {
	case 0x90:
		switch (mackie::button_type(message.midi_data[1] / STRIPS_PER_CONTROLLER)) {
		case mackie::RECORD:
			if(value)
				channel[button_nr]->record->SetBackgroundColour(wxColour(255, 0, 0));
			else
				channel[button_nr]->record->SetBackgroundColour(wxColour(255, 255, 255));
			break;
		case mackie::SOLO:
			if (value)
				channel[button_nr]->solo->SetBackgroundColour(wxColour(255, 255, 0));
			else
				channel[button_nr]->solo->SetBackgroundColour(wxColour(255, 255, 255));
			break;
		case mackie::MUTE:
			if (value)
				channel[button_nr]->mute->SetBackgroundColour(wxColour(255, 255, 0));
			else
				channel[button_nr]->mute->SetBackgroundColour(wxColour(255, 255, 255));
			break;
		case mackie::SELECT:
			if (value)
				channel[button_nr]->select->SetBackgroundColour(wxColour(0, 0, 255));
			else
				channel[button_nr]->select->SetBackgroundColour(wxColour(255, 255, 255));
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
		int value = ((message.midi_data[1] | (message.midi_data[2] << 7)) - MAX_14_BIT) * -1;
		this->channel[message.midi_data[0] & 0x0f]->fader->SetValue(value);
		//pitch bend
	}
	break;
	case 0xf0:
		{
		wxString string[8];
		long long test_data = 0;
		for (int i = 7; i < 119; i++) {
			int line_nr = (i - 7) / 56;
			int string_nr = ((i - 7 - (56 * line_nr))) / 7;
			if(message.midi_data[i] >= 0)
				string[string_nr].append(message.midi_data[i]);
			else 
				string[string_nr].append(' ');
		}
		for (int i = 0; i < 8; i++) {

			if (string[i].size() >= 8) {
				string[i].Remove(6, 1);
				string[i].insert(6, '\n');
			}
		}

		for (int i = 0; i < 8; i++) 
			this->channel[i]->display->SetLabel(string[i]);

		break;
		}
	}
}

//we have to use osc for every communication, and indicate wich osc messages come from our own controller, 
//and wich come from ardour...
//else we have again to invent our own protocol...

void Channel::OnSlider(wxCommandEvent& event)
{
	//we have to generate here a custom osc message, 
	//because from now on the gui thread will communicate over Osc...
	OscMessage osc_message("/wxSlider");
	//thread_message message;

	osc_message.PushInt(this->index);
	osc_message.PushFloat(((float(event.GetInt()) / MAX_14_BIT) - 1) * - 1);

	int size = 0;
	char* bytes;
	bytes = osc_message.GetBytes(size);
	OscMessage osc_message_2(bytes, size);

	wxThreadEvent event_1 = wxThreadEvent(wxEVT_THREAD); // No specific id
	event_1.SetPayload(osc_message_2);
	wxQueueEvent(handler, event_1.Clone());
}

void Channel::OnButton(wxCommandEvent& event)
{
	OscMessage osc_message("/wxButton");
	wxString label(this->function_button->GetLabel());
	if (event.GetId() == this->function_button->GetId()) {
		osc_message = OscMessage("/wxKnobPush");
		osc_message.PushInt(this->index);
	}
	if (event.GetId() == this->record->GetId()) {
		osc_message = OscMessage("/wxRecEnable");
		osc_message.PushInt(this->index);
	}
	if (event.GetId() == this->solo->GetId()) {
		osc_message = OscMessage("/wxSolo");
		osc_message.PushInt(this->index);
	}
	if (event.GetId() == this->mute->GetId()) {
		osc_message = OscMessage("/wxMute");
		osc_message.PushInt(this->index);
	}
	if (event.GetId() == this->select->GetId()) {
		osc_message = OscMessage("/wxSelect");
		osc_message.PushInt(this->index);
	}
	int size = 0;
	char* bytes;
	bytes = osc_message.GetBytes(size);
	OscMessage osc_message_2(bytes, size);

	wxThreadEvent event_1 = wxThreadEvent(wxEVT_THREAD);
	event_1.SetPayload(osc_message_2);
	wxQueueEvent(handler, event_1.Clone());
	return;
}

