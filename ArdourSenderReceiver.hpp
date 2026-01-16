#pragma once
#include "UdpSenderReceiver.hpp"
#include "oscmessage.hpp"
#include "PluginMultiplexer.hpp"
#include "Defines.hpp"



#define STRIP_ARRAY_SIZE (STRIPS_PER_CONTROLLER + ONE_BASED)

#define MAX_SEND_STRIPS 8
#define SEND_ARRAY_SIZE (MAX_SEND_STRIPS + ONE_BASED) 

#define MAX_PLUGIN_PARAMETERS 200



struct send{
		std::string name;
		volatile float volume;
		bool enable;
};

struct selected_strip_struct{
	int number;
	struct send sends[SEND_ARRAY_SIZE]; //we actually use here the ardour internal banking, so 
										//it should be 8 + ONE_BASED
	std::vector<std::string> plugin_list; //this is not feedback, so I cannot put it into the feedback struct...
	std::string selected_plugin_name;
	struct plugin_parameter selected_plugin[MAX_PLUGIN_PARAMETERS];
	int plugin_bank;
	//void update_global_state(enum controller::controller_message type, int strip_nr, float value);
	void update_selected_strip(enum controller::controller_message type, int nr, float value);
	void update_selected_strip(enum controller::controller_message type, int nr, std::string string);
	bool controller_channel_nr_is_within_plugin_bank(int fader_id);
	void initialize_selected_strip_sends();
	void initialize_selected_plugin_descriptor();
	void initialize_selected_strip_plugin_list();
	void initialize_selected_strip();
	int get_selected_plugin_index();
};

struct strip_feedback{
	std::string name;
	float volume;
	float stereo_position;
	bool rec;
	bool solo;
	bool mute;
	volatile bool fader_touch; //this is actually not feedback...
	void send_data(enum controller::controller_message type, float value);
	void send_data(enum controller::controller_message type, std::string string);
};

struct ardour_feedback_struct{
	struct strip_feedback strips[STRIP_ARRAY_SIZE]; //this is 1 based, 0 is not used... controller has 8 strip's
	struct selected_strip_struct selected_strip;
};

class ArdourSenderReceiver : public UdpSenderReceiver{
public:
	using UdpSenderReceiver::UdpSenderReceiver;
	void send_data(OscMessage message);
	OscMessage receive_data();
	void request_plugin_descriptor(int selected_strip_number, int selected_plugin_index);
	void get_plugin_list(int strip_number);
	void init_osc_controller();	
};

