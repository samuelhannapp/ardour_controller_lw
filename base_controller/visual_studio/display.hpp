#pragma once
#include "OscSenderReceiver.hpp"
class display : public OscSenderReceiver
{
	using OscSenderReceiver::OscSenderReceiver;
public:
	void update_display(std::vector<std::string> data);
};

