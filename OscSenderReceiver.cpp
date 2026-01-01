#include "OscSenderReceiver.hpp"

void OscSenderReceiver::send_data(OscMessage message)
{
	int size = 0;
	char* data = message.GetBytes(size);
	UdpSenderReceiver::send_data(data, size);
}

OscMessage OscSenderReceiver::receive_data()
{
	char buffer[1024];
	int length = UdpSenderReceiver::receive_data(buffer);
	OscMessage message(buffer, length);
	return message;
}
