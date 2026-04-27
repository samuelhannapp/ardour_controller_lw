#pragma once
#include "Defines.hpp"
#ifdef MACKIE_CONTROL_MIDI_VERSION
#include "MackieSenderReceiverMidi.hpp"
#endif
#ifdef MACKIE_CONTROL_UDP_VERSION
#include "MackieSenderReceiverUdp.hpp"
#endif

#include "PluginMultiplexer.hpp"
#include "ArdourSenderReceiver.hpp"
class MackieControl : public MackieSenderReceiver{
public:
	using MackieSenderReceiver::MackieSenderReceiver;
	using MackieSenderReceiver::receive_data;
	void update_display(const struct strip_feedback *strips);
	void update_display(const struct send *sends);
	void update_display(const struct plugin_parameter *selected_plugin, plugin_multiplexer_c *plugin_multiplexer, int plugin_bank);
	void update_faders(const struct plugin_parameter *selected_plugin, plugin_multiplexer_c *plugin_multiplexer, int plugin_bank);
	void update_faders(const struct send *sends);
	void update_faders(const struct strip_feedback *strips);
	void prepare_strip_names(const struct strip_feedback *strips);
	void prepare_selected_plugin_parameter_names(const struct selected_strip_struct *selected_strip, plugin_multiplexer_c *plugin_multiplexer);
	void prepare_selected_strip_send_names(const struct send *sends);
};

