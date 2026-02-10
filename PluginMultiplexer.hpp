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
	std::vector<std::array<int, 2>> from_controller;
};

struct plugin_multiplexer_struct{
	std::vector<unsigned char> plugin_multiplexer_from_plugin;
	std::vector<unsigned char> plugin_multiplexer_from_controller;
	std::vector<plugin_routing> plugin_multiplexer;
	void initialize_plugin_multiplexer();
	void initialize_plugin_multiplexer_from_controller_and_from_plugin();
	void setup(std::string plugin_name);
};

