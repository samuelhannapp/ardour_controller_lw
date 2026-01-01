#include "PluginMultiplexer.hpp"
#include "Defines.hpp"
#include <array>


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
