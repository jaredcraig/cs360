#pragma once

#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include "buffer.h"

#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include <map>
#define NUM_THREADS 10

using namespace std;

class Server {
public:
	Server(int port);
	~Server();

	void run();
	void handle();

	pthread_mutex_t lock;

private:
	void create();
	void close_socket();
	void serve();
	void *threadHandle(void*);
	string get_request(int);
	bool send_response(int, string);
	string parse(string&, int, string&);
	bool insertMessage(string, string, string&);
	string getMessage(string, int);
	string getSubjectList(string);
	string parseList(vector<vector<string> >);
	bool resetMessages();
	string readMessage(string &);
	string readPut(int, int, string&);

	int port_;
	int server_;
	int buflen_;
	char* buf_;

	Buffer buffer;
	map<string, vector<vector<string> > > messages;
};
