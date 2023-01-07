#include <fstream>
#include <iostream>

#include "FrameReceiver.hpp"


int main(int, char **)
{
    std::string addr = "localhost";

    FrameReceiverSimple fr([&](const char  *data, size_t size){

        
    });

    while (!fr.IsFinish())
    {
        fr.SendStatus();
    }

    std::cout << "   Hello, world!\n";
}