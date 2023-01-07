#ifndef URP_FRAME_PROCESSOR_H
#define URP_FRAME_PROCESSOR_H

#include <iostream>
#include <cmath>
#include <list>
#include <string>
#include <memory>
#include <thread>
#include <mutex>
#include <fstream>
#include <vector>
#include <unordered_set>
#include <set>
#include <type_traits>
#include <atomic>

#include "udp.h"

#include "Urp.h"

class FrameProcessorSimple
{
public:
    FrameProcessorSimple(const std::string_view p_interface_address, const std::string_view p_destination_address)
    {
        interface_address = p_interface_address;
        destination_address = p_destination_address;
        sender = std::make_unique<udp_client>(interface_address, 8888);
        receiver = std::make_unique<udp_server>(8889);
        termimnate = false;
        auto self = this;
        recv_thread = std::thread([&self]()
                                  { self->RecvDataThread(); });
    }

    bool Connect()
    {
        if (!sender)
            return false;
        while (!IsStart())
        {
            Start();
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
        return true;
    }

    void Close()
    {
        while (!IsEnd())
            End();

        termimnate = true;
    }

    ~FrameProcessorSimple()
    {
        recv_thread.join();
    }

    void AddData(DataFrame &data)
    {
        ++count;
        data.hdr.number = count;
        data.hdr.magic = magic_packet;
        std::lock_guard<std::mutex> lock(dloker);
        data_queue.push_back(data);
        unsended.insert(count);
    }

    void ClearData()
    {
        data_queue.clear();
    }
    bool
    SendData()
    {
        size_t pn = 0;
        const uint64_t one = 1;
        bool empty = false;
        size_t check_period = data_queue.size() / 10;
        size_t counter = 0;

        // first - send all data
        for (auto it = data_queue.begin(); it != data_queue.end(); ++it)
        {
            SendPacket(&(*it));
        }

        // second - resend all unsended data
        for (;;)
        {
            std::unordered_set<uint32_t> last;
            {
                std::lock_guard<std::mutex> lock(dloker);
                last = unsended;
            };

            if (last.empty())
                break;

            for (const auto &it : last)
            {
                DataFrame df;
                df.hdr.number = it;

                // if (v != data_queue.end())
                {
                    SendPacket(&data_queue[it - 1]);
                }
            }
        }

        return data_queue.empty();
    }

    void RecvDataThread()
    {
        DataFrame status;
        status.hdr.magic = 0;
        for (; !termimnate;)
        {
            auto res = receiver->recv(reinterpret_cast<char *>(&status), sizeof(status));
            // std::cout<<std::hex<<status.magic<<"\n";
            if (res == sizeof(status.hdr.magic))
            {
                std::lock_guard<std::mutex> lock(dloker);
                unsended.erase(status.hdr.magic);
            }
            if (res == sizeof(status.hdr.magic) + sizeof(status.hdr.number))
            {
                std::lock_guard<std::mutex> lock(dloker);
                this->start = this->start || status.hdr.magic == magic_start;
                std::cout << std::hex << "[ " << status.hdr.magic << "\n";
            }
            else if (res == sizeof(status))
            {
                DataFrame start;
                start.hdr.magic = magic_end;
                start.hdr.size = 0xffffff;
                sender->send(reinterpret_cast<const char *>(&start), sizeof(status));
                std::lock_guard<std::mutex> lock(dloker);
                this->end = status.hdr.magic == magic_end || status.hdr.magic == magic_exit;
                this->start = this->start || status.hdr.magic == magic_start;

                std::cout << std::hex << "] " << status.hdr.magic << "\n";
            }
        }
    }

    void ClearDeliveryStatus()
    {
        std::lock_guard<std::mutex> lock(dloker);

        count = 0;
        start = false;
        end = false;
        data_queue.clear();
    }

    void SendPacket(const DataFrame *d)
    {
        sender->send(reinterpret_cast<const char *>(d), d->hdr.size + sizeof(Header));
    }

private:
    bool IsEnd() const
    {
        std::lock_guard<std::mutex> lock(dloker);
        return end;
    }

    void Start()
    {
        DataFrame start;
        start.hdr.magic = magic_start;
        strcpy(start.data, interface_address.c_str());
        start.hdr.size = 0xffffff;
        sender->send(reinterpret_cast<const char *>(&start), sizeof(Header) + interface_address.length() + 1);
    }

    bool IsStart() const
    {
        std::lock_guard<std::mutex> lock(dloker);
        return start;
    }

    void End()
    {
        DataFrame start;
        start.hdr.magic = magic_end;
        start.hdr.size = 0xffffff;
        sender->send(reinterpret_cast<const char *>(&start), sizeof(Header));
    }

    std::vector<DataFrame> data_queue;
    std::string interface_address;
    std::string destination_address;
    int count = 0;
    std::unique_ptr<udp_client> sender;
    std::unique_ptr<udp_server> receiver;
    std::unordered_set<uint32_t> unsended;
    mutable std::mutex dloker;
    std::thread recv_thread;
    std::atomic_bool termimnate;
    bool start = false;
    bool end = false;
};

#endif