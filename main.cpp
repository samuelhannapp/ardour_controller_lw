#include "osc_controller.hpp"

int main()
{
    osc_controller controller_1("192.168.10.226", PORT, 3819); 

    while(1){
        sleep(100);
    }

    return 0;
}