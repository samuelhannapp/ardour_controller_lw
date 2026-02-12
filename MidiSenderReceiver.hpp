#pragma once

#ifdef _WIN64
#include <SDKDDKVer.h>
#define _WINSOCKAPI_
#include <Windows.h>
#include <conio.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")
#endif

#ifdef __linux__
#include <alsa/asoundlib.h>
#endif

#include "Defines.hpp"

class MidiSenderReceiver {
	protected:
	#ifdef __linux__
	snd_rawmidi_t* MidiDeviceIn;
	snd_rawmidi_t* MidiDeviceOut;
	#endif
	#ifdef _WIN64
	HMIDIIN MidiDeviceIn;
	HMIDIOUT MidiDeviceOut;
	#endif
public:
	MidiSenderReceiver(int port_in, int port_out);
	int initialize_midi(int port_in, int port_out);
	void receive_data(MidiMessage& message);
	//I changed this function from void send_data(char *data, int size) to this
	//because it was identical to the function of the UdpSenderReceiver, because 
	//for the udp version it means raw udp data, and for the midi version it means 
	//raw midi data, and I don't want to know the UdpSenderReceiver about Midi, 
	//I don't want the MidiSenderReceiver want to know about OscMessage, 
	//and so forth..., because of this, there is now this function, 
	//and if it's the midi version, it'll use direcly the function from the 
	//parent, MidiSenderReceiver, and in case the udp version is used, 
	//it will use this function wich is redirectin an OscMessage to the 
	//OscSenderReceiver 
	virtual void send_data(struct MidiMessage message);

};
