#pragma once
#include <vector>
#include <string>
#include <array>

struct plugin_parameter{
	//int ssid;
	//int plugin_id;
	int parameter_id;
	std::string name;
	int flags;
	std::string data_type;
	float min_value;
	float max_value;
	std::string scale_points;
	int zero_or_more_scale_points;
	volatile float value;
};

struct plugin_routing{
	std::string plugin_name;
	//why do we actually need to contain the index in this struct????
	//I think it's unnecessary because each controller has a defined knob count...
	//that means, this list saved on the disk (from_controller), the index of the knob is
	//the line number where the corresponding plugin_parameter_index is written in the file...
	//in case there is no plugin_parameter corresponding to the respective knob, we just write 0...
	//per line we can add than also other parameters as knob color, panel color, text background color, knob or switch mode...
	//and for that we need multiple dimensions, but not for the index...
	std::vector<int> from_controller; 
	std::vector<int> knob_colour;
	std::vector<int> panel_color;
	std::vector<int> label_colour;
};

struct plugin_multiplexer_struct{
	std::vector<unsigned char> plugin_multiplexer_from_plugin;
	std::vector<unsigned char> plugin_multiplexer_from_controller;
	int get_plugin_to_controller(int plugin_index);
	int get_controller_to_plugin(int controller_index);
	std::vector<plugin_routing> plugin_multiplexer;
	void initialize_plugin_multiplexer();
	void initialize_plugin_multiplexer_from_controller_and_from_plugin();
	void setup(std::string plugin_name);
};

