#pragma once
#include "UdpSenderReceiver.hpp"
#include "wxOscReceiveThread.h"
class GuiGpController : public UdpSenderReceiver
{
	using UdpSenderReceiver::UdpSenderReceiver;
};

