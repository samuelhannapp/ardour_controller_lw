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

struct rgb_colour {
	int r; int g; int b;
};

struct plugin_routing{
	std::string plugin_name;
	std::vector<int> from_controller; 
	std::vector<rgb_colour> knob_colour;
	std::vector<rgb_colour> panel_color;
	std::vector<rgb_colour> label_colour;
};

class plugin_multiplexer_c{
public:
	plugin_multiplexer_c();
	void initialize_plugin_multiplexer();
	int get_plugin_to_controller(int plugin_parameter_index);
	struct rgb_colour get_knob_colour(int controller_parameter_index);
	int get_controller_to_plugin(int controller_index);
	void setup(std::string plugin_name);
	int get_controller_size();
	int get_plugin_size();

private:
	std::vector<unsigned char> plugin_multiplexer_from_plugin;
	std::vector<unsigned char> plugin_multiplexer_from_controller;
	std::vector<rgb_colour> knob_colour;

	std::vector<plugin_routing> plugin_multiplexer;

	void initialize_plugin_multiplexer_from_controller_and_from_plugin();

	std::vector<int> split_string(std::string line);
};
