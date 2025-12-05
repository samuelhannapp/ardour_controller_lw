#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <cstdint>

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

	// Explicit Push functions
	OscMessage PushBlob(char* data, size_t size);

	OscMessage PushFloat32(float data);
	OscMessage PushFloat64(double data);
	OscMessage PushInt32(int data);
	OscMessage PushInt64(long long data);

	OscMessage PushBoolean(bool data);

	OscMessage PushString(std::string data);
	OscMessage PushStringRef(const std::string& data);
	OscMessage PushCStyleString(char* data);
	OscMessage PushCStyleStringRef(const char* data);

	OscMessage PushWString(std::wstring data);
	OscMessage PushWStringRef(const std::wstring& data);
	OscMessage PushCStyleWString(wchar_t* data);
	OscMessage PushCStyleWStringRef(const wchar_t* data);

	// Aliases
	OscMessage PushFloat(float data);
	OscMessage PushDouble(double data);
	OscMessage PushInt(int data);
	OscMessage PushLongLong(long long data);
	OscMessage PushBool(bool data);

	// Binary blobs
	OscMessage Push(char* data, size_t size);

	// Floating point number
	OscMessage Push(float data);
	OscMessage Push(double data);

	// Integers
	OscMessage Push(int data);
	OscMessage Push(long long data);

	// ASCII Strings
	OscMessage Push(std::string data);
	OscMessage Push(const std::string& data);
	OscMessage Push(char* data);
	OscMessage Push(const char* data);
	
	// Wide strings
	OscMessage Push(std::wstring data);
	OscMessage Push(const std::wstring& data);
	OscMessage Push(wchar_t* data);
	OscMessage Push(const wchar_t* data);

	template<typename T>
	OscMessage Push(T data) {
		//HEKKYOSC_ASSERT(m_readonly == false, "Cannot write to a message packet once sent to the network! Construct a new message instead.");

		return PushBlob(data, sizeof(data));
	}

	inline const std::string& GetAddress() const {
		return m_address;
	}
	inline const std::string& GetTypeList() const{
		return m_type;
	}
	inline const std::vector<char>& GetData() const{
		return m_data;
	}


	unsigned char get_int(int where);
	float get_float(int where);
	double get_double(int where);
	std::string get_string(int where);
	int get_type_list_size(){return this->m_type.size();}
	std::string get_type_list(){return this->m_type;}
	char* GetBytes(int& size);

private:
	std::vector<char> get_data(char* buffer, int buffer_length);
	std::string get_type_list(char* buffer, int buffer_length);

	int get_data_start_point();
	int get_argument_start_point(int where);
	int get_string_length(int where);


private:
	bool m_readonly;
	std::string m_address;
	std::string m_type;
	std::vector<char> m_data;
};
