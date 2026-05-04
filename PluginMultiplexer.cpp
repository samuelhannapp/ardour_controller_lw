#include "PluginMultiplexer.hpp"
#include "Defines.hpp"
#include <array>
#include <fstream>
#include <filesystem>


plugin_multiplexer_c::plugin_multiplexer_c()
{
	this->initialize_plugin_multiplexer();
	this->initialize_plugin_multiplexer_from_controller_and_from_plugin();
}

void plugin_multiplexer_c::setup(std::string plugin_name)
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
	knob_colour.clear();


	////////////////////////////////////////////////////////////////////////////////
	bool plugin_routing_does_not_exist = plugin_index == plugin_multiplexer.size();
	if(plugin_routing_does_not_exist){
		//rout the plugin_just as it is...
		plugin_multiplexer_from_plugin.resize(32 + ONE_BASED);
		plugin_multiplexer_from_controller.resize(32 + ONE_BASED);
		for(int i = 1; i <= 32; i++){
			plugin_multiplexer_from_controller[i] = i;
		}

		for(int i = 1; i <= 32; i++){
			plugin_multiplexer_from_plugin[i] = i;
		}

		return;
	}

	////////////////////////////////////////////////////////////////////////////////////
	int from_plugin_size = 0;
	for(int temp : plugin_multiplexer.at(plugin_index).from_controller)
		if(temp > from_plugin_size)
			from_plugin_size = temp;

	plugin_multiplexer_from_plugin.resize(from_plugin_size + ONE_BASED, 0);

	int from_controller_size = plugin_multiplexer.at(plugin_index).from_controller.size();
	plugin_multiplexer_from_controller.resize(from_controller_size + 1, 0);

	int knob_colour_size = plugin_multiplexer.at(plugin_index).knob_colour.size();
	struct rgb_colour standart_colour;
	standart_colour.r = 255;
	standart_colour.g = 255;
	standart_colour.b = 255;
	this->knob_colour.resize(knob_colour_size + 1, standart_colour);

	////////////////////////////////////////////////////////////////////////////////////////////
	for(int i = 0; i < plugin_multiplexer.at(plugin_index).from_controller.size(); i++){
		plugin_multiplexer_from_controller[i] = plugin_multiplexer.at(plugin_index).from_controller[i];
	}

	for (int i = 0; i < from_controller_size; i++) {
		plugin_multiplexer_from_plugin[plugin_multiplexer_from_controller[i]] = i + 1;
	}

	for (int i = 0; i < knob_colour_size; i++)
		knob_colour.at(i) = plugin_multiplexer.at(plugin_index).knob_colour[i];

	return;
}

int plugin_multiplexer_c::get_plugin_to_controller(int plugin_parameter_index)
{
	if (plugin_parameter_index < this->plugin_multiplexer_from_plugin.size())
		return this->plugin_multiplexer_from_plugin[plugin_parameter_index];
	else 
		return 0;
}

struct rgb_colour plugin_multiplexer_c::get_knob_colour(int controller_parameter_index)
{
	if(this->knob_colour.size() > 1)
		return this->knob_colour.at(controller_parameter_index);

	return this->knob_colour[0];
}

std::vector<struct rgb_colour> plugin_multiplexer_c::get_knob_colour_list()
{
	return this->knob_colour;
}

int plugin_multiplexer_c::get_controller_to_plugin(int controller_knob_index)
{
	if (controller_knob_index < this->plugin_multiplexer_from_controller.size())
		return this->plugin_multiplexer_from_controller[controller_knob_index];
	else 
		return 0;
}

int plugin_multiplexer_c::get_controller_size()
{
	return this->plugin_multiplexer_from_controller.size();
}

int plugin_multiplexer_c::get_plugin_size()
{
	return this->plugin_multiplexer_from_plugin.size();
}

std::vector<int> plugin_multiplexer_c::split_string(std::string line)
{
	std::vector<int> list;
	std::size_t chars_processed = 1;

	while (line.size()) {
		list.push_back(std::stoi(line, &chars_processed));
		line.erase(0, int(chars_processed));
	}

	return list;
}

void plugin_multiplexer_c::initialize_plugin_multiplexer()
{
#ifdef __linux__
	std::string path = "plugin_data";
#endif

#ifdef _WIN64
	std::string path = "C:\\Users\\Samuel\\Software\\ArdourOscController_Build\\windows_build\\plugin_data";
#endif

	std::vector<std::string> file_locations;
	for (const auto& entry : std::filesystem::directory_iterator(path))
		file_locations.push_back(entry.path().string());
	
	plugin_multiplexer.erase(plugin_multiplexer.begin(), plugin_multiplexer.end());

	struct plugin_routing temp;
	plugin_multiplexer.resize(file_locations.size(), temp);



	std::string line;
	int temp_plugin_index = 0;
	for (std::string file_location : file_locations) {
		std::string plugin_name = file_location;

#ifdef __linux__
		int name_position = plugin_name.find_last_of('/') + 1;
#endif

#ifdef _WIN64
		int name_position = plugin_name.find_last_of('\\') + 1;
#endif
		plugin_name.erase(0, name_position);

		int file_suffix_position = plugin_name.find(".txt");
		if(file_suffix_position == -1) //if it's not a .txt file
			continue;
		plugin_name.erase(file_suffix_position, plugin_name.size());
		plugin_multiplexer.at(temp_plugin_index).plugin_name = plugin_name;

		std::ifstream file(file_location);
		while (std::getline(file, line)) {
			std::vector<int> list = split_string(line);
			plugin_multiplexer.at(temp_plugin_index).from_controller.push_back(std::stoi(line));

			if (list.size() == 4) {
				struct rgb_colour temp_knob_colour;
				temp_knob_colour.r = list[1];
				temp_knob_colour.g = list[2];
				temp_knob_colour.b = list[3];
				plugin_multiplexer.at(temp_plugin_index).knob_colour.push_back(temp_knob_colour);
			}
						
		}

		temp_plugin_index++;

	}
}

void plugin_multiplexer_c::initialize_plugin_multiplexer_from_controller_and_from_plugin()
{
	for(int i = 0; i < MAX_PLUGIN_PARAMETERS; i++){
		plugin_multiplexer_from_plugin.push_back(i);
		plugin_multiplexer_from_controller.push_back(i);
	}
}
