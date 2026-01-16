//#include "MackieSenderReceiverMidi.hpp"
#include "MackieSenderReceiverUdp.hpp"
#include "ArdourSenderReceiver.hpp"
#include "OscMessage.hpp"
#include "Defines.hpp"

#ifdef __STM32F7xx_HAL_H
extern uint8_t IP_ADDRESS[4];
#endif

void MackieSenderReceiver::initialize_mackie_display_formated()
{
	std::string default_string("defa defa");
	for(int i = 0; i < STRIPS_PER_CONTROLLER; i++)
		mackie_display.mackie_display_formated.push_back(default_string);

	return;
}

void MackieSenderReceiver::send_data(enum controller::controller_message type, int strip_nr, float value)
{
	switch(type){
	case controller::STRIP_VOLUME:
	case controller::PLUGIN_PARAMETER_VALUE:
		{
		uint16_t value_14_bit = uint16_t(value * float(MAX_14_BIT));
    	uint8_t value_low = value_14_bit & 0x7f;
    	uint8_t value_high = ((value_14_bit & 0x3f80) >> 7);
        unsigned char midi_message[3]  = {(unsigned char)(0xe0 | (strip_nr - 1)), value_low, value_high};
	    send_data(MidiMessage(midi_message, 3));
		}
		break;
	case controller::REC_ENABLE:
		{
		unsigned char button_nr = mackie::RECORD * STRIPS_PER_CONTROLLER + strip_nr -1;
		unsigned char button_value = int(value) * 0x7f;
		unsigned char midi_message[3]  = {0x90, button_nr, button_value};
	    send_data(MidiMessage(midi_message, 3));
		}
		break;
	case controller::SOLO:
		{
		unsigned char button_nr = mackie::SOLO * STRIPS_PER_CONTROLLER + strip_nr -1;
    	unsigned char button_value = int(value) * 0x7f;
		unsigned char midi_message[3]  = {0x90, button_nr, button_value};
	    send_data(MidiMessage(midi_message, 3));
		}
		break;
	case controller::MUTE:
		{
		unsigned char button_nr = mackie::MUTE * STRIPS_PER_CONTROLLER + strip_nr -1;
    	unsigned char button_value = int(value) * 0x7f;
		unsigned char midi_message[3]  = {0x90, button_nr, button_value};
	    send_data(MidiMessage(midi_message, 3));
		}
		break;
	case controller::SELECT:
		{
    	unsigned char button_nr = mackie::SELECT * STRIPS_PER_CONTROLLER + strip_nr -1;
    	unsigned char button_value = int(value);
    	button_value = button_value * 0x7f;
		unsigned char midi_message[3]  = {0x90, button_nr, button_value};
	    send_data(MidiMessage(midi_message, 3));
		}
		break;
	case controller::STEREO_POSITION:
		{
		unsigned char knob_nr = mackie::LED_STRIP * STRIPS_PER_CONTROLLER + strip_nr -1;
    	float knob_value = value;
    	unsigned char led_strip_value = int(knob_value * float(MAX_14_BIT) / 1489.0);
    	//revert
    	led_strip_value = (led_strip_value - 11) * -1;
    	//in min = 1
    	if(led_strip_value == 0){
    		led_strip_value = 1;
    	}
		unsigned char midi_message[3]  = {0xb0, knob_nr, led_strip_value};
	    send_data(MidiMessage(midi_message, 3));
		}
		break;
	case controller::SEND_FADER:
		{
		unsigned char knob_nr = mackie::LED_STRIP * STRIPS_PER_CONTROLLER + strip_nr -1;
    	float knob_value = value;
    	unsigned char led_strip_value = int(knob_value * float(MAX_14_BIT) / 1489.0);
    	//revert
    	led_strip_value = (led_strip_value - 11) * -1;
    	//in min = 1
    	if(led_strip_value == 0){
    		led_strip_value = 1;
    	}
		unsigned char midi_message[3]  = {0xb0, knob_nr, led_strip_value};
	    send_data(MidiMessage(midi_message, 3));
		}
		break;
	case controller::METER:
		{
		unsigned char led_strip_value = int(value * float(MAX_14_BIT) / 1489.0);
		if(led_strip_value > 12)
			led_strip_value = 12;
		if(led_strip_value < 0)
			led_strip_value = 0;
		led_strip_value |= ((strip_nr - 1) << 4);
		unsigned char midi_message[3]  = {0xd0, led_strip_value, 0};
	    send_data(MidiMessage(midi_message, 3));
		}
		break;
	case controller::KNOB_PUSH:
		{
		unsigned char button_nr = mackie::KNOB_PUSH * STRIPS_PER_CONTROLLER + strip_nr - 1;
		unsigned char button_value = int(value) * 0x7f;
		unsigned char midi_message[3] = { 0x90, button_nr, button_value };
		send_data(MidiMessage(midi_message, 3));
		break;
		}
	default:
		break;
	}
}

void MackieSenderReceiver::send_data(std::string strings[9])
{
	for(int i = 0; i < STRIPS_PER_CONTROLLER; i++){
		this->mackie_display.mackie_display_formated.at(i) = strings[i + 1];
	}
	size_t position = 0;
	for(int i = 0; i < STRIPS_PER_CONTROLLER; i++){
		position = this->mackie_display.mackie_display_formated.at(i).find(' ');
		if(position < 6)
			this->mackie_display.mackie_display_formated.at(i).insert(position, 7 - position - 1, ' ');
	}

	mackie_display.fill_sysx_buffer();
	send_data(MidiMessage(mackie_display.MIDI_TX_SYSX_Buffer, 120));
}

void mackie_display_struct::fill_sysx_buffer()
{
	int buffer_counter = 0;
	int line_nr = 0;
	int string_nr = 0;
	unsigned int string_at = 0;
	for(int i = 0; i < 120; i++){
		switch(i){
			case 0:
				this->MIDI_TX_SYSX_Buffer[i] = 0xf0;
					break;
			case 1:
				this->MIDI_TX_SYSX_Buffer[i] = 0x00;
					break;
			case 2:
				this->MIDI_TX_SYSX_Buffer[i] = 0x00;
					break;
			case 3:
				this->MIDI_TX_SYSX_Buffer[i] = 0x66;
					break;
			case 4:
				this->MIDI_TX_SYSX_Buffer[i] = 0x14;
					break;
			case 5:
				this->MIDI_TX_SYSX_Buffer[i] = 18;
					break;
			case 6:
				this->MIDI_TX_SYSX_Buffer[i] = 0;
				break;
			case 119:
				this->MIDI_TX_SYSX_Buffer[i] = 0xf7;
					break;
			default:
					//here we can fill in the strip names,
					string_nr = buffer_counter / 7 - 1;
					line_nr = string_nr / STRIPS_PER_CONTROLLER;
					string_nr %= STRIPS_PER_CONTROLLER;
					string_at = buffer_counter % 7 + (line_nr * 7);
					if(this->mackie_display_formated.at(string_nr).size() > string_at)
						this->MIDI_TX_SYSX_Buffer[i] = this->mackie_display_formated.at(string_nr).at(string_at);
					else
						this->MIDI_TX_SYSX_Buffer[i] = ' ';
		}
		buffer_counter++;
	}
}


//This is only for the Udp version of the MackieSenderReceiver
;void MackieSenderReceiver::send_data(struct MidiMessage message)
{
	OscMessage osc_message("/Midi");
	if (message.length == 3) {
		int data = message.data[0] | (message.data[1] << 8) | (message.data[2] << 16);
		osc_message.PushInt(data);
		int size = 0;
		char* data_ptr = osc_message.GetBytes(size);
		UdpSenderReceiver::send_data(data_ptr, size);
	}
	long long temp[15] = { 0 };
	if (message.length == 120) {
		for (int i = 0; i < 120; i++)
			temp[i / 8] |= ((long long)message.data[i]) << ((i % 8) * 8);

		for (int i = 0; i < 15; i++) {
			osc_message.PushLongLong(temp[i]);
		}
	

	int size = 0;
	char* data_ptr = osc_message.GetBytes(size);
	UdpSenderReceiver::send_data(data_ptr, size);
	}
	return;
}

void MackieSenderReceiver::receive_data(MidiMessage &midi_message)
{
	char udp_data[1024];
	int length = UdpSenderReceiver::receive_data(udp_data);
	OscMessage osc_message(udp_data, length);
	int midi_data = osc_message.get_int(0);
	midi_message.data[0] = midi_data & 0xff;
	midi_message.data[1] = (midi_data & 0xff00) >> 8;
	midi_message.data[2] = (midi_data & 0xff0000) >> 16;
	midi_message.data[3] = 0;
}
