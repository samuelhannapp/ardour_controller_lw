#include "wxMidiReceiveThread.hpp"
#include "OscMessage.hpp"

wxMidiReceiveThread::wxMidiReceiveThread(wxEvtHandler* handler, MidiSenderReceiver* midi_controller_object)
	: wxThread(wxTHREAD_DETACHED), m_handler(handler)
{
	midi_controller = midi_controller_object;
}

//Receiving class setup
//thread = new wxOscReceiveThread(this);
//thread->Run();
//Bind(wxEVT_THREAD, &ReceivingClass...::OnThreadUpdate, this);

wxThread::ExitCode wxMidiReceiveThread::Entry()
{
	while (1) {
		OscMessage osc_message("/quadrature_rotary_encoder");
		MidiMessage midi_message;
		midi_controller->receive_data(midi_message);
		int nr = midi_message.data[0] &= 0x0f;
		int int_value = midi_message.data[1] | (midi_message.data[2] << 7);
		osc_message.PushInt(nr);
		osc_message.PushInt(int_value);
		wxThreadEvent event = wxThreadEvent();
		event.SetPayload(osc_message);
		wxQueueEvent(m_handler, event.Clone());
	}
	return nullptr;
}
