#pragma once
#define ONE_BASED 1

namespace controller{
	enum controller_message{
		STRIP_NAME, STRIP_VOLUME, REC_ENABLE, SOLO, MUTE, SELECT, STEREO_POSITION,
		SEND_NAME, SEND_ENABLE, SEND_FADER,
		PLUGIN_PARAMETER_NAME, PLUGIN_PARAMETER_ENABLE, PLUGIN_PARAMETER_VALUE,
		METER, BUTTON, KNOB, KNOB_PUSH, DISPLAY
	};
}

#define SYSX_BUFFER_SIZE 120

#define MACKIE_CONTROL_UDP_VERSION 1 
//#define MACKIE_CONTROL_MIDI_VERSION 1

namespace mackie{
    enum button_type {
        RECORD, SOLO, MUTE, SELECT, KNOB_PUSH, LED_STRIP = 6, FADER_TOUCH = 13
    };
	enum knob_push_assignment {
		show_busses, show_vcas, page_down, page_up, mode, plugin_down, plugin_up, spill
	};
}

struct MidiMessage {
	MidiMessage(unsigned char* midi_data, int data_length) {
		for (int i = 0; i < data_length; i++)
			this->data[i] = midi_data[i];
		this->length = data_length;
	}
	unsigned char data[120];//max 120 for sysx of mackie control...
	int length;
};

#define MAX_14_BIT (16383.0)
