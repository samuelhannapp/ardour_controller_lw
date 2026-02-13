#include "ArdourSenderReceiver.hpp"
#include "PluginMultiplexer.hpp"

//this one need's the plugin multiplexer...
std::vector<std::string> selected_strip_struct::get_selected_plugin_parameter_names(plugin_multiplexer_struct *plugin_multiplexer)
{
	std::vector<std::string> output;
	for (int i = 1; i <= STRIPS_PER_CONTROLLER; i++) {
		int index = plugin_multiplexer->get_controller_to_plugin(i + (STRIPS_PER_CONTROLLER * this->plugin_bank));
		output.push_back(this->selected_plugin[index].name);
	}
	return output;
}

std::vector<std::string> selected_strip_struct::get_selected_strip_send_names()
{
	std::vector<std::string> output;
	for(int i = 1; i <= STRIPS_PER_CONTROLLER; i++)
		output.push_back(sends[i].name);
	return output;
}

void selected_strip_struct::update_selected_strip(enum controller::controller_message type, int nr, float value)
{
	switch(type){
	case controller::SEND_ENABLE:
		this->sends[nr].enable = bool(value);
		break;
	case controller::SEND_FADER:
		this->sends[nr].volume = value;
		break;
	case controller::PLUGIN_PARAMETER_VALUE:
		this->selected_plugin[nr].value = value;
		break;
	case controller::SELECT:
		if(bool(value))
    		this->number = nr;
		break;
	default:
		break;
	}
}

void selected_strip_struct::update_selected_strip(enum controller::controller_message type, int nr, std::string string)
{
	switch(type){
	case controller::SEND_NAME:
		this->sends[nr].name = string;
		break;
	case controller::PLUGIN_PARAMETER_NAME:
		this->selected_plugin[nr].name = string;
	default:
		break;
	}
}

bool selected_strip_struct::controller_channel_nr_is_within_plugin_bank(int fader_id)
{
	if((fader_id / STRIPS_PER_CONTROLLER) == this->plugin_bank)
		return true;

	return false;
}

int selected_strip_struct::get_selected_plugin_index()
{
	std::vector<std::string> plugin_list = this->plugin_list;
	unsigned int index = 0;
	for(std::string name : this->plugin_list){
		if(name.compare(this->selected_plugin_name))
			index++;
		break;
	}
	if(index == this->plugin_list.size())
		return 0;

	return index + 1;
}

void selected_strip_struct::initialize_selected_plugin_descriptor()
{
	for(int i = 0; i < MAX_PLUGIN_PARAMETERS; i++){
		this->selected_plugin[i].name = std::string(" ");
		this->selected_plugin[i].value = 0;
	}
}

void selected_strip_struct::initialize_selected_strip_sends()
{
	for (int i = 0; i < SEND_ARRAY_SIZE; i++)
		this->sends[i].name = std::string(" ");
}

void selected_strip_struct::initialize_selected_strip_plugin_list(){
	for(int i = 0; i < 16; i++)
		this->plugin_list.push_back(" ");
	return;
}

void selected_strip_struct::initialize_selected_strip()
{
	this->plugin_bank = 0;
}
