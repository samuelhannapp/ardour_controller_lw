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

//maybe we rather just give a string as command...
void display::update_display(OscMessage message, std::vector<std::string> data)
{
for (int i = 0; i < data.size(); i++) {
	OscMessage temp_message(message.GetAddress());
	temp_message.PushInt(i + 1);
	temp_message.PushString(data[i]);
	OscSenderReceiver::send_data(temp_message);
}
}
