#include "MidiSenderReceiver.hpp"
#include <vector>
#include <string>

#ifdef _WIN64
struct midi_message_struct{
	unsigned char data[3];
};

struct midi_input {
	HMIDIIN device;
	std::vector<midi_message_struct> midi_data;
};

std::vector<midi_input> midi_input_buffer;

void CALLBACK MidiInProc(HMIDIIN hMidiIn, UINT wMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2)
{
	struct midi_message_struct message;
	message.data[0] = (dwParam1 & 0xff);
	message.data[1] = (dwParam1 & 0xff00) >> 8;
	message.data[2] = (dwParam1 & 0xff0000) >> 16;
	for(struct midi_input &midi_in : midi_input_buffer)
		if (midi_in.device == hMidiIn)
			midi_in.midi_data.push_back(message);
	return;	
}
#endif

#ifdef __linux__
int MidiSenderReceiver::initialize_midi(int port_in, int port_out)
{
	int status;
	std::string port_number = std::to_string(port_out);//this shoule be single digit for now...
	std::string port_name("hw:2,0,0");
	port_name.replace(3, 1, port_number);
	if ((status = snd_rawmidi_open(NULL, &MidiDeviceOut, port_name.c_str(), SND_RAWMIDI_SYNC)) < 0) {
      printf("Problem opening MIDI output: %s", snd_strerror(status));
	  return 1;
   }
	port_number = std::to_string(port_in);//this shoule be single digit for now...
	std::string port_name_2("hw:2,0,0");
	port_name_2.replace(3, 1, port_number);
	if ((status = snd_rawmidi_open(&MidiDeviceIn, NULL, port_name_2.c_str(), SND_RAWMIDI_SYNC)) < 0) {
      printf("Problem opening MIDI input: %s", snd_strerror(status));
      return 1;
   	}
	return 0;
}
#endif

#ifdef __linux__
MidiSenderReceiver::MidiSenderReceiver(int port_in, int port_out)
{
	int status;
	std::string port_number = std::to_string(port_out);//this shoule be single digit for now...
	std::string port_name("hw:2,0,0");
	port_name.replace(3, 1, port_number);
	if ((status = snd_rawmidi_open(NULL, &MidiDeviceOut, port_name.c_str(), SND_RAWMIDI_SYNC)) < 0) {
      printf("Problem opening MIDI output: %s", snd_strerror(status));
   }
	port_number = std::to_string(port_in);//this shoule be single digit for now...
	std::string port_name_2("hw:2,0,0");
	port_name_2.replace(3, 1, port_number);
	if ((status = snd_rawmidi_open(&MidiDeviceIn, NULL, port_name_2.c_str(), SND_RAWMIDI_SYNC)) < 0) {
      printf("Problem opening MIDI input: %s", snd_strerror(status));
   	}
}
#endif

#ifdef _WIN64
int MidiSenderReceiver::initialize_midi(int port_in, int port_out)
{
	MMRESULT result;
	//MidiSenderReceiver::PrintMidiDevices();//if there are none, later they have to be selected previously in the gui...
	result = midiInOpen(&MidiDeviceIn, port_in, (DWORD_PTR)(void*)MidiInProc, 0, CALLBACK_FUNCTION);  
	struct midi_input temp;
	temp.device = MidiDeviceIn;
	midi_input_buffer.push_back(temp);
	

if (result != MMSYSERR_NOERROR) {
		printf("midiInOpen() failed...rv=%d");
	}
	else {
		midiInStart(MidiDeviceIn);
	}

	result = midiOutOpen(&MidiDeviceOut, port_out, 0, 0, CALLBACK_WINDOW);
	if (result)
		printf("There was an error opening MIDI Mapper!\r\n");

	return 0;
}
#endif
#ifdef _WIN64
MidiSenderReceiver::MidiSenderReceiver(int port_in, int port_out)
{
	MMRESULT result;
	int nMidiDeviceNum = midiInGetNumDevs();

	//MidiSenderReceiver::PrintMidiDevices();//if there are none, later they have to be selected previously in the gui...
	result = midiInOpen(&MidiDeviceIn, port_in, (DWORD_PTR)(void*)MidiInProc, 0, CALLBACK_FUNCTION);  
	printf("result is %d", result);
	struct midi_input temp;
	temp.device = MidiDeviceIn;
	midi_input_buffer.push_back(temp);
	

if (result != MMSYSERR_NOERROR) {
		printf("midiInOpen() failed...rv=%d", result);
	}
	else {
		midiInStart(MidiDeviceIn);
	}


	result = midiOutOpen(&MidiDeviceOut, port_out, 0, 0, CALLBACK_WINDOW);
	if (result)
		printf("There was an error opening MIDI Mapper!\r\n");

}
#endif

#ifdef _WIN64
	void MidiSenderReceiver::receive_data(MidiMessage& message)
	{
		HMIDIIN midi_connection;
		int midi_connection_number = -1;
		for (int i = 0; i < midi_input_buffer.size(); i++)
			if (midi_input_buffer.at(i).device == this->MidiDeviceIn)
				midi_connection_number = i;

		while (midi_input_buffer.at(midi_connection_number).midi_data.size() == 0)
			Sleep(1);
		message.data[0] = midi_input_buffer.at(midi_connection_number).midi_data.back().data[0];
		message.data[1] = midi_input_buffer.at(midi_connection_number).midi_data.back().data[1];
		message.data[2] = midi_input_buffer.at(midi_connection_number).midi_data.back().data[2];
		midi_input_buffer.at(midi_connection_number).midi_data.pop_back();
		return;
	}
#endif



#ifdef __STM32F7xx_HAL_H
void MidiSenderReceiver::receive_data(char *buffer)
{

}
#endif

#ifdef __linux__
	void MidiSenderReceiver::receive_data(MidiMessage& message)
	{
		snd_rawmidi_read(MidiDeviceIn, message.data, 3);
		return;
	}

void MidiSenderReceiver::send_data(struct MidiMessage message)
{
	snd_rawmidi_write(MidiDeviceOut, message.data, message.length);	
}
#endif

#ifdef _WIN64
void MidiSenderReceiver::send_data(struct MidiMessage message)
{
	if (message.length == 3) {
		DWORD msg = 0;
		msg |= message.data[0];
		msg |= message.data[1] << 8;
		msg |= message.data[2] << 16;
		midiOutShortMsg(MidiDeviceOut, msg);
	}
	if (message.length == 120) {
		MIDIHDR     midiHdr;
		HANDLE      hBuffer;
		UINT        err;
		hBuffer = GlobalAlloc(GHND, 120);
		if (hBuffer){
			midiHdr.lpData = (LPSTR)GlobalLock(hBuffer);
			if (midiHdr.lpData)
			{
				midiHdr.dwBufferLength = 120;

				midiHdr.dwFlags = 0;

				err = midiOutPrepareHeader(MidiDeviceOut, &midiHdr, sizeof(MIDIHDR));
				if (!err)
				{
					memcpy(midiHdr.lpData, message.data, 120);

					err = midiOutLongMsg(MidiDeviceOut, &midiHdr, sizeof(MIDIHDR));
					if (err)
					{
						char errMsg[120];

						midiOutGetErrorText(err, (LPWSTR)&errMsg[0], 120);
						printf("Error: %s\r\n", &errMsg[0]);
					}

				while (MIDIERR_STILLPLAYING == midiOutUnprepareHeader(MidiDeviceOut, &midiHdr, sizeof(MIDIHDR)))
				{
					//Sleep(1000);
				}
			}
			GlobalUnlock(hBuffer);
		}
		GlobalFree(hBuffer);
	}
	}
}
#endif

#ifdef __STM32F7xx_HAL_H
void MidiSenderReceiver::send_data(unsigned char * message, int size)
{
	;
}
#endif