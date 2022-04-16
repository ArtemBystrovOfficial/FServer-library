#pragma once

#pragma comment(lib, "ws2_32.lib")

#include <winsock2.h>
#include <iostream>
#include <string>
#include <thread>
#include <atomic>
#include <mutex>
#include <vector>
#include <deque>
#include <chrono>
#include <map>
#include <algorithm>
#include <fstream>
#include <filesystem>
#include <memory>

#define DEBUG_CONSOLE

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma warning(disable: 4996)

using namespace std::chrono_literals;

typedef long long ll;

template <typename _Struct>
class Server
{
public:
    //+-+-+-+-+-+-+-+-++-+-+-+-+-+-+-+-++-+-+-+-+-+-+-+-+
    //+-+-+-+-+-+-+-+-+| Structures |+-+-+-+-+-+-+-+-+-+-
    //+-+-+-+-+-+-+-+-++-+-+-+-+-+-+-+-++-+-+-+-+-+-+-+-+

    struct Number_of_group // special struct for groups, you can use to_group
    {
        Number_of_group(int&& n) :n(n) {}
        Number_of_group() = delete;
        int n;
    };

    struct info_pack_signal // struct for files buffer
    {

        std::string name;
        int from;
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

    std::mutex event_in;
    std::mutex event_out;

    HANDLE not_buf_in_empty = CreateEvent(NULL, true, false, TEXT("rs"));
    HANDLE not_buf_out_empty = CreateEvent(NULL, true, false, TEXT("ss"));

    std::atomic <int> end_threads{ 0 };

    WSAData wsaData;
    WORD DLLVersion;
    SOCKADDR_IN addr;
    int sizeofaddr;

    std::mutex block_before_open_th;

    std::thread new_connects;

    SOCKET sListen;

    std::deque < std::pair <_Struct, int> >  buf_in;
    std::deque < std::pair <_Struct, int> >  buf_out;
    std::deque <info_pack_signal> buf_in_files;

    std::mutex block_buffer_in;
    std::mutex block_buffer_out;
    std::mutex block_bufeer_in_files;

    // struct for connection with client
    struct Connection
    {

        Connection(Connection&& st) noexcept
        {
            connect = std::move(st.connect);
            Read = std::move(st.Read);
        }

        Connection(SOCKET&& c, std::thread&& r) : connect(std::move(c)),
            Read(std::move(r)) {}
        SOCKET connect;
        std::thread Read;
        std::atomic <bool> is_close{ 0 };
    };

    //struct for groups to send
    struct Group
    {

        Group() {};
        Group(std::vector<int>&& vc) : group(std::move(vc)) {}
        Group(const std::vector<int>& vc) : group(vc) {}
        std::vector <int> group;
    };

    std::map <int, Group> groups;

    std::thread Send;
    std::vector<std::thread> timed_for_files;

    std::atomic<bool> Exit{ 0 };

    std::vector < Connection > Connections;

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
    void ext(std::atomic<bool>& mode);

    //main thread for read from another client
    void _Read_th();

    //main thread for send to another client
    void _Send_th();

    //main thread for send files
    void _send_file(std::string file_name, int _id_person);

    //main thread for find new connections
    void new_connected();

    //parsing name of file
    static void to_name(std::string& file_name)
    {
        std::string data = "";
        auto it1 = std::find(file_name.rbegin(), file_name.rend(), '/');
        if (it1 != file_name.rend())
        {
            std::copy(file_name.rbegin(), it1, std::back_inserter(data));
        }

        auto it2 = std::find(file_name.rbegin(), file_name.rend(), '\\');
        if (it2 != file_name.rend())
        {
            std::copy(file_name.rbegin(), it2, std::back_inserter(data));
        }
        std::reverse(data.begin(), data.end());
        file_name = data;
    }

public:

    //+-+-+-+-+-+-+-+-++-+-+-+-+-+-+-+-++-+-+-+-+-+-+-+-+
    //+-+-+-+-+-+-+-+-+| Operators |+-+-+-+-+-+-+-+-+-+-+
    //+-+-+-+-+-+-+-+-++-+-+-+-+-+-+-+-++-+-+-+-+-+-+-+-+

    template<typename _Struct>
    friend Server<_Struct>& operator>>(Server<_Struct>&, std::pair <_Struct, int >&);

    template<typename _Struct>
    friend Server<_Struct>& operator<<(Server<_Struct>&, std::pair <_Struct, int >&&);

    template<typename _Struct>
    friend Server<_Struct>& operator<<(Server<_Struct>&, const std::pair <_Struct, int >&);

    template<typename _Struct>
    friend Server< _Struct>& operator<<(Server < _Struct >&, const _Struct& st);

    template<typename _Struct>
    friend Server< _Struct>& operator<<(Server < _Struct >&, _Struct&& st);

    template<typename _Struct>
    friend Server<_Struct>& operator<<(Server<_Struct>&, std::pair <_Struct,
        typename Server<_Struct>::Number_of_group >&&);

    template<typename _Struct>
    friend Server<_Struct>& operator<<(Server<_Struct>&, const std::string&);

    template<typename _Struct>
    friend Server<_Struct>& operator<<(Server<_Struct>&, std::pair <std::string, int>);

    template<typename _Struct>
    friend Server<_Struct>& operator<<(Server<_Struct>&, std::pair <std::string,
        typename Server<_Struct>::Number_of_group >&&);

    template<typename _Struct>
    friend Server<_Struct>& operator>>(Server<_Struct>&, typename Server<_Struct>::info_pack_signal&);

    //+-+-+-+-+-+-+-+-++-+-+-+-+-+-+-+-++-+-+-+-+-+-+-+-+
    //+-+-+-+-+-+-+-+-+| public Methods |+-+-+-+-+-+-+-+-
    //+-+-+-+-+-+-+-+-++-+-+-+-+-+-+-+-++-+-+-+-+-+-+-+-+

    //Show you do you have any connection
    bool is_least_one_connection();

    //return true, if server have used disconnect() or another critical exit 
    bool is_server_stoped();

    //return true, when buf_in doesn't contain any pocket_message from clients
    bool is_buf_in_empty();

    //return true, when buf_out doesn't contain any pocket_message on server
    bool is_buf_out_empty();

    // the only one template constructor for make Server
    Server(const char* IP, const int& port);

    // start server
    void start();

    // use this method in all situations. if you don't use
    // you program can call terminated() or programm and threads
    // will finish not right. You can use RTTI and place method
    // in destructor. Read more about it in documentation, please
    void stop();

    //add your group with id of connections
    void add_group(int _id_group, std::vector <int>&& st);

    //add your group with id of connections
    void add_group(int _id_group, const std::vector <int>& st);

    //add your group
    void add_group(int _id_group);

    // add id to any group
    void add_to_group(int _id_group, int _id_person);

    //delete any id from group
    void delete_from_group(int _id_group, int _id_person);

    // function for better understanding your code
    static Number_of_group to_group(int _id_group);

    // find all groups of person where he exist
    std::vector<int> find_all_groups(int _id_person);

    // return list of online from all 
    std::vector <int> list_all_online();

    // count online from all
    int count_all_online();

    // return list online from group
    std::vector <int> list_online_in_group(int _id_group);

    // return count online from group 
    int count_online_in_group(int _id_group);

    // set path for download and send your files
    // read the documentation please
    void set_path_download(const std::string& s);

    // set path for download and send your files
    // read the documentation please
    void set_path_download(const char* s);

    // get last came file from client
    auto get_last_file()->info_pack_signal;

    // sinhrone waiting for new file
    void wait_file();

    // asinhrone waiting for new file
    bool is_buf_in_files_empty();

    //use stop() to avoid big problems with memory
    ~Server();
};


// exit signal for read or send thread with one connection
template <typename _Struct>
void Server<_Struct>::ext(std::atomic<bool>& mode)
{

    mode.store(true);
    event_out.lock();
    SetEvent(not_buf_out_empty);
    event_out.unlock();
}

//main thread for read from another client
template <typename _Struct>
void Server<_Struct>::_Read_th()
{

    block_before_open_th.lock();

    auto id = Connections.size() - 1;

    block_before_open_th.unlock();

    Commands type;
    char msg;

    while (!Exit.load() && !Connections[id].is_close.load())
    {
        if (recv(Connections[id].connect, &msg, sizeof(msg), 0) <= 0)
        {
            if (!block_buffer_out.try_lock()) continue;

#ifdef DEBUG_CONSOLE

            std::cout << "disconected Extra: " << id << "\n";

#endif
            ext(Connections[id].is_close);
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
#ifdef DEBUG_CONSOLE
                std::cout << "disconected: " << id << "\n";
#endif
                ext(Connections[id].is_close);
                closesocket(Connections[id].connect);
                break;
            }
            break;
            case(Commands::STRUCT):
            {
                _Struct data;

                recv(Connections[id].connect, (char*)&data, sizeof(_Struct), 0);

                block_buffer_in.lock();

                buf_in.push_back({ std::move(data),id });

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

                recv(Connections[id].connect, file_size_str, 64, 0);

                int file_size = atoi(file_size_str);
                char* bytes = new char[file_size];

                recv(Connections[id].connect, file_name, 64, 0);

                std::string _file_name = path;

#ifdef DEBUG_CONSOLE

                if (path == "" || !is_path_ready.load())
                    std::cout << "Warning: your path to download directory empty or haven't signed up yet, "
                    "your files in cpp directory\n";
#endif

                std::string file_name_to_export = file_name;

                _file_name += file_name;

                strcpy(file_name, _file_name.c_str());



                std::fstream file;
                file.open(file_name, std::ios_base::out | std::ios_base::binary);

                if (file.is_open())
                {

                    recv(Connections[id].connect, bytes, file_size, 0);

                    file.write(bytes, file_size);

                    file.close();
                }
                else throw std::exception("Error didn't write");

                block_buffer_in.unlock();

                block_bufeer_in_files.lock();

                buf_in_files.push_back(info_pack_signal{ file_name_to_export, int(id), size_t(file_size) });

                block_bufeer_in_files.unlock();
#ifdef DEBUG_CONSOLE
                std::cout << "file saved ok " << file_name << " size " << file_size + 128 << " bytes\n";
#endif
                delete[] bytes;
            }
            break;

            }

        }
    }
    end_threads.store(end_threads.load() + 1);
}



//main thread for send to another client
template <typename _Struct>
void Server<_Struct>::_Send_th()
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

            send(Connections[buf_out.front().second].connect, &command, sizeof(command), 0);

            auto data = buf_out.front();

            if (send(Connections[data.second].connect, (char*)&data.first, sizeof(_Struct), 0) > 0)
            {
#ifdef DEBUG_CONSOLE
                std::cout << "sended " << sizeof(_Struct) << " bytes to " << data.second << std::endl;
#endif
            }

            buf_out.pop_front();

        }

        block_buffer_out.unlock();


    }
    end_threads.store(end_threads.load() + 1);
}

//main thread for send files
template <typename _Struct>
void Server<_Struct>::_send_file(std::string file_name, int _id_person)
{

    auto file_name_handler = file_name;

    file_name = path + file_name;

    if (path == "" || !is_path_ready.load())
        throw "Error: your path to download directory empty or haven't signed up yet, "
        "your files in cpp directory\n";

    std::fstream file;
    file.open(file_name, std::ios_base::in | std::ios_base::binary);

    if (file.is_open())
    {
        int file_size = std::filesystem::file_size(file_name);

        char* bytes = new char[file_size];

        file.read((char*)bytes, file_size);

        auto command = char(Commands::FILE);

        block_buffer_out.lock();

        send(Connections[_id_person].connect, &command, sizeof(command), 0);

        send(Connections[_id_person].connect, std::to_string(file_size).c_str(), 64, 0);
        send(Connections[_id_person].connect, file_name_handler.c_str(), 64, 0);
        send(Connections[_id_person].connect, bytes, file_size, 0);

#ifdef DEBUG_CONSOLE
        std::cout << " sended file: " << file_name << " size " << file_size + 128 << " bytes "
            << " to " << _id_person << "\n";
#endif

        Sleep(10);

        block_buffer_out.unlock();

        file.close();

        delete[] bytes;
    }
    else
    {
        throw std::exception("File didn't open\n");
    }
}

//main thread for find new connections
template <typename _Struct>
void Server<_Struct>::new_connected()
{

    while (!Exit.load())
    {
        block_before_open_th.lock();

        Connection data(std::move(accept(sListen,
            (SOCKADDR*)&addr, &sizeofaddr)),
            std::move(std::thread(&Server::_Read_th, this)));

        Connections.push_back(std::move(data));
#ifdef DEBUG_CONSOLE
        if (!Exit.load()) std::cout << "connected new person\n";
#endif
        block_before_open_th.unlock();
        std::this_thread::sleep_for(10ms); //extra safe
    }
}

template <typename _Struct>
//Show you do you have any connection
bool Server<_Struct>::is_least_one_connection()
{
    return count_all_online() == 0;
}

//return true, if server have used disconnect() or another critical exit 
template <typename _Struct>
bool Server<_Struct>::is_server_stoped()
{

    return is_disconected.load();
}

//return true, when buf_in doesn't contain any pocket_message from clients
template <typename _Struct>
bool Server<_Struct>::is_buf_in_empty()
{

    block_buffer_in.lock();

    auto is = buf_in.empty();

    block_buffer_in.unlock();

    return is;
}

//return true, when buf_out doesn't contain any pocket_message on server
template <typename _Struct>
bool Server<_Struct>::is_buf_out_empty()
{

    block_buffer_out.lock();

    auto is = buf_out.empty();

    block_buffer_out.unlock();

    return is;
}

// the only one template constructor for make Server
template <typename _Struct>
Server<_Struct>::Server(const char* IP, const int& port) : ip(IP), port(port)
{

    DLLVersion = MAKEWORD(2, 1);

    if (WSAStartup(DLLVersion, &wsaData) != 0) {
        throw std::exception("Another Dll version\n");
    }

    sizeofaddr = sizeof(addr);
    addr.sin_addr.s_addr = inet_addr(IP);
    addr.sin_port = htons(port);
    addr.sin_family = AF_INET;

    sListen = socket(AF_INET, SOCK_STREAM, NULL);
    bind(sListen, (SOCKADDR*)&addr, sizeof(addr));
    listen(sListen, SOMAXCONN);
}

// start server
template <typename _Struct>
void Server<_Struct>::start()
{
    Send = std::move(std::thread(&Server::_Send_th, this));
    new_connects = std::move(std::thread(&Server::new_connected, this));
    std::this_thread::sleep_for(10ms); //Extra start
}

// use this method in all situations. if you don't use
// you program can call terminated() or programm and threads
// will finish not right. You can use RTTI and place method
// in destructor. Read more about it in documentation, please

template <typename _Struct>
void Server<_Struct>::stop()
{


    uint16_t time = 0;
#ifndef DEBUG_CONSOLE
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
#endif
    Exit.store(true);
    is_disconected.store(true);

    event_out.lock();
    SetEvent(not_buf_out_empty);
    event_out.unlock();


    block_buffer_in.lock();
    block_buffer_out.lock();

    auto command = char(Commands::CLOSE);

    for (int i = 0; i < Connections.size(); i++)
        send(Connections[i].connect, &command, sizeof(command), 0);

    for (int i = 0; i < Connections.size(); i++)
        closesocket(Connections[i].connect);

    std::cout << "disconected from all\n";

    block_buffer_out.unlock();
    block_buffer_in.unlock();
    std::this_thread::sleep_for(100ms); //Extra Exit
}

//add your group with id of connections
template <typename _Struct>
void Server<_Struct>::add_group(int _id_group, std::vector <int>&& st)
{

    auto mx = std::max_element(st.begin(), st.begin());
    auto mn = std::min_element(st.begin(), st.begin());

    if (*mn < 0 || *mx >= Connections.size()) throw std::exception("Attempt add group with bad id\n");

    auto group = groups.find(_id_group);
    if (group != groups.end())
        throw std::exception("Attempt add group to exist group\n");

    groups[_id_group] = Group(std::move(st));
}

//add your group with id of connections
template <typename _Struct>
void Server<_Struct>::add_group(int _id_group, const std::vector <int>& st)
{

    auto mx = std::max_element(st.begin(), st.begin());
    auto mn = std::min_element(st.begin(), st.begin());

    if (*mn < 0 || *mx >= Connections.size()) throw std::exception("Attempt add group with bad id\n");

    auto group = groups.find(_id_group);
    if (group != groups.end())
        throw std::exception("Attempt add to exist group\n");

    groups[_id_group] = Group(st);
}

template <typename _Struct>
void Server<_Struct>::add_group(int _id_group)
{
    auto group = groups.find(_id_group);
    if (group != groups.end())
        throw std::exception("Attempt add to exist group\n");

    groups[_id_group] = Group();
}

// add id to any group
template <typename _Struct>
void Server<_Struct>::add_to_group(int _id_group, int _id_person)
{

    if (_id_person >= Connections.size() || _id_person < 0)
        throw std::exception("Attempt add to group bad id\n");

    auto group = groups.find(_id_group);
    if (group == groups.end())
        throw std::exception("Attempt add to NULL group\n");

    group->second.group.emplace_back(_id_person);
}

//delete any id from group
template <typename _Struct>
void Server<_Struct>::delete_from_group(int _id_group, int _id_person)
{

    if (_id_person >= Connections.size() || _id_person < 0)
        throw std::exception("Attempt delete from group bad id\n");

    auto group = groups.find(_id_group);
    if (group == groups.end())
        throw std::exception("Attempt delete from NULL group\n");

    auto it = std::find(group->second.group.begin(), group->second.group.end(), _id_person);
    if (it == group->second.group.end())
        throw std::exception("Attempt delete null id from group\n");

    group->second.group.erase(it);
}

// function for better understanding your code
template <typename _Struct>
static typename Server<_Struct>::Number_of_group Server<_Struct>::to_group(int _id_group)
{

    return Number_of_group(std::move(_id_group));
}

// find all groups of person where he exist
template <typename _Struct>
std::vector<int> Server<_Struct>::find_all_groups(int _id_person)
{

    std::vector<int> v;
    for (auto i : groups)
    {
        auto it = std::find(i.second.group.begin(), i.second.group.end(), _id_person);
        if (it != i.second.group.end()) v.push_back(i.first);
    }
    return v;
}

// return list of online from all 
template <typename _Struct>
std::vector <int> Server<_Struct>::list_all_online()
{

    int j = 0;
    std::vector <int> v;
    for (auto i = Connections.begin(); i < Connections.end(); i++)
    {
        if (!i->is_close) v.push_back(j);
        j++;
    }
    return v;
}

// count online from all
template <typename _Struct>
int Server<_Struct>::count_all_online()
{

    int sum = 0;
    for (auto i = Connections.begin(); i < Connections.end(); i++)
    {
        if (!i->is_close) sum++;
    }
    return sum;
}

// return list online from group
template <typename _Struct>
std::vector <int> Server<_Struct>::list_online_in_group(int _id_group)
{

    std::vector <int> v;
    auto group = groups.find(_id_group);

    if (group == groups.end())
        throw std::exception("bad_group in list online groups\n");

    for (auto i : group->second.group)
    {
        if (!Connections[i].is_close) v.emplace_back(i);
    }
    return v;
}

// return count online from group 
template <typename _Struct>
int Server<_Struct>::count_online_in_group(int _id_group)
{

    int sum = 0;
    auto group = groups.find(_id_group);

    if (group == groups.end())
        throw std::exception("bad_group in count online groups\n");

    for (auto i : group->second.group)
    {
        if (!Connections[i].is_close) sum++;
    }
    return sum;
}

// set path for download and send your files
// read the documentation please
template <typename _Struct>
void Server<_Struct>::set_path_download(const std::string& s)
{
    path = s;
    is_path_ready.store(true);
}

// set path for download and send your files
// read the documentation please
template <typename _Struct>
void Server<_Struct>::set_path_download(const char* s)
{
    path = s;
    is_path_ready.store(true);
}

// get last came file from client
template <typename _Struct>
auto Server<_Struct>::get_last_file() -> info_pack_signal
{

    block_bufeer_in_files.lock();

    return buf_in_files.back();

    block_bufeer_in_files.unlock();
}

// sinhrone waiting for new file
template <typename _Struct>
void Server<_Struct>::wait_file()
{
    while (buf_in_files.empty()) std::this_thread::sleep_for(10ms);
}

// asinhrone waiting for new file
template <typename _Struct>
bool Server<_Struct>::is_buf_in_files_empty()
{
    block_bufeer_in_files.lock();
    return buf_in_files.empty();
    block_bufeer_in_files.unlock();
}

//use stop() to avoid big problems with memory
template <typename _Struct>
Server<_Struct>::~Server()
{

    Exit.store(true);

    event_out.lock();
    SetEvent(not_buf_out_empty);
    event_out.unlock();

    closesocket(sListen);

    new_connects.join();
    Send.join();

    for (int i = 0; i < timed_for_files.size(); i++)
    {
        timed_for_files[i].join();
    }
    for (decltype(Connections.size()) i = 0; i < Connections.size(); i++)
    {
        Connections[i].Read.join();
        closesocket(Connections[i].connect);
    }
    if (!is_disconected.load()) std::cout << "disconected from all\n";
}


//operator recv pocket from person
template<typename _Struct>
Server < _Struct >& operator>>(Server < _Struct >& sv, std::pair <_Struct, int >& st)
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


//operator send pocket to person
template<typename _Struct>
Server< _Struct>& operator<<(Server < _Struct >& sv, std::pair <_Struct, int >&& st)
{

    sv.block_buffer_out.lock();

    sv.buf_out.push_back(std::move(st));

    sv.event_out.lock();
    SetEvent(sv.not_buf_out_empty);
    sv.event_out.unlock();

    sv.block_buffer_out.unlock();

    return sv;
}


//operator send pocket to person
template<typename _Struct>
Server< _Struct>& operator<<(Server < _Struct >& sv, const std::pair <_Struct, int >& st)
{

    sv.block_buffer_out.lock();

    sv.buf_out.push_back(st);

    sv.event_out.lock();
    SetEvent(sv.not_buf_out_empty);
    sv.event_out.unlock();

    sv.block_buffer_out.unlock();

    return sv;
}


//operator send pocket to all
template<typename _Struct>
Server< _Struct>& operator<<(Server < _Struct >& sv, const _Struct& st)
{

    sv.block_buffer_out.lock();

    for (decltype(sv.Connections.size()) i = 0; i < sv.Connections.size(); i++)
    {
        if (!sv.Connections[i].is_close)
            sv.buf_out.push_back({ st,i });
    }

    sv.event_out.lock();
    SetEvent(sv.not_buf_out_empty);
    sv.event_out.unlock();

    sv.block_buffer_out.unlock();

    return sv;
}


//operator send pocket to person
template<typename _Struct>
Server< _Struct>& operator<<(Server < _Struct >& sv, _Struct&& st)
{

    sv.block_buffer_out.lock();

    for (decltype(sv.Connections.size()) i = 0; i < sv.Connections.size(); i++)
    {
        if (!sv.Connections[i].is_close)
            sv.buf_out.push_back({ st,i });
    }

    sv.event_out.lock();
    SetEvent(sv.not_buf_out_empty);
    sv.event_out.unlock();

    sv.block_buffer_out.unlock();

    return sv;
}


//operator send pocket to groups
template<typename _Struct>
Server<_Struct>& operator<<(Server<_Struct>& sv, std::pair <_Struct,
    typename Server<_Struct>::Number_of_group >&& st)
{

    sv.block_buffer_out.lock();

    auto group_it = sv.groups.find(st.second.n);

    if (group_it == sv.groups.end())
        throw std::exception("Bad group in operator<<\n");

    auto group = group_it->second;

    for (auto i : group.group)
    {
        if (!sv.Connections[i].is_close)
            sv.buf_out.push_back({ st.first,i });
    }

    sv.event_out.lock();
    SetEvent(sv.not_buf_out_empty);
    sv.event_out.unlock();

    sv.block_buffer_out.unlock();

    return sv;
}


//operator send file to all
template<typename _Struct>
Server<_Struct>& operator<<(Server<_Struct>& sv, const std::string& _name_file)
{

    auto it1 = std::find(_name_file.begin(), _name_file.end(), '\\');
    if (it1 != _name_file.end())
        throw std::exception("Attempt send file with absolute path, use"
            " set_path_download()\n");

    auto it2 = std::find(_name_file.begin(), _name_file.end(), '/');
    if (it2 != _name_file.end())
        throw std::exception("Attempt send file with absolute path, use"
            " set_path_download()\n");

    for (decltype(sv.Connections.size()) i = 0; i < sv.Connections.size(); i++)
    {
        if (!sv.Connections[i].is_close)
        {
            std::thread t([&]()
                {
                    sv._send_file(_name_file, i);
                });
            Sleep(10);

            sv.timed_for_files.push_back(std::move(t));
        }
    }
}


//operator send file to person
template<typename _Struct>
Server<_Struct>& operator<<(Server<_Struct>& sv, std::pair <std::string, int> st)
{

    auto& _name_file = st.first;

    auto it1 = std::find(_name_file.begin(), _name_file.end(), '\\');
    if (it1 != _name_file.end())
        throw std::exception("Attempt send file with absolute path, use"
            " set_path_download()\n");

    auto it2 = std::find(_name_file.begin(), _name_file.end(), '/');
    if (it2 != _name_file.end())
        throw std::exception("Attempt send file with absolute path, use"
            " set_path_download()\n");

    std::thread t([&]()
        {
            sv._send_file(_name_file, st.second);
        });
    Sleep(10);

    sv.timed_for_files.push_back(std::move(t));

}


//operator send file to group
template<typename _Struct>
Server<_Struct>& operator<<(Server<_Struct>& sv, std::pair <std::string,
    typename Server<_Struct>::Number_of_group >&& st)
{

    auto& _name_file = st.first;

    auto it1 = std::find(_name_file.begin(), _name_file.end(), '\\');
    if (it1 != _name_file.end())
        throw std::exception("Attempt send file with absolute path, use"
            " set_path_download()\n");

    auto it2 = std::find(_name_file.begin(), _name_file.end(), '/');
    if (it2 != _name_file.end())
        throw std::exception("Attempt send file with absolute path, use"
            " set_path_download()\n");

    auto group_it = sv.groups.find(st.second.n);

    if (group_it == sv.groups.end())
        throw std::exception("Bad group in operator<<\n");

    auto group = group_it->second;

    for (auto i : group.group)
    {
        if (!sv.Connections[i].is_close)
        {
            std::thread t([&]()
                {
                    sv._send_file(_name_file, i);
                });
            Sleep(10);

            sv.timed_for_files.push_back(std::move(t));
        }
    }

}


//operator recv files from client
template<typename _Struct>
Server<_Struct>& operator>>(Server<_Struct>& sv,
    typename Server<_Struct>::info_pack_signal& pack)
{

    sv.block_bufeer_in_files.lock();

#ifdef DEBUG_CONSOLE
    if (sv.buf_in_files.empty())
    {
        std::cout << "Warning files buffer is empty\n";
        sv.block_bufeer_in_files.unlock();
        return sv;
    }
#endif
    auto data = sv.buf_in_files.front();

    sv.buf_in_files.pop_front();

    pack = data;

    sv.block_bufeer_in_files.unlock();
}

int main()
{
    Server<void> sv("127.0.0.1", 1111);


}