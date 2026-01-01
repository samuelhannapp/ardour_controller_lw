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

#include "oscmessage.hpp"
#include "UdpSenderReceiver.hpp"
#include "MackieControl.hpp"
#include "PluginMultiplexer.hpp"
#include "ArdourSenderReceiver.hpp"
#include "OscSenderReceiver.hpp"


enum default_strip_types {
		AudioTracks, MidiTracks, AudioBusses, MidiBusses, VCAs, Master,
		Monitor, FoldbackBusses, Selected, Hidden, Use_Group
};

enum default_feedback {
		StripButtons, StripControls, SSIDasPathExtension, UseHeartBeat,
		MasterSection, PHPasBarBeat, PHPasST, MeteringasFloat,
		MeteringasLEDStrip, SignalPresent, PHPasSamples, PHPasMinutesSeconds,
		PHPasperGuiClock, ExtraSelectOnlyFeedback, UseSlashReply, report_8x8_trigger_grid_status,
		report_mixer_scene_status
};

enum plugin_descriptor_flags{
		ENUMERATION, INTEGER_STEP, LOGARITHMIC, MAX_UNBOUND, MIN_UNBOUND, SAMPLE_RATE_DEPENDENT, TOGGLED, CONTROLLABLE, HIDDEN
};

enum channel_mode{
		PanMode, SendMode, PluginMode, Default
};

#define PLUGIN_PAGES_SIZE 4

#define PLUGIN_ARRAY_SIZE (MAX_PLUGIN_PARAMETERS + ONE_BASED)

class OscController{
public:
	OscController(std::string destination_ip_address, unsigned int udp_port_in, unsigned int udp_port_out, unsigned int midi_port_in, unsigned int midi_port_out);

	void mackie_receive_thread();
	void ardour_receive_thread();

	MackieControl mackie_sender_receiver;
	ArdourSenderReceiver ardour_sender_receiver;
	OscSenderReceiver display;
	struct plugin_multiplexer_struct plugin_multiplexer;	
	volatile enum channel_mode mode = PanMode;
	void switch_channel_mode();
	void switch_channel_mode_without_updating_mackie(channel_mode input);
	
	struct ardour_feedback_struct local_strip_data;
};
#endif
