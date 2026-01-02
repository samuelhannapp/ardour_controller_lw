#include "MackieSenderReceiver.hpp"
#include "ArdourSenderReceiver.hpp"

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
	    MidiSenderReceiver::send_data(midi_message, 3);
		}
		break;
	case controller::REC_ENABLE:
		{
		unsigned char button_nr = mackie::RECORD * STRIPS_PER_CONTROLLER + strip_nr -1;
		unsigned char button_value = int(value) * 0x7f;
		unsigned char midi_message[3]  = {0x90, button_nr, button_value};
	    MidiSenderReceiver::send_data(midi_message, 3);
		}
		break;
	case controller::SOLO:
		{
		unsigned char button_nr = mackie::SOLO * STRIPS_PER_CONTROLLER + strip_nr -1;
    	unsigned char button_value = int(value) * 0x7f;
		unsigned char midi_message[3]  = {0x90, button_nr, button_value};
	    MidiSenderReceiver::send_data(midi_message, 3);
		}
		break;
	case controller::MUTE:
		{
		unsigned char button_nr = mackie::MUTE * STRIPS_PER_CONTROLLER + strip_nr -1;
    	unsigned char button_value = int(value) * 0x7f;
		unsigned char midi_message[3]  = {0x90, button_nr, button_value};
	    MidiSenderReceiver::send_data(midi_message, 3);
		}
		break;
	case controller::SELECT:
		{
    	unsigned char button_nr = mackie::SELECT * STRIPS_PER_CONTROLLER + strip_nr -1;
    	unsigned char button_value = int(value);
    	button_value = button_value * 0x7f;
		unsigned char midi_message[3]  = {0x90, button_nr, button_value};
	    MidiSenderReceiver::send_data(midi_message, 3);
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
	    MidiSenderReceiver::send_data(midi_message, 3);
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
	    MidiSenderReceiver::send_data(midi_message, 3);
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
	    MidiSenderReceiver::send_data(midi_message, 3);
		}
		break;
	default:
		break;
	}
}

void MackieSenderReceiver::update_display(const strip_feedback *strips)
{
	for(int i = 0; i < STRIPS_PER_CONTROLLER; i++){
		this->mackie_display.mackie_display_formated.at(i) = strips[i + 1].name;
	}
	size_t position = 0;
	for(int i = 0; i < STRIPS_PER_CONTROLLER; i++){
		position = this->mackie_display.mackie_display_formated.at(i).find(' ');
		if(position < 6)
			this->mackie_display.mackie_display_formated.at(i).insert(position, 7 - position - 1, ' ');
	}

	mackie_display.fill_sysx_buffer();
	MidiSenderReceiver::send_data(mackie_display.MIDI_TX_SYSX_Buffer, 120);
}

void MackieSenderReceiver::update_display(const struct send *sends)
{
	for(int i = 0; i < STRIPS_PER_CONTROLLER; i++){
		this->mackie_display.mackie_display_formated.at(i) = sends[i + 1].name;
	}
	size_t position = 0;
	for(int i = 0; i < STRIPS_PER_CONTROLLER; i++){
		position = this->mackie_display.mackie_display_formated.at(i).find(' ');
		if(position < 6)
			this->mackie_display.mackie_display_formated.at(i).insert(position, 7 - position - 1, ' ');
	}

	mackie_display.fill_sysx_buffer();
	MidiSenderReceiver::send_data(mackie_display.MIDI_TX_SYSX_Buffer, 120);
}

void MackieSenderReceiver::update_display(const plugin_parameter *selected_plugin, const plugin_multiplexer_struct *plugin_multiplexer, int plugin_bank)
{
	for(int i = 0; i < STRIPS_PER_CONTROLLER; i++){
		int plugin_parameter_id = plugin_multiplexer->plugin_multiplexer_from_controller[i + plugin_bank * STRIPS_PER_CONTROLLER + 1];
		this->mackie_display.mackie_display_formated.at(i) = selected_plugin[plugin_parameter_id].name;
	}
	size_t position = 0;
	for(int i = 0; i < STRIPS_PER_CONTROLLER; i++){
		position = this->mackie_display.mackie_display_formated.at(i).find(' ');
		if(position < 6)
			this->mackie_display.mackie_display_formated.at(i).insert(position, 7 - position - 1, ' ');
	}
	mackie_display.fill_sysx_buffer();
	MidiSenderReceiver::send_data(mackie_display.MIDI_TX_SYSX_Buffer, 120);
}

void MackieSenderReceiver::update_faders(const plugin_parameter *selected_plugin, const plugin_multiplexer_struct *plugin_multiplexer, int plugin_bank)
{
	for(int i = 0; i < STRIPS_PER_CONTROLLER; i++){
		int plugin_parameter_number = plugin_multiplexer->plugin_multiplexer_from_controller[i + ONE_BASED + STRIPS_PER_CONTROLLER * plugin_bank];
		float value = selected_plugin[plugin_parameter_number].value;
		this->send_data(controller::PLUGIN_PARAMETER_VALUE, i + ONE_BASED, value);
	}
}

void MackieSenderReceiver::update_faders(const struct send *sends)
{
	for(int i = 0; i < STRIPS_PER_CONTROLLER; i++){
		float value = sends[i + ONE_BASED].volume;
		this->send_data(controller::STRIP_VOLUME, i + ONE_BASED, value);
	}	
}

void MackieSenderReceiver::update_faders(const struct strip_feedback *strips)
{
	for(int i = 0; i < STRIPS_PER_CONTROLLER; i++){
		float value = strips[i + 1].volume;
		this->send_data(controller::STRIP_VOLUME, i + ONE_BASED, value);
	}
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

void mackie_display_struct::prepare_strip_names(const strip_feedback *strips)
{
	for(int i = 0; i < STRIPS_PER_CONTROLLER; i++){
		this->mackie_display_formated.at(i) = strips[i + 1].name;
	}
	size_t position = 0;
	for(int i = 0; i < STRIPS_PER_CONTROLLER; i++){
		position = this->mackie_display_formated.at(i).find(' ');
		if(position < 6)
			this->mackie_display_formated.at(i).insert(position, 7 - position - 1, ' ');
	}	
}

void mackie_display_struct::prepare_selected_plugin_parameter_names(const selected_strip_struct *selected_strip, const plugin_multiplexer_struct *plugin_multiplexer)
{
	for(int i = 0; i < STRIPS_PER_CONTROLLER; i++){
		int plugin_parameter_id = plugin_multiplexer->plugin_multiplexer_from_controller[i + selected_strip->plugin_bank * STRIPS_PER_CONTROLLER + 1];
		this->mackie_display_formated.at(i) = selected_strip->selected_plugin[plugin_parameter_id].name;
	}
	size_t position = 0;
	for(int i = 0; i < STRIPS_PER_CONTROLLER; i++){
		position = this->mackie_display_formated.at(i).find(' ');
		if(position < 6)
			this->mackie_display_formated.at(i).insert(position, 7 - position - 1, ' ');
	}
}

void mackie_display_struct::prepare_selected_strip_send_names(const struct send *sends)
{
	for(int i = 0; i < STRIPS_PER_CONTROLLER; i++){
		this->mackie_display_formated.at(i) = sends[i + 1].name;
	}
	size_t position = 0;
	for(int i = 0; i < STRIPS_PER_CONTROLLER; i++){
		position = this->mackie_display_formated.at(i).find(' ');
		if(position < 6)
			this->mackie_display_formated.at(i).insert(position, 7 - position - 1, ' ');
	}
}



