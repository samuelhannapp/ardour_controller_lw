#include "osc_controller.hpp"

int main()
{
    //osc_controller controller_1("192.168.10.226", PORT, 3819); 
    osc_controller controller_1("127.0.0.1", PORT, 3819);

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