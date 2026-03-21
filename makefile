build_dir = /home/samuel/ArdourOscController
object_files = $(build_dir)/ArdourSenderReceiver.o  $(build_dir)/MackieSenderReceiverMidi.o $(build_dir)/MackieSenderReceiverUdp.o $(build_dir)/MidiSenderReceiver.o $(build_dir)/PluginMultiplexer.o

all: ArdourSenderReceiver MackieSenderReceiverMidi MackieSenderReceiverUdp MidiSenderReceiver PluginMultiplexer Startup OscControllerMIDI OscControllerUDP BaseControllerDisplay BaseControllerGuiVersion GpControllerGuiVersion SpControllerGuiVersion PluginRoutingCustomizer

ArdourSenderReceiver: ArdourSenderReceiver.cpp
	g++ -g -c -o $(build_dir)/ArdourSenderReceiver.o ArdourSenderReceiver.cpp -I/home/samuel/Osc-Sender-Receiver

MackieSenderReceiverMidi: MackieSenderReceiver.cpp
	g++ -g -c -o $(build_dir)/MackieSenderReceiverMidi.o MackieSenderReceiver.cpp -I/home/samuel/Osc-Sender-Receiver -lasound -DMACKIE_CONTROL_MIDI_VERSION=1

MackieSenderReceiverUdp: MackieSenderReceiver.cpp
	g++ -g -c -o $(build_dir)/MackieSenderReceiverUdp.o MackieSenderReceiver.cpp -I/home/samuel/Osc-Sender-Receiver -lasound -DMACKIE_CONTROL_UDP_VERSION=1

MidiSenderReceiver: MidiSenderReceiver.cpp
	g++ -g -c -o $(build_dir)/MidiSenderReceiver.o MidiSenderReceiver.cpp -I/home/samuel/Osc-Sender-Receiver -lasound

PluginMultiplexer: PluginMultiplexer.cpp
	g++ -g -c -o $(build_dir)/PluginMultiplexer.o $(build_dir)/MidiSenderReceiver.o PluginMultiplexer.cpp -I/home/samuel/Osc-Sender-Receiver

startup_source = ardour_controller_startup/ardour_controller_startup

Startup: $(startup_source)/main.cpp
	g++ -g -o $(build_dir)/startup $(startup_source)/main.cpp -I/home/samuel/Osc-Sender-Receiver -I/ardour_controller_startup/ardour_controller_startup `wx-config --cxxflags` `wx-config --libs` -std=c++17 -lasound

osc_controller_source = base_controller/visual_studio

OscControllerMIDI: $(osc_controller_source)/display.cpp $(osc_controller_source)/MackieControl.cpp $(osc_controller_source)/OscController.cpp $(osc_controller_source)/main.cpp
	g++ -g -o $(build_dir)/osc_controller_MIDI $(build_dir)/MidiSenderReceiver.o $(build_dir)/PluginMultiplexer.o $(build_dir)/ArdourSenderReceiver.o $(build_dir)/MackieSenderReceiverMidi.o $(osc_controller_source)/display.cpp $(osc_controller_source)/MackieControl.cpp $(osc_controller_source)/OscController.cpp $(osc_controller_source)/main.cpp -g -L/home/samuel/Osc-Sender-Receiver -losc   -I/home/samuel/Osc-Sender-Receiver -I/home/samuel/ardour_controller_lw -I/home/samuel/ardour_controller_lw/base_controller/visual_studio  -std=c++17 -lasound -DMACKIE_CONTROL_MIDI_VERSION=1

OscControllerUDP: $(osc_controller_source)/display.cpp $(osc_controller_source)/MackieControl.cpp $(osc_controller_source)/OscController.cpp $(osc_controller_source)/main.cpp
	g++ -o $(build_dir)/osc_controller_UDP $(build_dir)/MidiSenderReceiver.o $(build_dir)/PluginMultiplexer.o $(build_dir)/ArdourSenderReceiver.o $(build_dir)/MackieSenderReceiverUdp.o $(osc_controller_source)/display.cpp $(osc_controller_source)/MackieControl.cpp $(osc_controller_source)/OscController.cpp $(osc_controller_source)/main.cpp -g -L/home/samuel/Osc-Sender-Receiver -losc   -I/home/samuel/Osc-Sender-Receiver -I/home/samuel/ardour_controller_lw -I/home/samuel/ardour_controller_lw/base_controller/visual_studio  -std=c++17 -lasound -DMACKIE_CONTROL_UDP_VERSION=1

display_source = display/mackie_display

BaseControllerDisplay: $(display_source)/main.cpp
	g++ -g -o $(build_dir)/base_controller_display $(display_source)/main.cpp   `wx-config --cxxflags` `wx-config --libs` -L/home/samuel/Osc-Sender-Receiver -losc   -I/home/samuel/Osc-Sender-Receiver -I/home/samuel/ardour_controller_lw -I/home/samuel/ardour_controller_lw/base_controller/visual_studio  -std=c++17

gui_controller_source = gui_controller/gui_controller

BaseControllerGuiVersion: $(gui_controller_source)/main.cpp
	g++ $(gui_controller_source)/main.cpp -g -o $(build_dir)/base_controller_gui_version $(build_dir)/MackieSenderReceiverMidi.o `wx-config --cxxflags` `wx-config --libs` -L/home/samuel/Osc-Sender-Receiver -losc -I/home/samuel/Osc-Sender-Receiver -I/home/samuel/ardour_controller_lw -I/home/samuel/ardour_controller_lw/base_controller/visual_studio  -std=c++17

gp_controller_gui_version_source = gui_gp_controller/gui_gp_controller

GpControllerGuiVersion: $(gp_controller_gui_version_source)/main.cpp
	g++ $(gp_controller_gui_version_source)/main.cpp $(build_dir)/PluginMultiplexer.o -g -o $(build_dir)/gp_controller_gui_version `wx-config --cxxflags` `wx-config --libs` -L/home/samuel/Osc-Sender-Receiver -losc   -I/home/samuel/Osc-Sender-Receiver -I/home/samuel/ardour_controller_lw -I/home/samuel/ardour_controller_lw/base_controller/visual_studio  -std=c++17

sp_controller_gui_version_source = gui_sp_controller/gui_sp_controller

SpControllerGuiVersion: $(sp_controller_gui_version_source)/main.cpp
	g++ $(sp_controller_gui_version_source)/main.cpp -g $(build_dir)/PluginMultiplexer.o -o $(build_dir)/sp_controller_gui_version `wx-config --cxxflags` `wx-config --libs` -L/home/samuel/Osc-Sender-Receiver -losc   -I/home/samuel/Osc-Sender-Receiver -I/home/samuel/ardour_controller_lw -I/home/samuel/ardour_controller_lw/base_controller/visual_studio  -std=c++17

plugin_routing_customizer_source = plugin_routing_customizer/plugin_router

PluginRoutingCustomizer: $(plugin_routing_customizer_source)/main.cpp
	g++ $(plugin_routing_customizer_source)/main.cpp -g -o $(build_dir)/plugin_routing_customizer `wx-config --cxxflags` `wx-config --libs` -L/home/samuel/Osc-Sender-Receiver -losc   -I/home/samuel/Osc-Sender-Receiver -I/home/samuel/ardour_controller_lw -I/home/samuel/ardour_controller_lw/base_controller/visual_studio  -std=c++17

CopyPluginData:
	cp -r plugin_data  $(build_dir)/plugin_data