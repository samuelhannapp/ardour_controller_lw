all: OscControllerMIDI OscControllerUDP

OscControllerMIDI: display.cpp MackieControl.cpp OscController.cpp main.cpp
	g++ display.cpp MackieControl.cpp OscController.cpp main.cpp -Wall -g -o osc_controller_midi.out -L/home/samuel/ardour_controller_lw -losc_controller_midi  -L/home/samuel/Osc-Sender-Receiver -losc   -I/home/samuel/Osc-Sender-Receiver -I/home/samuel/ardour_controller_lw -I/home/samuel/ardour_controller_lw/base_controller/visual_studio  -std=c++17 -lasound -DMACKIE_CONTROL_MIDI_VERSION=1

OscControllerUDP: display.cpp MackieControl.cpp OscController.cpp main.cpp
	g++ display.cpp MackieControl.cpp OscController.cpp main.cpp -Wall -g -o osc_controller_udp.out -L/home/samuel/ardour_controller_lw -losc_controller_udp  -L/home/samuel/Osc-Sender-Receiver -losc   -I/home/samuel/Osc-Sender-Receiver -I/home/samuel/ardour_controller_lw -I/home/samuel/ardour_controller_lw/base_controller/visual_studio  -std=c++17 -lasound -DMACKIE_CONTROL_UDP_VERSION=1

