#ifndef URP_FRAME_RECEIVER_H
#define URP_FRAME_RECEIVER_H

#include <cmath>
#include <forward_list>
#include <fstream>
#include <iostream>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <thread>
#include <vector>

#include "Urp.h"
#include "udp.h"

using PutDataFunc = std::function<void(const char *data, size_t size)>;

class FrameReceiverSimple
{
public:
    FrameReceiverSimple(PutDataFunc pf)
    {
        receiver = std::make_unique<udp_server>(8888);
        file.open("./text", std::ios::binary);
        put_data = pf;
    }
    ~FrameReceiverSimple() = default;
    bool IsEnd() { return true; }

    void WriteData()
    {
        std::cout << " w d " << data_of_session.size() << "\n";

        for (const auto &df : vdata)
        {
            std::cout << "wr___number " << df.hdr.number << "\n";
            file.write(df.data, df.hdr.size);
        }
        data_of_session.clear();
    }
    void SendStatus()
    {
        DataFrame df;
        std::vector<uint32_t> statuses(10000000);
        for (; true;)
        {

            auto res = receiver->recv(reinterpret_cast<char *>(&df), sizeof(df));
            if (df.hdr.magic == magic_end)
            {
                sender->send(reinterpret_cast<char *>(&df), sizeof(df));
                if (!finish)
                    WriteData();
                if (finish)
                    break;
                finish = true;
            }
            else if (df.hdr.magic == magic_exit)
            {
                sender->send(reinterpret_cast<char *>(&df), sizeof(df));

                break;
            }
            else if (df.hdr.magic == magic_start)
            {
                std::cout << " start " << df.data << "\n";
                if (!sender)
                    sender = std::make_unique<udp_client>(df.data, 8889);

                sender->send(reinterpret_cast<char *>(&df),
                             sizeof(df.hdr.magic) + sizeof(df.hdr.number));
            }
            else
            {
                data_of_session.insert(df);
                auto d = data_of_session.begin();

                DataFrame dfnd;
                dfnd.hdr.number = last_num + 1;

                for (
                    auto res = data_of_session.find(dfnd);
                    res != data_of_session.end();
                    res = data_of_session.find(dfnd))
                {
                    vdata.push_back(*res);
                    put_data(res->data, res->hdr.size);

                    data_of_session.erase(res);

                    last_num++;
                    dfnd.hdr.number = last_num + 1;
                }

                if (sender)
                    sender->send(reinterpret_cast<char *>(&df.hdr.number),
                                 sizeof(df.hdr.number));
            }
        }
    }

    bool IsFinish() { return finish; }

    std::ofstream file;
    std::unique_ptr<udp_client> sender;
    std::unique_ptr<udp_server> receiver;
    std::set<DataFrame> data_of_session;
    std::vector<DataFrame> vdata;
    bool finish = false;
    PutDataFunc put_data;
    uint32_t last_num = 0;
};

#endif // URP_FRAME_RECEIVER_H