#ifndef EXAMPLES_H
#define EXAMPLES_H

#include <iostream>
#include <fstream>
#include <filesystem>
#include <thread>
#include <vector>

#ifdef __linux__
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <errno.h>
#include <alsa/asoundlib.h>
#endif

#ifdef _WIN64
// Winsock API
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <ws2def.h>
#pragma comment(lib, "Ws2_32.lib")

#include <SDKDDKVer.h>
#define _WINSOCKAPI_
#include <Windows.h>
#include <conio.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")
#endif

#ifdef __STM32F7xx_HAL_H
#include "stm32_sock.h"
#endif

#include "UdpSenderReceiver.hpp"
//#include "MackieSenderReceiver.hpp"
#include "PluginMultiplexer.hpp"
#include "ArdourSenderReceiver.hpp"
#include "OscSenderReceiver.hpp"
#include "MackieControl.hpp"

#include "OscMessage.hpp"
#include "OscSenderReceiver.hpp"




#define PLUGIN_PAGES_SIZE 4

#define PLUGIN_ARRAY_SIZE (MAX_PLUGIN_PARAMETERS + ONE_BASED)

class OscController{
public:
	OscController(std::string destination_ip_address, unsigned int udp_port_in, unsigned int udp_port_out, unsigned int midi_port_in, unsigned int midi_port_out);

	void process_midi(MidiMessage message);
	void mackie_receive_thread();
	void ardour_receive_thread();

	MackieControl *mackie_sender_receiver;
	ArdourSenderReceiver ardour_sender_receiver;
	OscSenderReceiver display;
	struct plugin_multiplexer_struct plugin_multiplexer;	
	volatile enum channel_mode mode = PanMode;
	void switch_channel_mode();
	void switch_channel_mode_without_updating_mackie(channel_mode input);
	
	struct ardour_feedback_struct local_strip_data;
};
#endif
