#include <iostream>

#include "FrameProcessor.hpp"

std::string addr = "localhost";
std::string self_addr = "127.0.0.1";

// std::string addr ="192.168.2.73";
// std::string self_addr ="192.168.2.10";

int main(int, char **)
{

    std::cout << sizeof(Header) << " \n";
    FrameProcessorSimple fp(self_addr, addr);

    DataFrame df;
    df.hdr.size = 1400;

    /*std::ofstream tfl("./testb.txt");
     for(size_t i=0;i<10*2*10*650000;++i)
     {
        tfl << std::to_string(i)<<" ";
        if(i% 10 ==0) tfl << "\n";
     }
     tfl.close();
     */

    std::ifstream file("./testb.txt", std::ios::binary);

    // std::thread([&fp]()
    //           { fp.RecvDataThread(); })
    //   .detach();

    bool last = false;

#if 0
    while(!fp.IsStart())
    {
        fp.Start();
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
       

    for(;!last;)
    {
       
       for(size_t i=0;i<200000;++i)
       {
         df.size = file.read(df.data,df.size).gcount();
         if (!df.size) 
         {
            last = true;
            break;
         }
         fp.AddData(df);
        }
        
       fp.SendData();

       while(last && !fp.IsEnd())
       fp.End();
    }

#else

    for (size_t i = 0; true; ++i)
    {
        df.hdr.size = file.read(df.data, df.hdr.size).gcount();
        if (!df.hdr.size)
        {
            last = true;
            break;
        }
        fp.AddData(df);
    }

    if (!fp.Connect())
        return 0;

    // for(;!last;)
    {

        fp.SendData();

        if (last)
        {
            fp.Close();
        }
    }
#endif

#if 0
    FrameProcessor fp;
    fp.SetCapacity(64);
    DataFrame df;
    df.size =1400;

    std::ofstream tfl("./testb.txt");
     for(size_t i=0;i<64000;++i)
     {
        tfl << std::to_string(i)<<" ";
        if(i% 10 ==0) tfl << "\n";
     }
     tfl.close();


    std::ifstream file("./testb.txt",std::ios::binary);
    
    std::thread([&fp]()
    {
        fp.RecvDataThread();
    }).detach();

    bool last = false;
    for(;!last;)
    {
       for(size_t i=0;i<64;++i)
       {
         df.size = file.read(df.data,df.size).gcount();
         if (!df.size) 
         {
            fp.SetCapacity(i);
            last = true;
            break;
         }
         fp.AddData(df);
       }





        while(!fp.IsStarted())
        fp.Begin();

        
        while(!fp.SendData()) ;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        fp.ClearDeliveryStatus();

       while(last && !fp.IsEnd())
       fp.End();
    }

#endif
    std::cout << "Hello, world!\n";
}