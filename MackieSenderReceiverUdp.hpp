#pragma once
#include <stdint.h>
#include <vector>
#include <string>

#include "UdpSenderReceiver.hpp"
#include "Defines.hpp"
#include "MackieDisplay.hpp"

class MackieSenderReceiver : public UdpSenderReceiver{
public:
	using UdpSenderReceiver::UdpSenderReceiver;
	using UdpSenderReceiver::receive_data;
	struct mackie_display_struct mackie_display;
	void initialize_mackie_display_formated();
	void send_data(enum controller::controller_message type, int strip_nr, float value); //this is for controls
	void send_data(std::string strings[9]); //this is for display
	void send_data(MidiMessage message);
	void receive_data(MidiMessage &midi_message);
};
