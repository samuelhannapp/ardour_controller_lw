libosc_controller_midi: ArdourSenderReceiver.o MackieSenderReceiverMidi.o MidiSenderReceiver.o PluginMultiplexer.o 
	ar rcs libosc_controller_midi.a ArdourSenderReceiver.o MackieSenderReceiverMidi.o MidiSenderReceiver.o PluginMultiplexer.o

libosc_controller_udp: ArdourSenderReceiver.o MackieSenderReceiverMidi.o MidiSenderReceiver.o PluginMultiplexer.o 
	ar rcs libosc_controller_udp.a ArdourSenderReceiver.o MackieSenderReceiverUdp.o MidiSenderReceiver.o PluginMultiplexer.o

ArdourSenderReceiver.o: ArdourSenderReceiver.cpp
	g++ -g -c ArdourSenderReceiver.cpp -I/home/samuel/Osc-Sender-Receiver

MackieSenderReceiverMidi.o: MackieSenderReceiver.cpp
	g++ -g -c -o MackieSenderReceiverMidi.o MackieSenderReceiver.cpp -I/home/samuel/Osc-Sender-Receiver -lasound -DMACKIE_CONTROL_MIDI_VERSION=1

MackieSenderReceiverUdp.o: MackieSenderReceiver.cpp
	g++ -g -c -o MackieSenderReceiverUdp.o MackieSenderReceiver.cpp -I/home/samuel/Osc-Sender-Receiver -lasound -DMACKIE_CONTROL_UDP_VERSION=1

MidiSenderReceiver.o: MidiSenderReceiver.cpp
	g++ -g -c MidiSenderReceiver.cpp -I/home/samuel/Osc-Sender-Receiver -lasound

PluginMultiplexer.o: PluginMultiplexer.cpp
	g++ -g -c PluginMultiplexer.cpp -I/home/samuel/Osc-Sender-Receiver
