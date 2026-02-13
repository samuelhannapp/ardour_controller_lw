#include "display.hpp"
#include "OscMessage.hpp"
#include <ranges>
#include "Defines.hpp"

void display::update_display(enum channel_mode mode)
{
	OscMessage message("/base_controller/mode_switch");
	message.PushInt(mode);
	this->send_data(message);
}

void display::update_display(std::vector<std::string> data)
{
for (int i = 0; i < data.size(); i++) {
	OscMessage message("/select/plugin/parameter/name");
	message.PushInt(i + 1);
	message.PushString(data[i]);
	OscSenderReceiver::send_data(message);
}
}
