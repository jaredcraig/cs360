#pragma once

#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <map>
#include <string>

using namespace std;

class Server {
public:
	Server();
	~Server();

	void run();

protected:
	virtual void create();
	virtual void close_socket();
	void serve();
	void handle(int);
	string parse(string);
	string get_request(int);
	bool send_response(int, string);
	bool addMessage(string, string, string);
	string findMessage(string,int);
	string readPut(string, int);

	map<string, vector<vector<string> > > messages;
	int server_;
	int buflen_;
	char* buf_;
};
