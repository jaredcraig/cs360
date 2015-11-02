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
#include "buffer.h"

using namespace std;

//------------------------------------------------------------------------------
class Server {
public:
	Server();
	~Server();

	void run(Buffer *b);

	virtual void create();
	virtual void close_socket();
	void serve();
	void handle();
	string parse(string&, int);
	string get_request(int);
	bool send_response(int, string);
	bool addMessage(string, string, string&);
	string getMessage(string, int);
	string getSubjectList(string);
	string parseList(vector<vector<string> >);
	bool resetMessages();
	string readMessage();
	string readPut(int, int);
	void *threadHandle(void *);
	bool load_cache(int);

	map<string, vector<vector<string> > > messages;
	int server_;
	int buflen_;
	char* buf_;
	string cache;

	Buffer* buffer;

	pthread_mutex_t server_lock;
	
};

