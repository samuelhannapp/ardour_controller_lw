#include "display.hpp"
#include "OscMessage.hpp"
#include <ranges>

void display::update_display(std::vector<std::string> data)
{
	for (int i = 0; i < data.size(); i++) {
		OscMessage message("/select/plugin/parameter/name");
		message.PushInt(i + 1);
		message.PushString(data[i]);
		OscSenderReceiver::send_data(message);
	}
}
