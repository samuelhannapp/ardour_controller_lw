#include "PluginMultiplexer.hpp"
#include "Defines.hpp"
#include <array>
#include <fstream>
#include <filesystem>


void plugin_multiplexer_struct::setup(std::string plugin_name)
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
	for(std::array<int, 2> temp : plugin_multiplexer.at(plugin_index).from_controller)
		if(temp[1] > from_plugin_size)
			from_plugin_size = temp[1];

	plugin_multiplexer_from_plugin.resize(from_plugin_size + ONE_BASED, 0);

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

void plugin_multiplexer_struct::initialize_plugin_multiplexer()
{
#ifdef __linux__
	std::string path = "/home/samuel/Documents/plugin_data";
#endif

#ifdef _WIN64
	std::string path = "C:\\Users\\Samuel\\Documents\\plugin_data";
#endif

	std::vector<std::string> file_locations;
	for (const auto& entry : std::filesystem::directory_iterator(path))
		file_locations.push_back(entry.path().string());

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
		int parameter_index = 0;
		while (std::getline(file, line)) {
			parameter_index++;
			plugin_multiplexer.at(temp_plugin_index).from_controller.push_back(std::array<int, 2>{parameter_index, std::stoi(line)});
		}

		temp_plugin_index++;

	}
}

void plugin_multiplexer_struct::initialize_plugin_multiplexer_from_controller_and_from_plugin()
{
	for(int i = 0; i < MAX_PLUGIN_PARAMETERS; i++){
		plugin_multiplexer_from_plugin.push_back(i);
		plugin_multiplexer_from_controller.push_back(i);
	}
}
