#pragma once
#include <functional>


#include <stdint.h>
#include <vector>
#include <string>

#include "PluginMultiplexer.hpp"
#include "ArdourSenderReceiver.hpp"
#include "MidiSenderReceiver.hpp"


#define SYSX_BUFFER_SIZE 120
#define MAX_14_BIT (16383.0)

namespace mackie{
    enum button_type {
        RECORD, SOLO, MUTE, SELECT, KNOB_PUSH, LED_STRIP = 6, FADER_TOUCH = 13
    };
}

struct mackie_display_struct{
	uint8_t MIDI_TX_SYSX_Buffer[SYSX_BUFFER_SIZE];
	std::vector<std::string> mackie_display_formated;
	void prepare_strip_names(const struct strip_feedback *strips);
	void prepare_selected_plugin_parameter_names(const struct selected_strip_struct *selected_strip, const plugin_multiplexer_struct *plugin_multiplexer);
	void prepare_selected_strip_send_names(const struct send *sends);
	void fill_sysx_buffer();
};

class MackieSenderReceiver : public MidiSenderReceiver{
public:
	struct mackie_display_struct mackie_display;
	std::function<void(char buffer[3])> signal_data_to_outside;
	void initialize_mackie_display_formated();
	void send_data(enum controller::controller_message type, int strip_nr, float value);
	void send_data(std::vector<std::string> strings);
	void update_display(const struct strip_feedback *strips);
	void update_display(const struct send *sends);
	void update_display(const struct plugin_parameter *selected_plugin, const plugin_multiplexer_struct *plugin_multiplexer, int plugin_bank);
	void update_faders(const struct plugin_parameter *selected_plugin, const plugin_multiplexer_struct *plugin_multiplexer, int plugin_bank);
	void update_faders(const struct send *sends);
	void update_faders(const struct strip_feedback *strips);
};

