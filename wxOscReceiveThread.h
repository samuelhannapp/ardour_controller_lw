#pragma once
#include <wx/wx.h>
#include <wx/thread.h>
#include <UdpSenderReceiver.hpp>

struct thread_message {
	bool midi_message;
	enum controller::controller_message type;
	float value;
	int index;
	char midi_data[120];
	int midi_data_size;
};

class wxOscReceiveThread : public wxThread {
public:
	wxOscReceiveThread(wxEvtHandler* handler, UdpSenderReceiver* osc_controller_object);
	UdpSenderReceiver* osc_controller;
protected:
	ExitCode Entry() override;
	
private:
	wxEvtHandler* m_handler;
};



