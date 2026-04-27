#include "MackieControl.hpp"

void MackieControl::update_display(const strip_feedback *strips)
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
	send_data(MidiMessage(mackie_display.MIDI_TX_SYSX_Buffer, 120));
}

void MackieControl::update_display(const struct send *sends)
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
	send_data(MidiMessage(mackie_display.MIDI_TX_SYSX_Buffer, 120));
}

void MackieControl::update_display(const plugin_parameter *selected_plugin, plugin_multiplexer_c *plugin_multiplexer, int plugin_bank)
{
	for(int i = 0; i < STRIPS_PER_CONTROLLER; i++){
		int plugin_parameter_id = plugin_multiplexer->get_controller_to_plugin(i + plugin_bank * STRIPS_PER_CONTROLLER + 1);
		this->mackie_display.mackie_display_formated.at(i) = selected_plugin[plugin_parameter_id].name;
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

void MackieControl::update_faders(const plugin_parameter *selected_plugin, plugin_multiplexer_c *plugin_multiplexer, int plugin_bank)
{
	for(int i = 0; i < STRIPS_PER_CONTROLLER; i++){
		int plugin_parameter_number = plugin_multiplexer->get_controller_to_plugin(i + ONE_BASED + STRIPS_PER_CONTROLLER * plugin_bank);
		float value = selected_plugin[plugin_parameter_number].value;
		this->send_data(controller::PLUGIN_PARAMETER_VALUE, i + ONE_BASED, value);
	}
}

void MackieControl::update_faders(const struct send *sends)
{
	for(int i = 0; i < STRIPS_PER_CONTROLLER; i++){
		float value = sends[i + ONE_BASED].volume;
		this->send_data(controller::STRIP_VOLUME, i + ONE_BASED, value);
	}	
}

void MackieControl::update_faders(const struct strip_feedback *strips)
{
	for(int i = 0; i < STRIPS_PER_CONTROLLER; i++){
		float value = strips[i + 1].volume;
		this->send_data(controller::STRIP_VOLUME, i + ONE_BASED, value);
	}
}

void MackieControl::prepare_strip_names(const strip_feedback *strips)
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
}

void MackieControl::prepare_selected_plugin_parameter_names(const selected_strip_struct *selected_strip, plugin_multiplexer_c *plugin_multiplexer)
{
	for(int i = 0; i < STRIPS_PER_CONTROLLER; i++){
		int plugin_parameter_id = plugin_multiplexer->get_controller_to_plugin(i + selected_strip->plugin_bank * STRIPS_PER_CONTROLLER + 1);
		this->mackie_display.mackie_display_formated.at(i) = selected_strip->selected_plugin[plugin_parameter_id].name;
	}
	size_t position = 0;
	for(int i = 0; i < STRIPS_PER_CONTROLLER; i++){
		position = this->mackie_display.mackie_display_formated.at(i).find(' ');
		if(position < 6)
			this->mackie_display.mackie_display_formated.at(i).insert(position, 7 - position - 1, ' ');
	}
}

void MackieControl::prepare_selected_strip_send_names(const struct send *sends)
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
}

