#pragma once

#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include <fstream>
#include <iostream>
#include <string>

using namespace std;

class Client {
public:
    Client(string host, int port, bool debug);
    ~Client();

    void run();

private:
    virtual void create();
    virtual void close_socket();
    void message();
    void prompt();
    bool parse_request(string);
    bool send_request(string);
    string get_response();
    string get_user_message();
    void send_put(string, string, string);
    void respond_to_put();
    void send_list(string);
    void respond_to_list();
    void read_list_response(int);
    void send_get(string, string);
    void respond_to_get();
    void read_message_response(string, int);

    string host_;
    int port_;
    int server_;
    int buflen_;
    char* buf_;
    string cache;
    bool debug;
};
