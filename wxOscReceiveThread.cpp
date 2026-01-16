#include "wxOscReceiveThread.h"
#include "oscmessage.hpp"

wxOscReceiveThread::wxOscReceiveThread(wxEvtHandler* handler, UdpSenderReceiver* osc_controller_object)
	: wxThread(wxTHREAD_DETACHED), m_handler(handler)
{
	osc_controller = osc_controller_object;
}

//Receiving class setup
//thread = new wxOscReceiveThread(this);
//thread->Run();
//Bind(wxEVT_THREAD, &ReceivingClass...::OnThreadUpdate, this);



wxThread::ExitCode wxOscReceiveThread::Entry()
{
	thread_message message;
	message.midi_message = true;
	while (1) {
		char buffer[1024];
		int size = osc_controller->receive_data(buffer);
		
		OscMessage osc_message(buffer, size);
		if (osc_message.GetTypeList().at(0) == 'i') {
			message.midi_data_size = 3;
			int midi_data = osc_message.get_int(0);
			message.midi_data[0] = unsigned char(midi_data & 0xff);
			message.midi_data[1] = unsigned char((midi_data & 0xff00) >> 8);
			message.midi_data[2] = unsigned char((midi_data & 0xff0000) >> 16);
		}
		if (osc_message.GetTypeList().at(0) == 'h') {
			message.midi_data_size = 120;
			if (osc_message.GetTypeList().size() == 15)
				for (int i = 0; i < 120; i++) {
					int index = i / 8;
					long long temp = (long long)(osc_message.get_long_long(index));
					temp = temp & (0xff << ((i % 8) * 8));
					temp = temp >> ((i % 8) * 8);
					message.midi_data[i] = temp;
				}
		}
		wxThreadEvent event = wxThreadEvent();
		event.SetPayload(message);
		wxQueueEvent(m_handler, event.Clone());
	}
	return nullptr;
}
/*
void ReceivingClass...::OnThreadUpdate(wxThreadEvent& event)
{
	// SAFE: runs on GUI thread
	std::vector<char> data;
	data = event.GetPayload<std::vector<char>>();
	char array[1024];
	for (int i = 0; i < data.size(); i++)
		array[i] = data.at(i);
	OscMessage message(array, data.size());
	int index = message.get_int(0) - 1;
	this->channel_name[index]->SetLabel(message.get_string(1));
	this->channel_name[index]->SetSize(channel_name_panel[index]->GetSize());
}
*/
