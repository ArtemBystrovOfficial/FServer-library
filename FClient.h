#pragma once

#pragma comment(lib, "ws2_32.lib")
#include <winsock2.h>
#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>
#include <deque>
#include <fstream>
#include <filesystem>

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma warning(disable: 4996)

using namespace std::chrono_literals;

typedef long long ll;

template <typename _Struct>
class Client
{
public:

    //+-+-+-+-+-+-+-+-++-+-+-+-+-+-+-+-++-+-+-+-+-+-+-+-+
    //+-+-+-+-+-+-+-+-+| Structures |+-+-+-+-+-+-+-+-+-+-
    //+-+-+-+-+-+-+-+-++-+-+-+-+-+-+-+-++-+-+-+-+-+-+-+-+

    struct info_pack_signal // struct for files buffer
    {

        std::string name;
        size_t size_bytes;

    };
private:
    //+-+-+-+-+-+-+-+-++-+-+-+-+-+-+-+-++-+-+-+-+-+-+-+-+
    //+-+-+-+-+-+-+-+-+| Varibales |+-+-+-+-+-+-+-+-+-+-+
    //+-+-+-+-+-+-+-+-++-+-+-+-+-+-+-+-++-+-+-+-+-+-+-+-+

    std::atomic <bool> is_disconected{ 0 };

    std::string ip;
    int port;

    std::string path = "";
    std::atomic < bool > is_path_ready{ false };

    WSAData wsaData;
    WORD DLLVersion;
    SOCKADDR_IN addr;
    int sizeofaddr;
    SOCKET Connection;

    std::mutex event_in;
    std::mutex event_out;

    HANDLE not_buf_in_empty;
    HANDLE not_buf_out_empty;

    std::mutex block_buffer_in;
    std::mutex block_buffer_out;
    std::mutex block_bufeer_in_files;

    std::thread Send;
    std::thread Read;
    std::vector<std::thread> timed_for_files;

    std::deque <_Struct > buf_in;
    std::deque <_Struct > buf_out;
    std::deque <info_pack_signal> buf_in_files;

    std::atomic<bool> Exit{ 0 };
    std::atomic <bool> is_files_sends{ 0 };
    // main commands between client and server
    enum class Commands
    {
        CLOSE,
        STRUCT,
        FILE
    };

    //+-+-+-+-+-+-+-+-++-+-+-+-+-+-+-+-++-+-+-+-+-+-+-+-+-+-+-+
    //+-+-+-+-+-+-+-+-+| Private Methods |+-+-+-+-+-+-+-+-+-+-+
    //+-+-+-+-+-+-+-+-++-+-+-+-+-+-+-+-++-+-+-+-+-+-+-+-+-+-+-+

    // exit signal for read or send thread with one connection
    void ext();

    //main thread for read from another client
    void _Read_th();

    //main thread for send to another client
    void _Send_th();

    //main thread for send files
    void _send_file(std::string file_name);
    
public:
    //+-+-+-+-+-+-+-+-++-+-+-+-+-+-+-+-++-+-+-+-+-+-+-+-+
    //+-+-+-+-+-+-+-+-+| Operators |+-+-+-+-+-+-+-+-+-+-+
    //+-+-+-+-+-+-+-+-++-+-+-+-+-+-+-+-++-+-+-+-+-+-+-+-+

    template<typename _Struct>
    friend Client <_Struct>& operator>> (Client <_Struct>&, _Struct&);

    template<typename _Struct>
    friend Client <_Struct>& operator<< (Client <_Struct>&, _Struct&&);

    template<typename _Struct>
    friend Client <_Struct>& operator<< (Client <_Struct>&, const _Struct&);

    template<typename _Struct>
    friend Client <_Struct>& operator<<(Client<_Struct>&, std::string);

    template<typename _Struct>
    friend Client <_Struct>& operator>>(Client<_Struct>&, typename Client<_Struct>::info_pack_signal&);

    //+-+-+-+-+-+-+-+-++-+-+-+-+-+-+-+-++-+-+-+-+-+-+-+-+
    //+-+-+-+-+-+-+-+-+| public Methods |+-+-+-+-+-+-+-+-
    //+-+-+-+-+-+-+-+-++-+-+-+-+-+-+-+-++-+-+-+-+-+-+-+-+

    //parsing name of file
    static void to_name(std::string& file_name)
    {
        std::string data="";
        auto it1 = std::find(file_name.rbegin(), file_name.rend(), '/');
        if (it1 != file_name.rend())
        {
            std::copy(file_name.rbegin(), it1, std::back_inserter(data));
        }

        auto it2 = std::find(file_name.rbegin(), file_name.rend(), '\\');
        if (it2 != file_name.rend())
        {
            std::copy(file_name.rbegin(), it1, std::back_inserter(data));
        }
        std::reverse(data.begin(), data.end());
        file_name = data;
    }

    //return true, when buf_in doesn't contain any pocket_message from clients
    bool is_buf_in_empty();

    //return true, when buf_out doesn't contain any pocket_message on server
    bool is_buf_out_empty();

    // the only one template constructor for make Server
    Client(const char* IP, const int& port);

    // use this method in all situations. if you don't use
    // you program can call terminated() or programm and threads
    // will finish not right. You can use RTTI and place method
    // in destructor. Read more about it in documentation, please
    void disconnect();

    // start server
    void connect_to_server();

    //return true, if server have used disconnect() or another critical exit 
    bool is_client_disconected();

    // set path for download and send your files
    // read the documentation please
    void set_path_download(const std::string& s);

    // set path for download and send your files
    // read the documentation please
    void set_path_download(const char* s);

    auto get_last_file()->info_pack_signal;

    // sinhrone waiting for new file
    void wait_file();

    // asinhrone waiting for new file
    bool is_buf_in_files_empty();

    //use disconnect() to avoid big problems with memory
    ~Client();

};



// exit signal for read or send thread with one connection
template<typename _Struct>
void Client<_Struct>::ext()
{
    Exit.store(true);
    is_disconected.store(true);
    event_out.lock();
    SetEvent(not_buf_out_empty);
    event_out.unlock();
    event_in.lock();
    SetEvent(not_buf_in_empty);
    event_in.unlock();
}


//main thread for read from another client
template<typename _Struct>
void Client<_Struct>::_Read_th()
{
    Commands type;
    char msg;

    while (!Exit.load())
    {
        if (recv(Connection, &msg, sizeof(msg), 0) <= 0)
        {
            if (!block_buffer_out.try_lock()) continue;
            std::cout << "disconected Extra from server \n";
            ext();
            block_buffer_out.unlock();
            break;
        }
        else
        {

            type = static_cast<Commands>(msg);

            switch (type)
            {
            case(Commands::CLOSE):
            {
                std::cout << "disconected from server \n";
                ext();
                closesocket(Connection);
                break;
            }
            break;

            case(Commands::STRUCT):
            {
                _Struct data;

                recv(Connection, (char*)&data, sizeof(_Struct), 0);

                block_buffer_in.lock();

                buf_in.push_back(std::move(data));

                event_in.lock();
                SetEvent(not_buf_in_empty);
                event_in.unlock();

                block_buffer_in.unlock();

            }
            break;
            case(Commands::FILE):
            {
                char file_size_str[64];
                char file_name[64];

                block_buffer_in.lock();

                recv(Connection, file_size_str, 64, 0);

                int file_size = atoi(file_size_str);
                char* bytes = new char[file_size];

                recv(Connection, file_name, 64, 0);

                std::string _file_name = path.c_str();

                if (path == "" || !is_path_ready.load())
                    std::cout << "Warning: your path to download directory empty or haven't signed up yet, "
                    "your files in cpp directory\n";

                _file_name += file_name;

                strcpy(file_name, _file_name.c_str());

                std::fstream file;
                file.open(file_name, std::ios_base::out | std::ios_base::binary);

                if (file.is_open())
                {

                    recv(Connection, bytes, file_size, 0);

                    file.write(bytes, file_size);

                    file.close();
                }
                else throw std::exception("Error didn't write");

                block_buffer_in.unlock();

                block_bufeer_in_files.lock();

                buf_in_files.push_back(info_pack_signal{ file_name, size_t(file_size) });

                block_bufeer_in_files.unlock();

                std::cout << "file saved ok " << file_name << " size " << file_size + 128 << " bytes \n";

                delete[] bytes;


            }
            break;

            }
        }
    }
}


//main thread for send to another client
template<typename _Struct>
void Client<_Struct>::_Send_th()
{
    while (!Exit.load())
    {
        if (buf_out.empty())
            WaitForSingleObject(not_buf_out_empty, INFINITE);

        event_out.lock();
        ResetEvent(not_buf_out_empty);
        event_out.unlock();

        if (Exit.load()) break;

        block_buffer_out.lock();

        if (!buf_out.empty())
        {
            auto command = char(Commands::STRUCT);
            send(Connection, &command, sizeof(command), 0);

            auto data = buf_out.front();

            send(Connection, (char*)&data, sizeof(_Struct), 0);

            buf_out.pop_front();

        }
        block_buffer_out.unlock();
    }
}

//main thread for send files
template<typename _Struct>
void Client<_Struct>::_send_file(std::string file_name)
{
    auto file_name_handler = file_name;
    to_name(file_name_handler);

    file_name = path + file_name;

    if (path == "" || !is_path_ready.load())
        std::cout << "Warning: your path to download directory empty or haven't signed up yet, "
        "your files in cpp directory\n";

    std::fstream file;

    file.open(file_name, std::ios_base::in | std::ios_base::binary);

    if (file.is_open())
    {
        int file_size = std::filesystem::file_size(file_name);

        // max-size 50mb
        if (file_size > 52'428'800)
            throw std::exception("So big file");

        char* bytes = new char[file_size];

        file.read((char*)bytes, file_size);

        auto command = char(Commands::FILE);

        block_buffer_out.lock();

        send(Connection, &command, sizeof(command), 0);

        send(Connection, std::to_string(file_size).c_str(), 64, 0);
        send(Connection, file_name_handler.c_str(), 64, 0);
        send(Connection, bytes, file_size, 0);

        std::cout << " sended file: " << file_name << " size " << file_size + 128 << " bytes "
            << " to " << ip << ":" << port << "\n";

        Sleep(10);

        block_buffer_out.unlock();

        delete[] bytes;
        file.close();
    }
    else
    {
        throw std::exception("File didn't open\n");
    }

}

template<typename _Struct>
bool Client<_Struct>::is_buf_in_empty()
{
    block_buffer_in.lock();

    auto is = buf_in.empty();

    block_buffer_in.unlock();

    return is;
}
//return true, when buf_out doesn't contain any pocket_message on server
template<typename _Struct>
bool Client<_Struct>::is_buf_out_empty()
{
    block_buffer_out.lock();

    auto is = buf_out.empty();

    block_buffer_out.unlock();

    return is;

}
// the only one template constructor for make Server
template<typename _Struct>
Client<_Struct>::Client(const char* IP, const int& port) : ip(IP), port(port)
{
    srand(time(0));

    not_buf_in_empty = CreateEvent(NULL, true, false, TEXT("rc" + char(rand() % 256) + char(rand() % 256) + char(rand() % 256)
        + char(rand() % 256) + char(rand() % 256) + char(rand() % 256)));
    not_buf_out_empty = CreateEvent(NULL, true, false, TEXT("sc" + char(rand() % 256) + char(rand() % 256) + char(rand() % 256)
        + char(rand() % 256) + char(rand() % 256) + char(rand() % 256)));

    DLLVersion = MAKEWORD(2, 1);

    if (WSAStartup(DLLVersion, &wsaData) != 0) {
        throw std::exception("Another Dll version\n");
    }

    sizeofaddr = sizeof(addr);
    addr.sin_addr.s_addr = inet_addr(IP);
    addr.sin_port = htons(port);
    addr.sin_family = AF_INET;

    Connection = socket(AF_INET, SOCK_STREAM, NULL);

}

// use this method in all situations. if you don't use
// you program can call terminated() or programm and threads
// will finish not right. You can use RTTI and place method
// in destructor. Read more about it in documentation, please
template<typename _Struct>
void Client<_Struct>::disconnect()
{

    uint16_t time = 0;
    while (!is_buf_out_empty()) {
        if (time > 100)
        {
            std::cout << "WARNING : Buf_out don't send all files, "
                "use is_buf_out_empty() for it\n";
            break;
        }
        time += 1;
        std::this_thread::sleep_for(1ms);
    }

    block_buffer_in.lock();

    block_buffer_out.lock();

    Sleep(10); // extra safe

    Exit.store(true);

    is_disconected.store(true);

    event_out.lock();
    SetEvent(not_buf_out_empty);
    event_out.unlock();

    auto command = char(Commands::CLOSE);

    send(Connection, &command, sizeof(command), 0);

    closesocket(Connection);

    std::cout << "disconected from : " << ip << ":" << port << std::endl;

    block_buffer_out.unlock();

    block_buffer_in.unlock();

    std::this_thread::sleep_for(100ms); //Extra Exit

}

// start server
template<typename _Struct>
void Client<_Struct>::connect_to_server()
{
    if (connect(Connection, (SOCKADDR*)&addr, sizeof(addr)) != 0) {
        throw std::exception("Error: failed connect to server\n");
    }

    std::cout << "connected to: " << ip << ":" << port << "\n";

    Send = std::move(std::thread(&Client::_Send_th, this));
    Read = std::move(std::thread(&Client::_Read_th, this));

    std::this_thread::sleep_for(10ms); //Extra start
}

//return true, if server have used disconnect() or another critical exit 
template<typename _Struct>
bool Client<_Struct>::is_client_disconected()
{
    return is_disconected.load();
}

// set path for download and send your files
// read the documentation please
template<typename _Struct>
void Client<_Struct>::set_path_download(const std::string& s)
{

    path = s;
    is_path_ready.store(true);
}

// set path for download and send your files
// read the documentation please
template<typename _Struct>
void Client<_Struct>::set_path_download(const char* s)
{

    path = s;
    is_path_ready.store(true);
}

template<typename _Struct>
auto Client<_Struct>::get_last_file() -> info_pack_signal
{
    block_bufeer_in_files.lock();

    return buf_in_files.back();

    block_bufeer_in_files.unlock();
}

// sinhrone waiting for new file
template<typename _Struct>
void Client<_Struct>::wait_file()
{
    while (buf_in_files.empty()) std::this_thread::sleep_for(10ms);
}

// asinhrone waiting for new file
template<typename _Struct>
bool Client<_Struct>::is_buf_in_files_empty()
{

    block_bufeer_in_files.lock();
    return buf_in_files.empty();
    block_bufeer_in_files.unlock();
}

//use disconnect() to avoid big problems with memory
template<typename _Struct>
Client<_Struct>::~Client()
{

    Exit.store(true);

    event_out.lock();
    SetEvent(not_buf_out_empty);
    event_out.unlock();

    closesocket(Connection);

    for (int i = 0; i < timed_for_files.size(); i++)
        timed_for_files[i].join();

    Send.join();
    Read.join();

    if (!is_disconected.load())std::cout << "disconected from : " << ip << ":" << port;
}

//operator recv pocket from server
template<typename _Struct>
Client < _Struct >& operator>>(Client  < _Struct >& sv, _Struct& st)
{

    if (sv.buf_in.empty()) {

        WaitForSingleObject(sv.not_buf_in_empty, INFINITE);
        if (sv.is_disconected) return sv;

        sv.event_in.lock();
        ResetEvent(sv.not_buf_in_empty);
        sv.event_in.unlock();


    }

    sv.block_buffer_in.lock();


    if (!sv.buf_in.empty())
    {
        st = std::move(sv.buf_in.front());

        sv.buf_in.pop_front();
    }
    sv.block_buffer_in.unlock();

    return sv;
}

//operator send pocket to server
template<typename _Struct>
Client < _Struct>& operator<<(Client < _Struct >& sv, _Struct&& st)
{

    sv.block_buffer_out.lock();

    sv.buf_out.push_back(std::move(st));

    sv.event_out.lock();
    SetEvent(sv.not_buf_out_empty);
    sv.event_out.unlock();

    sv.block_buffer_out.unlock();

    return sv;
}

//operator send pocket to server
template<typename _Struct>
Client < _Struct>& operator<<(Client < _Struct >& sv, const _Struct& st)
{

    sv.block_buffer_out.lock();

    sv.buf_out.push_back(st);

    sv.event_out.lock();
    SetEvent(sv.not_buf_out_empty);
    sv.event_out.unlock();

    sv.block_buffer_out.unlock();

    return sv;
}

//operator recv files from server
template<typename _Struct>
Client<_Struct>& operator>>(Client<_Struct>& sv, typename Client<_Struct>::info_pack_signal& pack)
{

    sv.block_bufeer_in_files.lock();

    if (sv.buf_in_files.empty())
    {
        std::cout << "Warning files buffer is empty\n";
        sv.block_bufeer_in_files.unlock();
        return sv;
    }

    auto data = sv.buf_in_files.front();

    sv.buf_in_files.pop_front();

    pack = data;

    sv.block_bufeer_in_files.unlock();
}

//operator send file to server
template<typename _Struct>
Client<_Struct>& operator<<(Client<_Struct>& sv, std::string _name_file)
{

    std::thread t([&]()
        {
            sv._send_file(_name_file);
        });
    Sleep(10);

    sv.timed_for_files.push_back(std::move(t));

    return sv;
}