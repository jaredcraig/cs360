#pragma once

#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include <sstream>
#include <fstream>
#include <iostream>
#include <string>

using namespace std;

class Client {
public:
	Client();
	~Client();

	void run();

protected:
	virtual void close_socket();
    virtual void create();
	bool send_request(string);
	string get_response();
	void prompt();
	bool parseCommand(string);
	string getMessage();
	void responseToPut();
	void responseToRead();
	void responseToList();
	void sendPut(string, string, string);
	void sendRead(string, int);
	void sendList(string);
	int server_;
	int buflen_;
	char* buf_;
};
