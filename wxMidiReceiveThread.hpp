#pragma once
#include <wx/wx.h>
#include <wx/thread.h>
#include <MidiSenderReceiver.hpp>

class wxMidiReceiveThread : public wxThread {
public:
	wxMidiReceiveThread(wxEvtHandler* handler, MidiSenderReceiver* midi_controller_object);
	MidiSenderReceiver* midi_controller;
protected:
	ExitCode Entry() override;
	
private:
	wxEvtHandler* m_handler;
};

