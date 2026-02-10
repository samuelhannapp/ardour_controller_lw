libosc_controller.a: ArdourSenderReceiver.o MackieSenderReceiver.o MidiSenderReceiver.o PluginMultiplexer.o 
	ar rcs libosc_controller.a ArdourSenderReceiver.o MackieSenderReceiver.o MidiSenderReceiver.o PluginMultiplexer.o

ArdourSenderReceiver.o: ArdourSenderReceiver.cpp
	g++ -g -c ArdourSenderReceiver.cpp -I/home/samuel/Osc-Sender-Receiver

MackieSenderReceiver.o: MackieSenderReceiver.cpp
	g++ -g -c MackieSenderReceiver.cpp -I/home/samuel/Osc-Sender-Receiver -lasound

MidiSenderReceiver.o: MidiSenderReceiver.cpp
	g++ -g -c MidiSenderReceiver.cpp -I/home/samuel/Osc-Sender-Receiver -lasound

PluginMultiplexer.o: PluginMultiplexer.cpp
	g++ -g -c PluginMultiplexer.cpp -I/home/samuel/Osc-Sender-Receiver



