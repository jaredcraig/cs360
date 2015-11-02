#pragma once

#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string>
#include <map>
#include <vector>
#include <sstream>
#include <iostream>

#include "message.h"

using namespace std;

class Server {
public:
    Server(int port, bool debug);
    ~Server();

    void run();
    
private:
    void create();
    void close_socket();
    void serve();
    void handle(int);
    string get_request(int);
    bool send_response(int, string);
    string parse_request(int, string);
    void read_put(int, Message&);
    void store_message(string, Message&);
    vector<Message> get_subjects(string);
    Message get_message(string, int);

    int port_;
    int server_;
    int buflen_;
    char* buf_;
    string cache;
    bool debug;
    map<string, vector<Message> > messages;
};
