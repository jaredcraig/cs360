/*
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include "inet-server.h"
#include "buffer.h"

using namespace std;

Buffer b;
pthread_t threads[NUM_THREADS];

void *threadHandle(void*);

int main(int argc, char **argv) {
	int option, port;

	// setup default arguments
	port = 5000;

	// process command line options using getopt()
	// see "man 3 getopt"
	while ((option = getopt(argc, argv, "p:")) != -1) {
		switch (option) {
		case 'p':
			port = atoi(optarg);
			break;
		default:
			cout << "server [-p port]" << endl;
			exit (EXIT_FAILURE);
		}
	}

	InetServer server = InetServer(port);
	server.run(&b);

	for (int i = 0; i < NUM_THREADS; i++) {
		int rc;
		rc = pthread_create(&threads[i], NULL, &threadHandle, &server);
		if (rc) {
			cerr << "Unable to create thread, " << rc << endl;
			exit(-1);
		}
	}

	server.serve();

	for (int i = 0; i < NUM_THREADS; i++) {
		pthread_join(threads[i], NULL);
	}
}

//-----------------------------------------------------------------------------
void *threadHandle(void *vptr) {
	Server *s = (Server*) vptr;
	do {
		pthread_mutex_lock(&s->server_lock);
			s->handle();
		pthread_mutex_unlock(&s->server_lock);
	} while (1);
}


















#include "server.h"

Server::Server() {
	// setup variables
	buflen_ = 1024;
	buf_ = new char[buflen_ + 1];

	pthread_mutex_init(&server_lock, NULL);
 }

Server::~Server() {
	delete buf_;
}

//-----------------------------------------------------------------------------
void Server::run(Buffer *b) {
	// create and run the server
	this->buffer = b;
	create();
}

//-----------------------------------------------------------------------------
void Server::create() {
}

//-----------------------------------------------------------------------------
void Server::close_socket() {
}

//-----------------------------------------------------------------------------
void Server::serve() {
// setup client
	int client;
	struct sockaddr_in client_addr;
	socklen_t clientlen = sizeof(client_addr);
	// accept clients
	while ((client = accept(server_, (struct sockaddr *) &client_addr,
			&clientlen)) > 0) {
		buffer->append(client);
	}
	cout << "SERVER CLOSING SOCKET";
	close_socket();
}

//-----------------------------------------------------------------------------
void Server::handle() {
	while (1) {
		int client = buffer->take();
		// process the message

		string data = get_request(client);
		cache += data;

		if (cache.empty())
			break;

		string message = readMessage();

		if (message == "") {
			continue;
		}

		string response = parse(message, client);
		send_response(client, response);

		buffer->append(client);
		return;
	}
}
//-----------------------------------------------------------------------------
string Server::readMessage() {
	int index = cache.find("\n");
	if (index == -1) {
		return "";
	}

	// copy all characters up to and including the newline character
	string message = cache.substr(0, index + 1);

	cache = cache.substr(index + 1, cache.size());

	return message;
}

// PARSE ----------------------------------------------------------------------
string Server::parse(string &message, int client) {
	istringstream iss;
	string cmd = "";
	string name = "";
	string subject = "";
	string response = "error invalid message\n";
	int i = -1;

	iss.clear();
	iss.str(message);
	iss >> cmd;

	if (cmd == "put") {
		iss >> name;
		iss >> subject;
		iss >> i;

		if (iss.fail()) {
			return "error invalid message\n";
		}
		string data = readPut(client, i);

		if (!addMessage(name, subject, data)) {
			return "Failed to add message!";
		}
		response = "OK\n";

	} else if (cmd == "list") {
		iss >> name;
		if (iss.fail())
			return "error invalid message\n";
		response = getSubjectList(name);

	} else if (cmd == "get") {

		iss >> name;
		iss >> i;

		if (iss.fail())
			return "error invalid message\n";

		string message = getMessage(name, i);
		if (message == "")
			return "error no such message for that user\n";

		response = message;
	} else if (cmd == "reset") {
		if (resetMessages())
			response = "OK\n";
	}
	return response;
}

//-----------------------------------------------------------------------------
bool Server::resetMessages() {
	map<string, vector<vector<string> > >::iterator it;
	it = messages.begin();
	while (it != messages.end()) {
		messages.erase(it);
		it++;
	}
	bool success = messages.empty();
	return success;
}

//-----------------------------------------------------------------------------
string Server::getSubjectList(string name) {
	map<string, vector<vector<string> > >::iterator it;

	it = messages.find(name);

	if (it == messages.end())
		return " 0\n";

	string list = parseList(it->second);
	return list;
}

//-----------------------------------------------------------------------------
string Server::parseList(vector<vector<string> > list) {
	ostringstream oss;
	ostringstream oss_data;

	for (int i = 0; i < list.size(); i++) {
		oss_data << i + 1 << " " << list[i][0] << "\n";
	}
	string data = oss_data.str();
	oss << "list " << list.size() << endl << data;
	return oss.str();
}

//-----------------------------------------------------------------------------
string Server::getMessage(string name, int index) {
	map<string, vector<vector<string> > >::iterator it;
	if (index <= 0)
		return "";
	it = messages.find(name);

	if (it == messages.end())
		return "";

	if (index - 1 >= it->second.size())
		return "";
	vector < string > message = it->second[index - 1];
	string subject = message[0];
	string data = message[1];

	ostringstream oss;
	oss << "message " << subject << " " << data.size() << "\n" << data << "\n";
	if (!oss.fail())
		return oss.str();
	return "";
}

//-----------------------------------------------------------------------------
bool Server::addMessage(string name, string subject, string &data) {
	map<string, vector<vector<string> > >::iterator it;
	vector < vector<string> > messageList;
	vector < string > subject_data;

	it = messages.find(name);
	if (it == messages.end()) {
		it =
				messages.insert(
						pair<string, vector<vector<string> > >(name,
								messageList)).first;
	}

	messageList = it->second;
	subject_data.push_back(subject);
	subject_data.push_back(data);
	messageList.push_back(subject_data);
	it->second = messageList;

	return true;
}

//-----------------------------------------------------------------------------
string Server::readPut(int client, int length) {
	string data = "";
	data += cache;
	while (data.size() < length) {
		string d = get_request(client);
		if (d.empty()) {
			return "";
		}
		data += d;
	}
	if (data.size() > length) {
		cache = data.substr(length, data.size());
		data = data.substr(0, length);
	} else {
		cache.clear();
	}

	return data;
}

//-----------------------------------------------------------------------------
string Server::get_request(int client) {
	string request = "";
// read until we get a newline
	while (request.find("\n") == string::npos) {
		memset(buf_, 0, buflen_ + 1);
		int nread = recv(client, buf_, 1024, 0);
		if (nread < 0) {
			if (errno == EINTR)
				// the socket call was interrupted -- try again
				continue;
			else
				// an error occurred, so break out
				return "";
		} else if (nread == 0) {
			// the socket is closed
			return "";
		}
		// be sure to use append in case we have binary data
		request.append(buf_, nread);
	}
// a better server would cut off anything after the newline and
// save it in a cache
	return request;
}

//-----------------------------------------------------------------------------
bool Server::send_response(int client, string response) {
// prepare to send response
	const char* ptr = response.c_str();
	int nleft = response.length();
	int nwritten;
// loop to be sure it is all sent
	while (nleft) {
		if ((nwritten = send(client, ptr, nleft, 0)) < 0) {
			if (errno == EINTR) {
				// the socket call was interrupted -- try again
				continue;
			} else {
				// an error occurred, so break out
				perror("write");
				return false;
			}
		} else if (nwritten == 0) {
			// the socket is closed
			return false;
		}
		nleft -= nwritten;
		ptr += nwritten;
	}
	return true;
}














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


*/
