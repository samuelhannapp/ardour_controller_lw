#pragma once
#include "OscSenderReceiver.hpp"
#include "Defines.hpp"
class display : public OscSenderReceiver
{
	using OscSenderReceiver::OscSenderReceiver;
public:
	void update_display(std::vector<std::string> data);
	void update_display(enum channel_mode);
};

