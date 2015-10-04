#include "server.h"
#include <pthread.h>

//-----------------------------------------------------------------------------
Server::Server(int port) {
	// setup variables
	pthread_mutex_init(&lock, NULL);
	server_ = 0;
	port_ = port;
	buflen_ = 1024;
	buf_ = new char[buflen_ + 1];
}

//-----------------------------------------------------------------------------
Server::~Server() {
	delete buf_;
}

//-----------------------------------------------------------------------------
void Server::run() {
	// create and run the server
	create();
	serve();
}

//-----------------------------------------------------------------------------
void Server::create() {

	struct sockaddr_in server_addr;

	// setup socket address structure
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port_);
	server_addr.sin_addr.s_addr = INADDR_ANY;

	// create socket
	server_ = socket(PF_INET, SOCK_STREAM, 0);
	if (!server_) {
		perror("socket");
		exit(-1);
	}

	// set socket to immediately reuse port when the application closes
	int reuse = 1;
	if (setsockopt(server_, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse))
			< 0) {
		perror("setsockopt");
		exit(-1);
	}

	// call bind to associate the socket with our local address and
	// port
	if (bind(server_, (const struct sockaddr *) &server_addr,
			sizeof(server_addr)) < 0) {
		perror("bind");
		exit(-1);
	}

	// convert the socket to listen for incoming connections
	if (listen(server_, SOMAXCONN) < 0) {
		perror("listen");
		exit(-1);
	}
}

//-----------------------------------------------------------------------------
void Server::close_socket() {
	close(server_);
}

// SERVE ======================================================================
void Server::serve() {
	// setup client
	int client;
	struct sockaddr_in client_addr;
	socklen_t clientlen = sizeof(client_addr);

	// accept clients
	while ((client = accept(server_, (struct sockaddr *) &client_addr,
			&clientlen)) > 0) {
		buffer.append(client);
	}
	close_socket();
}

// HANDLE =====================================================================
void Server::handle() {
	// loop to handle all requests
	while (1) {
		int client = buffer.take();
		string cache = buffer.getCache(client);

		// get a request
		string request = get_request(client);

		// break if client is done or an error occurred
		if (request.empty())
			break;

		cache += request;
		string message = readMessage(cache);

		if (message.empty()) {
			continue;
		}

		//cout << client << " <SERVER> MESSAGE: " << message;
		string response = parse(message, client, cache);
		//cout << client << " <SERVER> RESPONSE: " << response;

		// send response
		bool success = send_response(client, response);
		if (not success)
			break;

		buffer.append(client);
	}
}

//-----------------------------------------------------------------------------
string Server::readMessage(string &cache) {
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
string Server::parse(string &message, int client, string &cache) {
	istringstream iss;
	string cmd = "";
	string name = "";
	string subject = "";
	string response = "error invalid message\n";
	int i = -1;
	iss.clear();
	iss.str(message);
	iss >> cmd;

	// PUT ------------------------------------------------
	if (cmd == "put") {
		iss >> name;
		iss >> subject;
		iss >> i;
		if (iss.fail())
			return "error invalid message\n";
		string data = readPut(client, i, cache);
		if (data == "")
			return "error could not read entire message\n";
		insertMessage(name, subject, data);

		response = "OK\n";
		return response;

		// LIST -----------------------------------------------
	} else if (cmd == "list") {
		iss >> name;
		if (iss.fail())
			return "error invalid message\n";

		response = getSubjectList(name);
		return response;

		// GET ------------------------------------------------
	} else if (cmd == "get") {
		iss >> name;
		iss >> i;
		if (iss.fail())
			return "error invalid message\n";

		response = getMessage(name, i);
		return response;

		// RESET ----------------------------------------------
	} else if (cmd == "reset") {
		resetMessages();

		response = "OK\n";
		return response;
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
		return "list 0\n";
	return parseList(it->second);
}

//-----------------------------------------------------------------------------
string Server::parseList(vector<vector<string> > list) {
	ostringstream oss;
	ostringstream oss_data;

	for (unsigned i = 0; i < list.size(); i++) {
		oss_data << i + 1 << " " << list[i][0] << endl;
	}
	string subjects = oss_data.str();
	oss << "list " << list.size() << endl << subjects;
	return oss.str();
}

//-----------------------------------------------------------------------------
string Server::getMessage(string name, int index) {
	map<string, vector<vector<string> > >::iterator it;
	if (index <= 0)
		return "error invalid message\n";
	it = messages.find(name);
	if (it == messages.end())
		return "error no such message for that user\n";
	if ((unsigned) index - 1 >= it->second.size())
		return "error invalid message\n";

	vector<string> message = it->second[index - 1];
	string subject = message[0];
	string data = message[1];

	ostringstream oss;
	oss << "message " << subject << " " << data.size() << endl;
	oss << data;
	return oss.str();
}

//-----------------------------------------------------------------------------
bool Server::insertMessage(string name, string subject, string &data) {
	map<string, vector<vector<string> > >::iterator it;
	vector<vector<string> > messageList;
	vector<string> subject_data;

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
string Server::readPut(int client, int length, string &cache) {
	string data = cache;
	while (data.size() < (unsigned) length) {
		memset(buf_, 0, buflen_ + 1);
		recv(client, buf_, 1024, 0);
		string d = buf_;
		if (d.empty()) {
			return "";
		}
		data += d;
	}
	if (data.size() > (unsigned) length) {
		cache = data.substr(length, data.size());
		data = data.substr(0, length);
	} else {
		cache = "";
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
