#include "server.h"

Server::Server() {
	// setup variables
	buflen_ = 1024;
	buf_ = new char[buflen_ + 1];
}

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

		handle(client);
	}
	close_socket();
}

//-----------------------------------------------------------------------------
void Server::handle(int client) {
// loop to handle all requests
	while (1) {
		// get a request
		string request = get_request(client);
		string response = parse(request);

		// break if client is done or an error occurred
		if (request.empty())
			break;
		// send response
		bool success = send_response(client, response);
		// break if an error occurred
		if (not success)
			break;
	}
	close(client);
}

// PARSE ----------------------------------------------------------------------
string Server::parse(string message) {
	istringstream iss;
	string cmd = "";
	string name = "";
	string subject = "";
	string response = "error invalid message\n";
	int i = -1;
	iss.clear();

	try {
		iss.str(message);
		iss >> cmd;

		if (cmd == "put") {
			iss >> name;
			iss >> subject;
			iss >> i;

			if (iss.fail()) {
				return "error invalid message\n";
			}

			string data = readPut(message, i);

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

			response = "";
		}
	} catch (exception &e) {
		cout << "error: " << e.what() << endl;
	}
	return response;
}

//-----------------------------------------------------------------------------
string Server::getSubjectList(string name) {
	string list;
	map<string, vector<vector<string> > >::iterator it;
	it = messages.find(name);

	if (it == messages.end())
		return "";

	return parseList(it->second);
}

//-----------------------------------------------------------------------------
string Server::parseList(vector<vector<string> > list) {
	ostringstream oss;
	ostringstream oss_data;

	for (int i = 0; i < list.size(); i++) {
		oss_data << i + 1 << " " << list[i][0] << "\n";
	}
	string data = oss_data.str();
	oss << "list " << data.size() << " " << data;
	return oss.str();
}

//-----------------------------------------------------------------------------
string Server::findMessage(string name, int index) {
	// TODO
	return NULL;
}

//-----------------------------------------------------------------------------
bool Server::addMessage(string name, string subject, string data) {
	map<string, vector<vector<string> > >::iterator it;
	vector<vector<string> > messageList;
	vector<string> subject_data;

	try {
		it = messages.find(name);
		if (it == messages.end()) {
			it =
					messages.insert(
							pair<string, vector<vector<string> > >(name,
									messageList)).first;
		}

		messageList = it->second;
		cout << "<SERVER>[addMessage-messageList-size] " << messageList.size()
				<< endl;

		subject_data.push_back(subject);
		subject_data.push_back(data);
		messageList.push_back(subject_data);
		it->second = messageList;

		cout << "<SERVER>[addMessage-messageList-size] " << messageList.size()
				<< endl;

	} catch (exception &e) {
		cout << "ERROR: " << e.what() << endl;
		return false;
	}
	return true;
}

//-----------------------------------------------------------------------------
string Server::readPut(string message, int length) {
	return message.substr(message.size() - length, length);
}

//-----------------------------------------------------------------------------
string Server::get_request(int client) {
	string request = "";
// read until we get a newline
	while (request.find("\n") == string::npos) {
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
