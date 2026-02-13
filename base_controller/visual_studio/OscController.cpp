#include "OscController.hpp"
#include "UdpSenderReceiver.hpp"
#ifdef MACKIE_CONTROL_MIDI_VERSION
#include "MackieSenderReceiverMidi.hpp"
#endif
#ifdef MACKIE_CONTROL_UDP_VERSION
#include "MackieSenderReceiverUdp.hpp"
#endif
#include <array>
#include <functional>
#include "Defines.hpp"

OscController::OscController(std::string destination_ip_address, unsigned int udp_port_in, unsigned int udp_port_out, unsigned int midi_port_in, unsigned int midi_port_out, unsigned int display_port_in, unsigned int display_port_out)
{
	this->plugin_multiplexer.initialize_plugin_multiplexer();
	this->local_strip_data.selected_strip.initialize_selected_plugin_descriptor();
	this->local_strip_data.selected_strip.initialize_selected_strip_sends();
	this->local_strip_data.selected_strip.initialize_selected_strip_plugin_list();
	this->local_strip_data.selected_strip.initialize_selected_strip();
	this->plugin_multiplexer.initialize_plugin_multiplexer_from_controller_and_from_plugin();
	
    this->ardour_sender_receiver = ArdourSenderReceiver(destination_ip_address, udp_port_in, udp_port_out);
#ifdef MACKIE_CONTROL_MIDI_VERSION
	this->mackie_sender_receiver = new MackieControl(midi_port_in, midi_port_out);
#endif
#ifdef MACKIE_CONTROL_UDP_VERSION
	this->mackie_sender_receiver = new MackieControl("127.0.0.1", 13, 14);
#endif
	this->mackie_sender_receiver->initialize_mackie_display_formated();//this should be inside the constructor...

    std::thread mackie_control_thread(&OscController::mackie_receive_thread, this);
	mackie_control_thread.detach();
	
	std::thread ardour_thread(&OscController::ardour_receive_thread, this);
	ardour_thread.detach();

	display_object = display("127.0.0.1", display_port_in, display_port_out);

	this->ardour_sender_receiver.init_osc_controller();
}

void OscController::process_midi(MidiMessage message)
{
	OscMessage msg("/nothing");
	bool button_pressed = false;
	int channel_nr = 0;
	int fader_nr = 0;
	int value_14_bit = 0;
	float value_float = 0;
	float increment = 0;

	enum mackie::button_type type = (enum mackie::button_type)(message.data[1] / STRIPS_PER_CONTROLLER);

	int command = message.data[0] & 0xf0;
	switch(command) {
	//case 0x80: // Note off
	case 0x90: // Note on
  		button_pressed = message.data[2];
		channel_nr = message.data[1] % STRIPS_PER_CONTROLLER + ONE_BASED;

		if(!button_pressed)
			if(!(type == mackie::FADER_TOUCH))
				break;

		switch(type){
			case mackie::RECORD:
				msg = OscMessage("/strip/recenable");
				msg.PushInt(channel_nr);
				msg.PushInt(!local_strip_data.strips[channel_nr].rec);
				break;
			case mackie::SOLO:
				msg = OscMessage("/strip/solo");
				msg.PushInt(channel_nr);
				msg.PushInt(!local_strip_data.strips[channel_nr].solo);
				break;
			case mackie::MUTE:
				msg = OscMessage("/strip/mute");
				msg.PushInt(channel_nr);
				msg.PushInt(!local_strip_data.strips[channel_nr].mute);
				break;
			case mackie::SELECT:
				local_strip_data.selected_strip.update_selected_strip(controller::SELECT, channel_nr, 1);
				msg = OscMessage("/strip/select");
				msg.PushInt(channel_nr);
				msg.PushInt(0);
				ardour_sender_receiver.send_data(msg);
                break;
			case mackie::KNOB_PUSH:
				switch(channel_nr){
					case 1:
						this->switch_channel_mode_without_updating_mackie(PanMode);
						msg = OscMessage("/set_surface/strip_types");
						msg.PushInt(1 << AudioBusses);
						break;
					case 2:
						this->switch_channel_mode_without_updating_mackie(PanMode);
						msg = OscMessage("/set_surface/strip_types");
						msg.PushInt(1 << VCAs);
						break;
					case 3: //bank down
						if(this->mode == PanMode)
							msg = OscMessage("/bank_down");
						if(this->mode == PluginMode){
							if(local_strip_data.selected_strip.plugin_bank > 0)
								local_strip_data.selected_strip.plugin_bank--;
							this->mackie_sender_receiver->update_display(this->local_strip_data.selected_strip.selected_plugin, &this->plugin_multiplexer, this->local_strip_data.selected_strip.plugin_bank);
                            this->mackie_sender_receiver->update_faders(this->local_strip_data.selected_strip.selected_plugin, &this->plugin_multiplexer, this->local_strip_data.selected_strip.plugin_bank);
							this->display_object.update_display(OscMessage("/select/plugin/parameter/name"), this->local_strip_data.selected_strip.get_selected_plugin_parameter_names(&this->plugin_multiplexer));
						}
						if(this->mode == SendMode){
							msg = OscMessage("/select/send_page");
							msg.PushFloat(-1);
						}
						break;
					case 4: //bank up
						if(this->mode == PanMode)
							msg = OscMessage("/bank_up");
						if(this->mode == PluginMode){
							if(local_strip_data.selected_strip.plugin_bank < (PLUGIN_PAGES_SIZE - 1))
								local_strip_data.selected_strip.plugin_bank++;

							this->mackie_sender_receiver->update_display(this->local_strip_data.selected_strip.selected_plugin, &this->plugin_multiplexer, this->local_strip_data.selected_strip.plugin_bank);
                            this->mackie_sender_receiver->update_faders(this->local_strip_data.selected_strip.selected_plugin, &this->plugin_multiplexer, this->local_strip_data.selected_strip.plugin_bank);
							this->display_object.update_display(OscMessage("/select/plugin/parameter/name"), this->local_strip_data.selected_strip.get_selected_plugin_parameter_names(&this->plugin_multiplexer));
						}
						if(this->mode == SendMode){
							msg = OscMessage("/select/send_page");
							msg.PushFloat(1);
						}
						break;
					case 5:
						this->switch_channel_mode();
                        break;
					case 6:
						msg = OscMessage("/select/plugin");
						msg.PushFloat(-1);
						break;
					case 7:
						msg = OscMessage("/select/plugin");
						msg.PushFloat(1);
						break;
					case 8:
						//msg = OscMessage("/select/spill");
						msg = OscMessage("/strip/spill");
						msg.PushInt(this->local_strip_data.selected_strip.number);
						break;
					default:
						break;
				}
				break;
			case mackie::FADER_TOUCH:
				this->local_strip_data.strips[channel_nr].fader_touch = button_pressed;
				if(this->mode != PanMode)
					break;
				msg = OscMessage("/strip/fader/touch");
				msg.PushInt(channel_nr);
				msg.PushInt(button_pressed);
				break;
			default:
				break;
	  	}
		ardour_sender_receiver.send_data(msg);
		break;
	case 0xa0: // Aftertouch
	case 0xB0: // Continuous controller
		channel_nr = message.data[1] % STRIPS_PER_CONTROLLER + 1;
		if(this->mode == PanMode){
			msg = OscMessage("/strip/pan_stereo_position");
			msg.PushInt(channel_nr);
			increment = message.data[2] == 0x1 ? -1 : 1;
			value_float = this->local_strip_data.strips[channel_nr].stereo_position + increment * 0.03;
			if(value_float < 0) value_float = 0;
			if(value_float > 1) value_float = 1;
			msg.PushFloat(value_float);
		}
		if(this->mode == PluginMode){
			if(!plugin_multiplexer.plugin_multiplexer_from_controller.size())
                break;
			msg = OscMessage("/select/plugin/parameter");
			int plugin_parameter_number = plugin_multiplexer.plugin_multiplexer_from_controller[channel_nr + STRIPS_PER_CONTROLLER * this->local_strip_data.selected_strip.plugin_bank];
			if(!plugin_parameter_number)
                break;
			msg.PushInt(plugin_parameter_number);

			increment = message.data[2] == 0x1 ? 1 : -1;

			float value_float = this->local_strip_data.selected_strip.selected_plugin[plugin_parameter_number].value + increment * 0.03;
			if(value_float < 0) value_float = 0;
			if(value_float > 1) value_float = 1;

			if(this->local_strip_data.selected_strip.selected_plugin[plugin_parameter_number].flags & (1 << TOGGLED))
				value_float = increment == 1 ? 1 : 0;

			msg.PushFloat(value_float);
		}
		if(this->mode == SendMode){
			msg = OscMessage("/select/send_fader");
			msg.PushInt(channel_nr);
			increment = message.data[2] == 0x1 ? 1 : -1;
			value_float = this->local_strip_data.selected_strip.sends[channel_nr].volume + increment * 0.03;
			if(value_float < 0) value_float = 0;
			if(value_float > 1) value_float = 1;
			msg.PushFloat(value_float);
		}
		ardour_sender_receiver.send_data(msg);
		break;
	case 0xC0: // Patch change
	case 0xD0: // Channel Pressure
	case 0xE0: // Pitch bend
		fader_nr = message.data[0] & 0xf;
		value_14_bit = message.data[1] | (message.data[2] << 7);
		value_float = float(value_14_bit / float(MAX_14_BIT));
		if(this->mode == PanMode){
			msg = OscMessage("/strip/fader");
			msg.PushInt(fader_nr + ONE_BASED);
			msg.PushFloat(value_float);
		}
		if(this->mode == PluginMode){
			msg = OscMessage("/select/plugin/parameter");
			int plugin_parameter_number = plugin_multiplexer.plugin_multiplexer_from_controller[fader_nr + 1 + STRIPS_PER_CONTROLLER * this->local_strip_data.selected_strip.plugin_bank];
			if(!plugin_parameter_number)
                break;
			msg.PushInt(plugin_parameter_number);
			msg.PushFloat(value_float);  
		}
		if(this->mode == SendMode){
			msg = OscMessage("/select/send_fader");
			msg.PushInt(fader_nr + 1);
			msg.PushFloat(value_float);
		}
		ardour_sender_receiver.send_data(msg);
		break;
	case 0xF0: // (non-musical commands)
    	break;
	default: 
		break;
    }
}

#ifdef __STM32F7xx_HAL_H
static void write_to_itm(std::string string)
{
	for(char c : string)
		ITM_SendChar(c);
	ITM_SendChar('\n');
	return;
}
#endif

void OscController::mackie_receive_thread()
{
	while (1) {
		unsigned char* ptr = 0;
		MidiMessage message(ptr, 0);
		this->mackie_sender_receiver->receive_data(message);
		this->process_midi(message);
	}
}

bool OscController::is_index_within_bank(int index, int bank_nr, int bank_size)
{
	return ((index / bank_size) == bank_nr) ? true : false;
}

void OscController::ardour_receive_thread()
{
	do{
		OscMessage message = ardour_sender_receiver.receive_data();

		//std::cout << message.GetAddress() << "\n";

		//write_to_itm(message.GetAddress());

		if(!message.GetAddress().compare(0, 13, "/strip/fader\0")){
			int strip_nr = message.get_int(0);
			float value = message.get_float(1);
            local_strip_data.strips[strip_nr].send_data(controller::STRIP_VOLUME, value);
			if(this->mode == PanMode)
				mackie_sender_receiver->send_data(controller::STRIP_VOLUME, strip_nr, value);
			//update_fader(strip_nr, value, local_strip_data.mode);
			continue;
		}
		else if( !message.GetAddress().compare("/strip/recenable")){
			int strip_nr = message.get_int(0);
			float value = message.get_float(1);
			local_strip_data.strips[strip_nr].send_data(controller::REC_ENABLE, value);
			mackie_sender_receiver->send_data(controller::REC_ENABLE, strip_nr, value);
			continue;
		}
		else if(!message.GetAddress().compare("/strip/solo")){
			int strip_nr = message.get_int(0);
			float value = message.get_float(1);
			local_strip_data.strips[strip_nr].send_data(controller::SOLO, value);
			mackie_sender_receiver->send_data(controller::SOLO, strip_nr, value);
			continue;
		}
		else if(!message.GetAddress().compare("/strip/mute")){
			int strip_nr = message.get_int(0);
			float value = message.get_float(1);
			local_strip_data.strips[strip_nr].send_data(controller::MUTE, value);
			mackie_sender_receiver->send_data(controller::MUTE, strip_nr, value);
			continue;
		}
		else if(!message.GetAddress().compare("/strip/select")){
			int strip_nr = message.get_int(0);
			float value = message.get_float(1);
			local_strip_data.selected_strip.update_selected_strip(controller::SELECT, strip_nr, value);
			mackie_sender_receiver->send_data(controller::SELECT, strip_nr, value);
			display_object.send_data(message);
			continue;
		}
		else if(!message.GetAddress().compare("/strip/pan_stereo_position")){
			if (message.GetTypeList().size() == 1)
				break;
			int strip_nr = message.get_int(0);
			float value = message.get_float(1);
			local_strip_data.strips[strip_nr].send_data(controller::STEREO_POSITION, value);
			mackie_sender_receiver->send_data(controller::STEREO_POSITION, strip_nr, value);
			continue;
		}
		else if(!message.GetAddress().compare("/strip/meter")){
			int strip_nr = message.get_int(0);
			float value = message.get_float(1);
			mackie_sender_receiver->send_data(controller::METER, strip_nr, value);
			continue;
		}
		else if(!message.GetAddress().compare("/select/send_enable")){
			int send_nr = message.get_int(0);
			float value = message.get_float(1);

			local_strip_data.selected_strip.update_selected_strip(controller::SEND_ENABLE, send_nr, value);
			continue;
		}
		else if(!message.GetAddress().compare("/select/send_name")){
			int send_nr = message.get_int(0);
			std::string string = message.get_string(1);
			local_strip_data.selected_strip.update_selected_strip(controller::SEND_NAME, send_nr, string);
			if(this->mode == SendMode){
				mackie_sender_receiver->update_display(this->local_strip_data.selected_strip.sends);
				display_object.send_data(message);
			}
			continue;
		}
		else if(!message.GetAddress().compare("/select/send_fader")){
			int send_id = message.get_int(0);
			float value = message.get_float(1);
			local_strip_data.selected_strip.update_selected_strip(controller::SEND_FADER, send_id, value);
			if(this->mode == SendMode)
				mackie_sender_receiver->send_data(controller::SEND_FADER, send_id, value);
			continue;
		}
		else if(!message.GetAddress().compare("/select/plugin/name")){
			std::string plugin_name = message.get_string(0);
			if(plugin_name.size() == 1)
				continue;
			plugin_multiplexer.setup(plugin_name);
			if(local_strip_data.selected_strip.selected_plugin_name == plugin_name) //everything is already setup...
				continue;
			local_strip_data.selected_strip.selected_plugin_name = plugin_name;
			ardour_sender_receiver.get_plugin_list(local_strip_data.selected_strip.number);
			continue;
		}
		else if(!message.GetAddress().compare("/select/plugin/parameter/name")){
			if(!plugin_multiplexer.plugin_multiplexer_from_plugin.size()){
				continue;
			}
			int plugin_parameter_id = message.get_int(0);
			std::string plugin_parameter_name = message.get_string(1);
			if (plugin_parameter_id < 0)
				continue;
			this->local_strip_data.selected_strip.update_selected_strip(controller::PLUGIN_PARAMETER_NAME, plugin_parameter_id, plugin_parameter_name);
			if(this->mode == PluginMode){
				this->mackie_sender_receiver->update_display(this->local_strip_data.selected_strip.selected_plugin, &this->plugin_multiplexer, this->local_strip_data.selected_strip.plugin_bank);
				int plugin_parameter_id_in_controller = this->plugin_multiplexer.get_plugin_to_controller(plugin_parameter_id);
				if(is_index_within_bank(plugin_parameter_id_in_controller, this->local_strip_data.selected_strip.plugin_bank, STRIPS_PER_CONTROLLER) ){
					//we should here only send a single one, not the whole list...
					display_object.update_display(OscMessage("/select/plugin/parameter/name"), this->local_strip_data.selected_strip.get_selected_plugin_parameter_names(&this->plugin_multiplexer));
				}
			}
			continue;
		}
		else if(!message.GetAddress().compare("/select/plugin/parameter")){
			int plugin_parameter_id = message.get_int(0);
			float plugin_parameter_value = message.initialize_type_list().at(1) == 'f' ? message.get_float(1) : message.get_double(1);
			local_strip_data.selected_strip.update_selected_strip(controller::PLUGIN_PARAMETER_VALUE, plugin_parameter_id, plugin_parameter_value);
			if (plugin_parameter_id < plugin_multiplexer.plugin_multiplexer_from_plugin.size()) {
				int fader_id = plugin_multiplexer.plugin_multiplexer_from_plugin[plugin_parameter_id];
				if (local_strip_data.selected_strip.controller_channel_nr_is_within_plugin_bank(fader_id) && (this->mode == PluginMode)) {
					mackie_sender_receiver->send_data(controller::PLUGIN_PARAMETER_VALUE, fader_id, plugin_parameter_value);
				}
			}
			continue;
		}
		else if(!message.GetAddress().compare("/strip/name")){
			int strip_nr = message.get_int(0);
			std::string strip_name = message.get_string(1);
			local_strip_data.strips[strip_nr].send_data(controller::STRIP_NAME, strip_name);
			if (this->mode == PanMode) {
				mackie_sender_receiver->update_display(this->local_strip_data.strips);
				display_object.send_data(message);
			}
			continue;
		}
		else if(!message.GetAddress().compare("/strip/plugin/list")){
			local_strip_data.selected_strip.plugin_list.clear();
			//write_to_itm(std::to_string(message.GetTypeList().size()));
			int plugin_quantity = (message.GetTypeList().size() - 1) / 3;
			std::string type_list = message.GetTypeList();
			for(int i = 0; i < plugin_quantity; i++)
				local_strip_data.selected_strip.plugin_list.push_back(message.get_string(i + 2 + i * 2));
			ardour_sender_receiver.request_plugin_descriptor(local_strip_data.selected_strip.number, local_strip_data.selected_strip.get_selected_plugin_index());
			continue;
		}
		else if(!message.GetAddress().compare("/strip/plugin/descriptor")){
			struct plugin_parameter parameter_data;

			parameter_data.name = message.get_string(3);
			parameter_data.parameter_id = message.get_int(2);
			parameter_data.flags = message.get_int(4);
			parameter_data.data_type = message.get_string(5);
			parameter_data.min_value = message.get_float(6);
			parameter_data.max_value = message.get_float(7);
			parameter_data.scale_points = message.get_string(8);
			parameter_data.zero_or_more_scale_points = message.get_int(9);
			std::string type_list = message.GetTypeList();
			parameter_data.value = type_list.at(10) == 'f' ? message.get_float(10) : message.get_double(10);
			if((parameter_data.parameter_id < MAX_PLUGIN_PARAMETERS) && parameter_data.parameter_id > 0)
				local_strip_data.selected_strip.selected_plugin[parameter_data.parameter_id] = parameter_data;
			continue;
		}
		else if(!message.GetAddress().compare("/strip/plugin/descriptor_end")){
			continue;
		}
	}while(1);
}

void strip_feedback::send_data(enum controller::controller_message type, float value)
{
	switch(type){
	case controller::STRIP_VOLUME:
		this->volume = value;
		break;
	case controller::REC_ENABLE:
		this->rec = int(value);
		break;
	case controller::SOLO:
		this->solo = bool(value);
		break;
	case controller::MUTE:
		this->mute = bool(value);
		break;
	case controller::STEREO_POSITION:
		this->stereo_position = value;
		break;
	default:
		break;
	}
}

void strip_feedback::send_data(enum controller::controller_message type, std::string string)
{
	switch(type)
	{
	case controller::STRIP_NAME:
		this->name = string;
		break;
	default:
		break;
	}
}

//this shoudl also exist for the selected strip in osc...
void ArdourSenderReceiver::get_plugin_list(int strip_number)
{
	OscMessage message("/strip/plugin/list");
	message.PushInt(strip_number);
	send_data(message);
	return;
}

void ArdourSenderReceiver::request_plugin_descriptor(int selected_strip_number, int selected_plugin_index)
{
	OscMessage message("/strip/plugin/descriptor");
	message.PushInt(selected_strip_number);
	message.PushInt(selected_plugin_index);
	send_data(message);
	return;
}

void ArdourSenderReceiver::send_data(OscMessage message)
{
	int size = 0;
	char* data = message.GetBytes(size);
	UdpSenderReceiver::send_data(data, size);
}

OscMessage ArdourSenderReceiver::receive_data()
{
	char buffer[1024];
	int length = UdpSenderReceiver::receive_data(buffer);
	OscMessage message(buffer, length);
	return message;
}

//code duplication!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

void OscController::switch_channel_mode_without_updating_mackie(channel_mode input)
{
	switch(input){
	case SendMode:
		this->mode = SendMode;
		break;
	case PluginMode:
		this->mode = PluginMode;
		break;
	case PanMode:
		this->mode = PanMode;
		break;
	default:
		break;
	}
	return;
}

void OscController::switch_channel_mode()
{
	switch(this->mode){
	case PanMode:
		this->mode = PluginMode;
		this->mackie_sender_receiver->update_display(this->local_strip_data.selected_strip.selected_plugin, &this->plugin_multiplexer, this->local_strip_data.selected_strip.plugin_bank);
		this->mackie_sender_receiver->update_faders(this->local_strip_data.selected_strip.selected_plugin, &this->plugin_multiplexer, this->local_strip_data.selected_strip.plugin_bank);
		display_object.update_display(OscMessage("/select/plugin/parameter/name"), this->local_strip_data.selected_strip.get_selected_plugin_parameter_names(&this->plugin_multiplexer));
		display_object.update_display(this->mode);
		break;
		
	case PluginMode:
		this->mode = SendMode;
		this->mackie_sender_receiver->update_display(this->local_strip_data.selected_strip.sends);
		this->mackie_sender_receiver->update_faders(this->local_strip_data.selected_strip.sends);
		display_object.update_display(OscMessage("/select/send_name"), this->local_strip_data.selected_strip.get_selected_strip_send_names()); 
		display_object.update_display(this->mode);
		break;	
	case SendMode:
		this->mode = PanMode;
		this->mackie_sender_receiver->update_display(this->local_strip_data.strips);
		this->mackie_sender_receiver->update_faders(this->local_strip_data.strips);
		display_object.update_display(this->mode);
		break;
	default:
		break;
	}
}

void ArdourSenderReceiver::init_osc_controller()
{
	OscMessage setup_msg("/set_surface");
	setup_msg.PushInt(8);
	setup_msg.PushInt((1 << AudioTracks) | (1 << MidiTracks));
	setup_msg.PushInt((1 << StripButtons) | (1 << StripControls) | (1 << ExtraSelectOnlyFeedback)/* | (1 << MeteringasFloat)*/);
	setup_msg.PushInt(0b1000);
	this->send_data(setup_msg);
}




