#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <cstdint>

#define OSC_MINIMUM_PACKET_BYTES  8

struct OscMessage {
public:
	OscMessage(const std::string& address);
	OscMessage(char* buffer, int buffer_length);
	~OscMessage();

	static uint64_t GetAlignedStringLength(const std::string& string);
	static uint64_t GetAlignedStringLength(const std::wstring& string);
	static bool IsLittleEndian();
	static uint32_t SwapInt32(uint32_t num);
	static uint64_t SwapInt64(uint64_t num);
	static double SwapFloat64(double num);
	static float SwapFloat32(float num);

	OscMessage PushString(std::string data);
	OscMessage PushFloat(float data);
	OscMessage PushDouble(double data);
	OscMessage PushInt(int data);
	OscMessage PushLongLong(long long data);

	inline const std::string& GetAddress() const {
		return m_address;
	}
	inline const std::string& GetTypeList() const{
		return m_type;
	}
	inline const std::vector<char>& GetData() const{
		return m_data;
	}

	int get_int(int where);
	float get_float(int where);
	double get_double(int where);
	std::string get_string(int where);
	std::string initialize_type_list(){return this->m_type;}
	char* GetBytes(int& size);

private:
	std::vector<char> initialize_data(char* buffer, int buffer_length);
	std::string get_type_list(char* buffer, int buffer_length);

	int get_data_start_point();
	int get_argument_start_point(int where);
	int get_string_length(int where);

	bool m_readonly;
	std::string m_address;
	std::string m_type;
	std::vector<char> m_data;
};
