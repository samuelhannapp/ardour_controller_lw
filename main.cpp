#include "osc_controller.hpp"

int main()
{
    //use ip address "127.0.0.1" if the osc controller runs on the same 
    //computer as ardour
    // 
    //use the ip address of the computer on wich ardour runs
    //in case the ardour controller run's on another computer
    //for example "192.168.10.226"
    //osc_controller controller_1("ip address of the computer where ardour runs", 9, 3819, 2, 2);
    

    #ifdef __linux__
    //for Linux:
    //for midi input and output id put "amidi -l" into terminal
    //number for input and output is usually the same
    osc_controller controller_1("127.0.0.1", 9, 3819, 2, 2);
	#endif
    #ifdef _WIN64
    //for Windows:
    //use tool like MIDI-OX to find device id's for input and output of midi
    //normally it's ascending, meaning for example, input is 1, output is 2
    osc_controller controller_1("127.0.0.1", 9, 3819, 1, 2);
	#endif

    while(1){
        #ifdef __linux__
        sleep(1000);
        #endif
        #ifdef _WIN64
        Sleep(1000);
        #endif
    }

    return 0;
}