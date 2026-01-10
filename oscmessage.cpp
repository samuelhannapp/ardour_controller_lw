#include "oscmessage.hpp"
#include <math.h>
#include <string.h>
#include "oscmessage.hpp"
#include "UdpSenderReceiver.hpp"


OscMessage::OscMessage(const std::string& address)
	: m_address(address), m_type(","), m_readonly(false)
{
	//HEKKYOSC_ASSERT(address.length() > 1, "The address is invalid!");
	//HEKKYOSC_ASSERT(address[0] == '/', "The address is invalid! It should start with a '/'!");
	m_data.reserve(OSC_MINIMUM_PACKET_BYTES);
}

OscMessage::OscMessage(char* buffer, int buffer_length)
{
	m_address = std::string(buffer);
	//HEKKYOSC_ASSERT(m_address.length() > 1, "The address is invalid!");
	//HEKKYOSC_ASSERT(m_address.at(0) == '/', "The address is invalid! It should start with a '/'!");
	m_type = get_type_list(buffer, buffer_length);
	m_data = initialize_data(buffer, buffer_length);
	m_readonly = false;

}

OscMessage::~OscMessage() {
	m_data.clear();
}

OscMessage OscMessage::PushString(std::string data) {
	//HEKKYOSC_ASSERT(m_readonly == false, "Cannot write to a message packet once sent to the network! Construct a new message instead.");

	std::copy(data.begin(), data.end(), std::back_inserter(m_data));
	m_data.insert(m_data.end(), GetAlignedStringLength(data) - data.length(), 0);
	m_type += "s";
	return *this;
}

// Aliases
OscMessage OscMessage::PushFloat(float data) {
	if (isinf(data)) {
		m_type += "I";
	}
	else {
		union {
			float f;
			char c[4];
		} primitiveLiteral = { data };

		if (OscMessage::IsLittleEndian()) {
			primitiveLiteral.f = SwapFloat32(data);
		}

		m_data.insert(m_data.end(), primitiveLiteral.c, primitiveLiteral.c + 4);
		m_type += "f";
	}
	return *this;
}
OscMessage OscMessage::PushDouble(double data) {
	if (isinf(data)) {
		m_type += "I";
	}
	else {
		union {
			double d;
			char c[8];
		} primitiveLiteral = { data };

		if (IsLittleEndian()) {
			primitiveLiteral.d = SwapFloat64(data);
		}

		m_data.insert(m_data.end(), primitiveLiteral.c, primitiveLiteral.c + 8);
		m_type += "d";
	}
	return *this;
}
OscMessage OscMessage::PushInt(int data) {
	union {
		int i;
		char c[4];
	} primitiveLiteral = { data };

	if (IsLittleEndian()) {
		primitiveLiteral.i = SwapInt32(data);
	}

	m_data.insert(m_data.end(), primitiveLiteral.c, primitiveLiteral.c + 4);
	m_type += "i";
	return *this;
}
OscMessage OscMessage::PushLongLong(long long data) {
	union {
		long long i;
		char c[8];
	} primitiveLiteral = { data };

	if (IsLittleEndian()) {
		primitiveLiteral.i = SwapInt64(data);
	}

	m_data.insert(m_data.end(), primitiveLiteral.c, primitiveLiteral.c + 8);
	m_type += "h";
	return *this;
}

// Internal function
char* OscMessage::GetBytes(int& size) {
	std::vector<char> headerData;

	// Append address
	std::copy(m_address.begin(), m_address.end(), std::back_inserter(headerData));
	headerData.insert(headerData.end(), GetAlignedStringLength(m_address) - m_address.length(), 0);

	// Append types
	std::copy(m_type.begin(), m_type.end(), std::back_inserter(headerData));
	headerData.insert(headerData.end(), GetAlignedStringLength(m_type) - m_type.length(), 0);

	// Add header to start of data block
	m_data.insert(m_data.begin(), headerData.begin(), headerData.end());

	// Lock this packet
	m_readonly = true;
	size = static_cast<int>(m_data.size());
	return m_data.data();
}

std::string OscMessage::get_type_list(char* buffer, int buffer_length){
	int ctr = 0;
	std::string ret;
	while (ctr++ < buffer_length) {
		if (*buffer++ == ',')
			break;
	}
	while (ctr++ < buffer_length) {
		if (*buffer != '\0')
			ret.push_back(*buffer++);
		else
			break;
	}
	return ret;
}

std::vector<char> OscMessage::initialize_data(char* buffer, int buffer_length){
	std::vector<char> out;
	for(int i = 0; i < buffer_length; i++)
		out.push_back(*(buffer + i));
	return out;
}

int OscMessage::get_data_start_point(){
		//std::string debug_string;
		//bool debug_active = false;
		//int c = 0;
		//for(c = 0; c < this->m_data.size(); c++)
			//debug_string.push_back(this->m_data.at(c));
		//if(c != 0)
			//debug_active = true;

		int i = 0;
		while (this->m_data[i] != ',')
			i++;
		while (this->m_data[i] != '\0')
			i++;
		//we found the closing '\0' after the type string, now lets go one further...
		i++;
		while ((i % 4) != 0)
			i++;
		return i;
}

int OscMessage::get_string_length(int start_point){
	size_t size = this->m_data.size();
	while(this->m_data.at(start_point) != '\0')
		start_point++;
	//we found the closing '\0', now let's go one further
	start_point++;
	//now make it even to 4 bytes
	while (start_point % sizeof(int) != 0)
		start_point++;
	return start_point;
}

int OscMessage::get_argument_start_point(int argument_nr){

	int start_point = this->get_data_start_point();
	for(int i = 0; i < argument_nr; i++)
		switch(this->m_type.at(i)){
		case 'i':
			start_point += 4;
			break;
		case 'f':
			start_point += 4;
			break;
		case 's':
			start_point = this->get_string_length(start_point);
			break;
		case 'd':
			start_point += 8;
			break;
		}
	return start_point;
}

float OscMessage::get_float(int argument_nr){
	int argument_start_point = this->get_argument_start_point(argument_nr);

	unsigned char byte_array[4];
	for (int i = 0; i < 4; i++)
	{
		byte_array[4 - (i + 1)] = this->m_data[argument_start_point + i];
	}

	float ret = 0;
	memcpy(&ret, byte_array, sizeof(float));
	return ret;
}

int OscMessage::get_int(int argument_nr)
{
	int argument_start_point = this->get_argument_start_point(argument_nr);
	int ret = 0;

	for (int i = 0; i < 4; i++) {
		ret |= (((unsigned char)this->m_data[argument_start_point++]) << (24 - (i * 8)));
	}

	return ret;
}


double OscMessage::get_double(int argument_nr){
	int argument_start_point = this->get_argument_start_point(argument_nr);

	unsigned char byte_array[8];
	for (int i = 0; i < 8; i++)
	{
		byte_array[8 - (i + 1)] = this->m_data[argument_start_point + i];
	}

	double val = 0;
	memcpy(&val, byte_array, sizeof(double));
	return val;
}
	std::string OscMessage::get_string(int argument_nr){
		int argument_start_point = this->get_argument_start_point(argument_nr);

		std::string ret;
		while (this->m_data[argument_start_point] != '\0')
			ret.push_back(this->m_data[argument_start_point++]);
		return ret;
	}

uint64_t OscMessage::GetAlignedStringLength(const std::string& string) {
		uint64_t len = string.length() + (4 - string.length() % 4);
		if (len <= string.length()) len += 4;
		return len;
	}
	uint64_t OscMessage::GetAlignedStringLength(const std::wstring& string) {
		uint64_t len = string.length() + (4 - string.length() % 4);
		if (len <= string.length()) len += 4;
		return len;
	}

	bool OscMessage::IsLittleEndian() {
		union {
			uint32_t i;
			char c[4];
		} endianCheck = { 0x01020304 };

		return endianCheck.c[0] != 1;
	}

	uint32_t OscMessage::SwapInt32(uint32_t num) {
		return static_cast<std::uint32_t>((num << 24) | ((num << 8) & 0x00FF0000) | ((num >> 8) & 0x0000FF00) | (num >> 24));
	}

	float OscMessage::SwapFloat32(float num) {

		union {
			float f;
			uint32_t ui32;
		} swapper = { num };

		swapper.ui32 = SwapInt32(swapper.ui32);
		return swapper.f;
	}

	uint64_t OscMessage::SwapInt64(uint64_t num) {
		num = (num & 0x00000000FFFFFFFF) << 32	| (num & 0xFFFFFFFF00000000) >> 32;
		num = (num & 0x0000FFFF0000FFFF) << 16	| (num & 0xFFFF0000FFFF0000) >> 16;
		num = (num & 0x00FF00FF00FF00FF) << 8	| (num & 0xFF00FF00FF00FF00) >> 8;
		return num;
	}

	double OscMessage::SwapFloat64(double num) {

		union {
			double d;
			uint64_t ui64;
		} swapper = { num };

		swapper.ui64 = SwapInt64(swapper.ui64);
		return swapper.d;
	}

