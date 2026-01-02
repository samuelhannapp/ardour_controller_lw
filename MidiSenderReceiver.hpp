#pragma once

#ifdef _WIN64
#include <SDKDDKVer.h>
#define _WINSOCKAPI_
#include <Windows.h>
#include <conio.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")
#endif

class MidiSenderReceiver {
	#ifdef __linux__
	snd_rawmidi_t* MidiDeviceIn;
	snd_rawmidi_t* MidiDeviceOut;
	#endif
	#ifdef _WIN64
	HMIDIIN MidiDeviceIn;
	HMIDIOUT MidiDeviceOut;
	#endif
public:
	int initialize_midi(int port_in, int port_out);
	void receive_data(char *buffer);
	void send_data(unsigned char * message, int size);

};
