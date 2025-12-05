#include "osc_controller.hpp"
osc_controller::osc_controller(std::string destination_ip_address, unsigned int udp_port_in, unsigned int udp_port_out)
{
	this->plugin_multiplexer.initialize_plugin_multiplexer();
	this->local_strip_data.selected_strip.initialize_selected_plugin_descriptor();
	this->local_strip_data.selected_strip.initialize_selected_strip_sends();
	this->local_strip_data.selected_strip.initialize_selected_strip_plugin_list();
	this->local_strip_data.selected_strip.initialize_selected_strip();
	this->plugin_multiplexer.initialize_plugin_multiplexer_from_controller_and_from_plugin();
	this->mackie_sender_receiver.initialize_mackie_display_formated();

    this->ardour_sender_receiver.initialize_udp(destination_ip_address, udp_port_in, udp_port_out);
	this->mackie_sender_receiver.initialize_midi(2, 2);

    std::thread mackie_control_thread(&osc_controller::midi_receive_thread, this);
	mackie_control_thread.detach();
	
	std::thread ardour_thread(&osc_controller::osc_receive_thread, this);
	ardour_thread.detach();

	this->ardour_sender_receiver.init_osc_controller();
}

void osc_controller::udp_sender_receiver::initialize_udp(std::string destination_ip_address, unsigned int udp_port_in, unsigned int udp_port_out)
{
	struct hostent* h;
	//check ip adress
	h = gethostbyname(destination_ip_address.c_str());
	if (h == NULL) {
		std::cout << ("Invalid IP Address!");
		return;
		//exit (EXIT_FAILURE);
	}
	m_destinationAddress.sin_family = h->h_addrtype;
	memcpy((char*)&m_destinationAddress.sin_addr.s_addr, h->h_addr_list[0], h->h_length);
	m_destinationAddress.sin_port = htons(udp_port_out);
	// Open the network socket
	m_nativeSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (m_nativeSocket < 0) {
		std::cout << ("Cannot open Socket!");
		return;
		//exit (EXIT_FAILURE);
	}
	//Bind network socket
	sockaddr_in m_localAddress;
	m_localAddress.sin_family = AF_INET;
	m_localAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	m_localAddress.sin_port = htons(udp_port_in);
	int result = bind(m_nativeSocket, (struct sockaddr*)&m_localAddress, sizeof(m_localAddress));
	if (result < 0) {
		std::cout << ("Failed to bind to network socket!");
		return;
		//exit (EXIT_FAILURE);
	}
}

void osc_controller::midi_receive_thread()
{
	char buffer[3];

	while (1) {
	mackie_sender_receiver.receive_midi_data(buffer);	

    OscMessage msg("/nothing");
	bool button_pressed = false;
	int channel_nr = 0;
	int fader_nr = 0;
	int value_14_bit = 0;
	float value_float = 0;
	float increment = 0;

	enum mackie::button_type type = (enum mackie::button_type)(buffer[1] / STRIPS_PER_CONTROLLER);

	int command = buffer[0] & 0xf0;
	switch(command) {
	//case 0x80: // Note off
	case 0x90: // Note on
  		button_pressed = buffer[2];
		channel_nr = buffer[1] % STRIPS_PER_CONTROLLER + ONE_BASED;

		if(!button_pressed)
			if(!(type == mackie::FADER_TOUCH))
				break;

		switch(type){
			case mackie::RECORD:
				msg = OscMessage("/strip/recenable");
				msg.PushInt32(channel_nr);
				msg.PushInt32(!local_strip_data.strips[channel_nr].rec);
				break;
			case mackie::SOLO:
				msg = OscMessage("/strip/solo");
				msg.PushInt32(channel_nr);
				msg.PushInt32(!local_strip_data.strips[channel_nr].solo);
				break;
			case mackie::MUTE:
				msg = OscMessage("/strip/mute");
				msg.PushInt32(channel_nr);
				msg.PushInt32(!local_strip_data.strips[channel_nr].mute);
				break;
			case mackie::SELECT:
				local_strip_data.selected_strip.update_selected_strip(controller::SELECT, channel_nr, 1);
				msg = OscMessage("/strip/select");
				msg.PushInt32(channel_nr);
				msg.PushInt32(0);
				ardour_sender_receiver.send_udp_data(msg);
                break;
			case mackie::KNOB_PUSH:
				switch(channel_nr){
					case 1:
						this->switch_channel_mode_without_updating_mackie(PanMode);
						msg = OscMessage("/set_surface/strip_types");
						msg.PushInt32(1 << AudioBusses);
						break;
					case 2:
						this->switch_channel_mode_without_updating_mackie(PanMode);
						msg = OscMessage("/set_surface/strip_types");
						msg.PushInt32(1 << VCAs);
						break;
					case 3:
						if(this->mode == PanMode)
							msg = OscMessage("/bank_down");
						if(this->mode == PluginMode){
							if(local_strip_data.selected_strip.plugin_bank > 0)
								local_strip_data.selected_strip.plugin_bank--;
							this->mackie_sender_receiver.update_display(this->local_strip_data.selected_strip.selected_plugin, &this->plugin_multiplexer, this->local_strip_data.selected_strip.plugin_bank);
                            this->mackie_sender_receiver.update_faders(this->local_strip_data.selected_strip.selected_plugin, &this->plugin_multiplexer, this->local_strip_data.selected_strip.plugin_bank);
						}
						if(this->mode == SendMode){
							msg = OscMessage("/select/send_page");
							msg.PushFloat(-1);
						}
						break;
					case 4:
						if(this->mode == PanMode)
							msg = OscMessage("/bank_up");
						if(this->mode == PluginMode){
							if(local_strip_data.selected_strip.plugin_bank < PLUGIN_PAGES_SIZE)
								local_strip_data.selected_strip.plugin_bank++;

							mackie_sender_receiver.update_display(this->local_strip_data.selected_strip.selected_plugin, &this->plugin_multiplexer, this->local_strip_data.selected_strip.plugin_bank);
                            mackie_sender_receiver.update_faders(this->local_strip_data.selected_strip.selected_plugin, &this->plugin_multiplexer, this->local_strip_data.selected_strip.plugin_bank);
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
						msg.PushInt32(this->local_strip_data.selected_strip.number);
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
				msg.PushInt32(channel_nr);
				msg.PushInt(button_pressed);
				break;
			default:
				break;
	  	}
		ardour_sender_receiver.send_udp_data(msg);
		break;
	case 0xa0: // Aftertouch
	case 0xB0: // Continuous controller
		channel_nr = buffer[1] % STRIPS_PER_CONTROLLER + 1;
		if(this->mode == PanMode){
			msg = OscMessage("/strip/pan_stereo_position");
			msg.PushInt32(channel_nr);
			increment = buffer[2] == 0x1 ? -1 : 1;
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
			msg.PushInt32(plugin_parameter_number);

			increment = buffer[2] == 0x1 ? 1 : -1;

			float value_float = this->local_strip_data.selected_strip.selected_plugin[plugin_parameter_number].value + increment * 0.03;
			if(value_float < 0) value_float = 0;
			if(value_float > 1) value_float = 1;

			if(this->local_strip_data.selected_strip.selected_plugin[plugin_parameter_number].flags & (1 << TOGGLED))
				value_float = increment == 1 ? 1 : 0;

			msg.PushFloat(value_float);
		}
		if(this->mode == SendMode){
			msg = OscMessage("/select/send_fader");
			msg.PushInt32(channel_nr);
			increment = buffer[2] == 0x1 ? 1 : -1;
			value_float = this->local_strip_data.selected_strip.sends[channel_nr].volume + increment * 0.03;
			if(value_float < 0) value_float = 0;
			if(value_float > 1) value_float = 1;
			msg.PushFloat(value_float);
		}
		ardour_sender_receiver.send_udp_data(msg);
		break;
	case 0xC0: // Patch change
	case 0xD0: // Channel Pressure
	case 0xE0: // Pitch bend
		fader_nr = buffer[0] & 0xf;
		value_14_bit = buffer[1] | (buffer[2] << 7);
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
			msg.PushInt32(plugin_parameter_number);
			msg.PushFloat(value_float);
		}
		if(this->mode == SendMode){
			msg = OscMessage("/select/send_fader");
			msg.PushInt32(fader_nr + 1);
			msg.PushFloat(value_float);
		}
		ardour_sender_receiver.send_udp_data(msg);
		break;
	case 0xF0: // (non-musical commands)
    	break;
	default: 
		break;
    }
	//printf("buffer[0] = %x buffer[1] = %x buffer[2] = %x\n", buffer[0], buffer[1], buffer[2]);
	//fflush(stdout);
   }
}

void osc_controller::osc_receive_thread()
{
	while(1){
		char buffer[1024];
		int buffer_length = 0;
		buffer_length = ardour_sender_receiver.receive_udp_data(buffer);			
		OscMessage message = OscMessage(buffer, buffer_length);
		//std::cout << message.GetAddress() << "\n";

		if(!message.GetAddress().compare(0, 13, "/strip/fader\0")){
			int strip_nr = message.get_int(0);
			float value = message.get_float(1);
            local_strip_data.strips[strip_nr].update(controller::STRIP_VOLUME, value);
			if(this->mode == PanMode)
				mackie_sender_receiver.update(controller::STRIP_VOLUME, strip_nr, value);
			//update_fader(strip_nr, value, local_strip_data.mode);
			continue;
		}
		else if( !message.GetAddress().compare("/strip/recenable")){
			int strip_nr = message.get_int(0);
			float value = message.get_float(1);
			local_strip_data.strips[strip_nr].update(controller::REC_ENABLE, value);
			mackie_sender_receiver.update(controller::REC_ENABLE, strip_nr, value);
			continue;
		}
		else if(!message.GetAddress().compare("/strip/solo")){
			int strip_nr = message.get_int(0);
			float value = message.get_float(1);
			local_strip_data.strips[strip_nr].update(controller::SOLO, value);
			mackie_sender_receiver.update(controller::SOLO, strip_nr, value);
			continue;
		}
		else if(!message.GetAddress().compare("/strip/mute")){
			int strip_nr = message.get_int(0);
			float value = message.get_float(1);
			local_strip_data.strips[strip_nr].update(controller::MUTE, value);
			mackie_sender_receiver.update(controller::MUTE, strip_nr, value);
			continue;
		}
		else if(!message.GetAddress().compare("/strip/select")){
			int strip_nr = message.get_int(0);
			float value = message.get_float(1);
			local_strip_data.selected_strip.update_selected_strip(controller::SELECT, strip_nr, value);
			mackie_sender_receiver.update(controller::SELECT, strip_nr, value);
			continue;
		}
		else if(!message.GetAddress().compare("/strip/pan_stereo_position")){
			int strip_nr = message.get_int(0);
			float value = message.get_float(1);
			local_strip_data.strips[strip_nr].update(controller::STEREO_POSITION, value);
			mackie_sender_receiver.update(controller::STEREO_POSITION, strip_nr, value);
			continue;
		}
		else if(!message.GetAddress().compare("/strip/meter")){
			int strip_nr = message.get_int(0);
			float value = message.get_float(1);

			mackie_sender_receiver.update(controller::METER, strip_nr, value);
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
			if(this->mode == SendMode)
				mackie_sender_receiver.update_display(this->local_strip_data.selected_strip.sends);
			continue;
		}
		else if(!message.GetAddress().compare("/select/send_fader")){
			int send_id = message.get_int(0);
			float value = message.get_float(1);
			local_strip_data.selected_strip.update_selected_strip(controller::SEND_FADER, send_id, value);
			if(this->mode == SendMode)
				mackie_sender_receiver.update(controller::SEND_FADER, send_id, value);
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
			this->local_strip_data.selected_strip.update_selected_strip(controller::PLUGIN_PARAMETER_NAME, plugin_parameter_id, plugin_parameter_name);
			if(this->mode == PluginMode)
				this->mackie_sender_receiver.update_display(this->local_strip_data.selected_strip.selected_plugin, &this->plugin_multiplexer, this->local_strip_data.selected_strip.plugin_bank);
			continue;
		}
		else if(!message.GetAddress().compare("/select/plugin/parameter")){
			int plugin_parameter_id = message.get_int(0);
			float plugin_parameter_value = message.get_type_list().at(1) == 'f' ? message.get_float(1) : message.get_double(1);
			local_strip_data.selected_strip.update_selected_strip(controller::PLUGIN_PARAMETER_VALUE, plugin_parameter_id, plugin_parameter_value);
			int fader_id = plugin_multiplexer.plugin_multiplexer_from_plugin[plugin_parameter_id];
			if(local_strip_data.selected_strip.controller_channel_nr_is_within_plugin_bank(fader_id) && (this->mode == PluginMode)){
				mackie_sender_receiver.update(controller::PLUGIN_PARAMETER_VALUE, fader_id, plugin_parameter_value);
			}
			continue;
		}
		else if(!message.GetAddress().compare("/strip/name")){
			int strip_nr = message.get_int(0);
			std::string strip_name = message.get_string(1);
			local_strip_data.strips[strip_nr].update(controller::STRIP_NAME, strip_name);
			if(this->mode == PanMode)
				mackie_sender_receiver.update_display(this->local_strip_data.strips);
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
			if(parameter_data.parameter_id < MAX_PLUGIN_PARAMETERS)
				local_strip_data.selected_strip.selected_plugin[parameter_data.parameter_id] = parameter_data;
			continue;
		}
		else if(!message.GetAddress().compare("/strip/plugin/descriptor_end")){
			continue;
		}
	}
}

int osc_controller::udp_sender_receiver::receive_udp_data(char *buffer)	
{
	struct sockaddr_in sender_address;
	int sender_address_size = sizeof(sender_address);
	int buffer_length = 1024;
	recvfrom(m_nativeSocket, buffer, buffer_length, 0, (struct sockaddr*)&sender_address, (socklen_t*)&sender_address_size);
	return buffer_length;
}

void osc_controller::midi_sender_receiver::receive_midi_data(char *buffer)
{
	snd_rawmidi_read(MidiDeviceIn, buffer, 3); 
	return;	
}

void osc_controller::strip_feedback::update(enum controller::controller_message type, float value)
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

void osc_controller::strip_feedback::update(enum controller::controller_message type, std::string string)
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

void osc_controller::midi_sender_receiver::update(enum controller::controller_message type, int strip_nr, float value)
{
	switch(type){
	case controller::STRIP_VOLUME:
	case controller::PLUGIN_PARAMETER_VALUE:
		{
		uint16_t value_14_bit = uint16_t(value * float(MAX_14_BIT));
    	uint8_t value_low = value_14_bit & 0x7f;
    	uint8_t value_high = ((value_14_bit & 0x3f80) >> 7);
        unsigned char midi_message[3]  = {(unsigned char)(0xe0 | (strip_nr - 1)), value_low, value_high};
	    send_midi_data(midi_message, 3);
		}
		break;
	case controller::REC_ENABLE:
		{
		unsigned char button_nr = mackie::RECORD * STRIPS_PER_CONTROLLER + strip_nr -1;
		unsigned char button_value = int(value) * 0x7f;
		unsigned char midi_message[3]  = {0x90, button_nr, button_value};
	    send_midi_data(midi_message, 3);
		}
		break;
	case controller::SOLO:
		{
		unsigned char button_nr = mackie::SOLO * STRIPS_PER_CONTROLLER + strip_nr -1;
    	unsigned char button_value = int(value) * 0x7f;
		unsigned char midi_message[3]  = {0x90, button_nr, button_value};
	    send_midi_data(midi_message, 3);
		}
		break;
	case controller::MUTE:
		{
		unsigned char button_nr = mackie::MUTE * STRIPS_PER_CONTROLLER + strip_nr -1;
    	unsigned char button_value = int(value) * 0x7f;
		unsigned char midi_message[3]  = {0x90, button_nr, button_value};
	    send_midi_data(midi_message, 3);
		}
		break;
	case controller::SELECT:
		{
    	unsigned char button_nr = mackie::SELECT * STRIPS_PER_CONTROLLER + strip_nr -1;
    	unsigned char button_value = int(value);
    	button_value = button_value * 0x7f;
		unsigned char midi_message[3]  = {0x90, button_nr, button_value};
	    send_midi_data(midi_message, 3);
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
	    send_midi_data(midi_message, 3);
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
	    send_midi_data(midi_message, 3);
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
	    send_midi_data(midi_message, 3);
		}
		break;
	default:
		break;
	}
}

void osc_controller::midi_sender_receiver::send_midi_data(unsigned char * message, int size)
{
	snd_rawmidi_write(MidiDeviceOut, message, size);	
}

//this is now in update_selected_strip...
/*
void update_global_state(enum controller::controller_message type, int strip_nr, float value)
{
	switch(type)
	{
	case controller::SELECT:
		if(bool(value))
    		selected_strip.number = strip_nr;
		break;
	}
}
*/

void osc_controller::selected_strip_struct::update_selected_strip(enum controller::controller_message type, int nr, float value)
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

void osc_controller::selected_strip_struct::update_selected_strip(enum controller::controller_message type, int nr, std::string string)
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

void osc_controller::midi_sender_receiver::update_display(const strip_feedback *strips)
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
	send_midi_data(mackie_display.MIDI_TX_SYSX_Buffer, 120);
}

void osc_controller::midi_sender_receiver::update_display(const send *sends)
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
	send_midi_data(mackie_display.MIDI_TX_SYSX_Buffer, 120);
}

void osc_controller::midi_sender_receiver::update_display(const plugin_parameter *selected_plugin, const plugin_multiplexer_struct *plugin_multiplexer, int plugin_bank)
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
	send_midi_data(mackie_display.MIDI_TX_SYSX_Buffer, 120);
}

void osc_controller::mackie_display_struct::fill_sysx_buffer()
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

void osc_controller::mackie_display_struct::prepare_strip_names(const strip_feedback *strips)
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

void osc_controller::mackie_display_struct::prepare_selected_plugin_parameter_names(const selected_strip_struct *selected_strip, const plugin_multiplexer_struct *plugin_multiplexer)
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
/*
void osc_controller::mackie_display_struct::prepare_strip_names()
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
*/
/*
void osc_controller::mackie_display_struct::prepare_selected_plugin_parameter_names()
{
	for(int i = 0; i < STRIPS_PER_CONTROLLER; i++){
		int plugin_parameter_id = plugin_multiplexer_from_controller[i + selected_strip.plugin_bank * STRIPS_PER_CONTROLLER + 1];
		this->mackie_display_formated.at(i) = selected_strip.selected_plugin[plugin_parameter_id].name;
	}
	size_t position = 0;
	for(int i = 0; i < STRIPS_PER_CONTROLLER; i++){
		position = this->mackie_display_formated.at(i).find(' ');
		if(position < 6)
			this->mackie_display_formated.at(i).insert(position, 7 - position - 1, ' ');
	}
}
*/

void osc_controller::mackie_display_struct::prepare_selected_strip_send_names(const send *sends)
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

void osc_controller::plugin_multiplexer_struct::setup(std::string plugin_name)
{
	unsigned int plugin_index = 0;
	for(struct plugin_routing index : plugin_multiplexer){
		if(!index.plugin_name.compare(plugin_name))
			break;
		plugin_index++;
	}

	////////////////////////////////////////////////////////////////////////////
	plugin_multiplexer_from_plugin.clear();
	plugin_multiplexer_from_controller.clear();

	////////////////////////////////////////////////////////////////////////////////
	bool plugin_routing_does_not_exist = plugin_index == plugin_multiplexer.size();
	if(plugin_routing_does_not_exist){
		//rout the plugin_just as it is...
		for(int i = 1; i <= 32; i++){
			plugin_multiplexer_from_controller[i] = i;
		}

		for(int i = 1; i <= 32; i++){
			plugin_multiplexer_from_plugin[i] = i;
		}
	}

	////////////////////////////////////////////////////////////////////////////////////
	int from_plugin_size = 0;
	for(std::array<int, 2> temp : plugin_multiplexer.at(plugin_index).from_controller)
		if(temp[1] > from_plugin_size)
			from_plugin_size = temp[1];

	plugin_multiplexer_from_plugin.resize(from_plugin_size + 1, 0);

	int from_controller_size = 0;
	for(std::array<int, 2> temp : plugin_multiplexer.at(plugin_index).from_controller)
		if(temp[0] > from_controller_size)
			from_controller_size = temp[0];

	plugin_multiplexer_from_controller.resize(from_controller_size + 1, 0);

	////////////////////////////////////////////////////////////////////////////////////////////
	for(std::array<int, 2> routing : plugin_multiplexer.at(plugin_index).from_controller){
		plugin_multiplexer_from_controller[routing[0]] = routing[1];
	}

	for(std::array<int, 2> routing : plugin_multiplexer.at(plugin_index).from_controller){
		plugin_multiplexer_from_plugin[routing[1]] = routing[0];
	}
	return;
}

//this shoudl also exist for the selected strip in osc...
void osc_controller::udp_sender_receiver::get_plugin_list(int strip_number)
{
	OscMessage message("/strip/plugin/list");
	message.PushInt(strip_number);
	send_udp_data(message);
	return;
}

bool osc_controller::selected_strip_struct::controller_channel_nr_is_within_plugin_bank(int fader_id)
{
	if((fader_id / STRIPS_PER_CONTROLLER) == this->plugin_bank)
		return true;

	return false;
}

void osc_controller::udp_sender_receiver::request_plugin_descriptor(int selected_strip_number, int selected_plugin_index)
{
	OscMessage message("/strip/plugin/descriptor");
	message.PushInt(selected_strip_number);
	message.PushInt(selected_plugin_index);
	send_udp_data(message);
	return;
}

void osc_controller::udp_sender_receiver::send_udp_data(OscMessage message)
{
	int size = 0;
	char* data = message.GetBytes(size);
	// Send data over the socket
	if (size < 1)
		return;
		// Send data over the socket
	sendto(m_nativeSocket, data, size, 0, (struct sockaddr*)&m_destinationAddress, sizeof(m_destinationAddress)); //thats exactly the same, we should get rid of that one
}

int osc_controller::selected_strip_struct::get_selected_plugin_index()
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
/*
void osc_controller::midi_sender_receiver::update_faders(enum channel_mode mode)
{
	switch(mode){
		case PluginMode:
			for(int i = 0; i < STRIPS_PER_CONTROLLER; i++){
				int plugin_parameter_number = plugin_multiplexer_from_controller[i + 1 + STRIPS_PER_CONTROLLER * selected_strip.plugin_bank];
				float value = selected_strip.selected_plugin[plugin_parameter_number].value;
				this->update(controller::PLUGIN_PARAMETER_VALUE, i + 1, value);
			}
			break;
		case PanMode:
			for(int i = 0; i < STRIPS_PER_CONTROLLER; i++){
				float value = strips[i + 1].volume;
				this->update(controller::STRIP_VOLUME, i + 1, value);
			}
			break;
		case SendMode:
			for(int i = 0; i < STRIPS_PER_CONTROLLER; i++){
				float value = selected_strip.sends[i + 1].volume;
				this->update(controller::STRIP_VOLUME, i + 1, value);
			}
			break;
	}
}
*/

//in case there would be a update of an array in the mackie control, we could do like for the display, 
//updating all at the same time...
//like putting all into a message than send 3 * 8 bytes in size...

void osc_controller::midi_sender_receiver::update_faders(const plugin_parameter *selected_plugin, const plugin_multiplexer_struct *plugin_multiplexer, int plugin_bank)
{
	for(int i = 0; i < STRIPS_PER_CONTROLLER; i++){
		int plugin_parameter_number = plugin_multiplexer->plugin_multiplexer_from_controller[i + ONE_BASED + STRIPS_PER_CONTROLLER * plugin_bank];
		float value = selected_plugin[plugin_parameter_number].value;
		this->update(controller::PLUGIN_PARAMETER_VALUE, i + ONE_BASED, value);
	}
}

void osc_controller::midi_sender_receiver::update_faders(const send *sends)
{
	for(int i = 0; i < STRIPS_PER_CONTROLLER; i++){
		float value = sends[i + ONE_BASED].volume;
		this->update(controller::STRIP_VOLUME, i + ONE_BASED, value);
	}	
}

void osc_controller::midi_sender_receiver::update_faders(const strip_feedback *strips)
{
	for(int i = 0; i < STRIPS_PER_CONTROLLER; i++){
		float value = strips[i + 1].volume;
		this->update(controller::STRIP_VOLUME, i + ONE_BASED, value);
	}
}


void osc_controller::switch_channel_mode_without_updating_mackie(channel_mode input)
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

void osc_controller::switch_channel_mode()
{
	switch(this->mode){
	case PanMode:
		this->mode = SendMode;
		this->mackie_sender_receiver.update_display(this->local_strip_data.selected_strip.sends);
		this->mackie_sender_receiver.update_faders(this->local_strip_data.selected_strip.sends);
		break;
	case SendMode:
		this->mode = PluginMode;
		this->mackie_sender_receiver.update_display(this->local_strip_data.selected_strip.selected_plugin, &this->plugin_multiplexer, this->local_strip_data.selected_strip.plugin_bank);
		this->mackie_sender_receiver.update_faders(this->local_strip_data.selected_strip.selected_plugin, &this->plugin_multiplexer, this->local_strip_data.selected_strip.plugin_bank);
		break;
	case PluginMode:
		this->mode = PanMode;
		this->mackie_sender_receiver.update_display(this->local_strip_data.strips);
		this->mackie_sender_receiver.update_faders(this->local_strip_data.strips);
		break;
	default:
		break;
	}
}

void osc_controller::udp_sender_receiver::init_osc_controller()
{
	OscMessage setup_msg("/set_surface");
	setup_msg.PushInt32(8);
	setup_msg.PushInt32((1 << AudioTracks) | (1 << MidiTracks));
	setup_msg.PushInt32((1 << StripButtons) | (1 << StripControls) | (1 << ExtraSelectOnlyFeedback)/* | (1 << MeteringasFloat)*/);
	setup_msg.PushInt32(0b1000);
	this->send_udp_data(setup_msg);
}

int osc_controller::midi_sender_receiver::initialize_midi(int port_in, int port_out)
{
	int status;
	std::string port_number = std::to_string(port_out);//this shoule be single digit for now...
	std::string port_name("hw:2,0,0");
	port_name.replace(3, 1, port_number);
	if ((status = snd_rawmidi_open(NULL, &MidiDeviceOut, port_name.c_str(), SND_RAWMIDI_SYNC)) < 0) {
      printf("Problem opening MIDI output: %s", snd_strerror(status));
	  return 1;
   }
	port_number = std::to_string(port_in);//this shoule be single digit for now...
	std::string port_name_2("hw:2,0,0");
	port_name_2.replace(3, 1, port_number);
	if ((status = snd_rawmidi_open(&MidiDeviceIn, NULL, port_name_2.c_str(), SND_RAWMIDI_SYNC)) < 0) {
      printf("Problem opening MIDI input: %s", snd_strerror(status));
      return 1;
   	}
	return 0;
}

void osc_controller::plugin_multiplexer_struct::initialize_plugin_multiplexer()
{
	std::string path = "/home/samuel/Software/hekky-osc-extension/examples/plugin_data";
	std::vector<std::string> file_locations;
	for (const auto & entry : std::filesystem::directory_iterator(path))
		file_locations.push_back(entry.path());

	struct plugin_routing temp;
	plugin_multiplexer.resize(file_locations.size(), temp);

	std::string line;
	int temp_plugin_index = 0;
	for(std::string file_location : file_locations){
		std::string plugin_name = file_location;
		int pos = plugin_name.find_last_of('/') + 1;
		plugin_name.erase(0, pos);
		pos = plugin_name.find(".txt");
		plugin_name.erase(pos, plugin_name.size());
		plugin_multiplexer.at(temp_plugin_index).plugin_name = plugin_name;

		std::ifstream file(file_location);
		int parameter_index = 0;
		while(std::getline(file, line)){
			parameter_index++;
			plugin_multiplexer.at(temp_plugin_index).from_controller.push_back(std::array<int, 2>{parameter_index, std::stoi(line)});
		}

		temp_plugin_index++;
	}
}

void osc_controller::plugin_multiplexer_struct::initialize_plugin_multiplexer_from_controller_and_from_plugin()
{
	for(int i = 0; i < MAX_PLUGIN_PARAMETERS; i++){
		plugin_multiplexer_from_plugin.push_back(i);
		plugin_multiplexer_from_controller.push_back(i);
	}
}

void osc_controller::selected_strip_struct::initialize_selected_plugin_descriptor()
{
	for(int i = 0; i < MAX_PLUGIN_PARAMETERS; i++){
		this->selected_plugin[i].name = std::string("default");
		this->selected_plugin[i].value = 0;
	}
}

void osc_controller::selected_strip_struct::initialize_selected_strip_sends()
{
	for (int i = 0; i < SEND_ARRAY_SIZE; i++)
		this->sends[i].name = std::string("default");
}

void osc_controller::selected_strip_struct::initialize_selected_strip_plugin_list(){
	for(int i = 0; i < 16; i++)
		this->plugin_list.push_back("default");
	return;
}

void osc_controller::selected_strip_struct::initialize_selected_strip()
{
	this->plugin_bank = 0;
}

void osc_controller::midi_sender_receiver::initialize_mackie_display_formated()
{
	std::string default_string("defa defa");
	for(int i = 0; i < STRIPS_PER_CONTROLLER; i++)
		mackie_display.mackie_display_formated.push_back(default_string);

	return;
}