#pragma once

struct mackie_display_struct{
	uint8_t MIDI_TX_SYSX_Buffer[SYSX_BUFFER_SIZE];
	std::vector<std::string> mackie_display_formated; //this maybe should be rather copied or used as reference..., I don't know...
	void fill_sysx_buffer();
};
