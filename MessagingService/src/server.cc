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
string Server::findMessage(string name, int index) {
	// TODO
	return NULL;
}

//-----------------------------------------------------------------------------
bool Server::addMessage(string name, string subject, string data) {
	vector<vector<string> > messageList;
	map<string, vector<vector<string> > >::iterator it;
	try {
		it = messages.find(name);
		if (it == messages.end()) {
			it =
					messages.insert(
							pair<string, vector<vector<string> > >(name,
									messageList)).first;
		}

		vector<string> subject_data;
		subject_data.push_back(subject);
		subject_data.push_back(data);
		messageList.push_back(subject_data);
		it->second = messageList;

	} catch (exception &e) {
		cout << "ERROR: " << e.what() << endl;
		return false;
	}
	return true;
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

//-----------------------------------------------------------------------------
string Server::readPut(string message, int length) {
	return message.substr(message.size() - length, length);
}

//-----------------------------------------------------------------------------
string Server::parse(string message) {
	string cmd = "";
	istringstream iss;
	iss.str(message);
	iss >> cmd;
	if (cmd == "put") {
		string name;
		iss >> name;
		string subject;
		iss >> subject;
		int length;
		iss >> length;
		if (!iss.fail()) {
			string data = readPut(message, length);
			if (!addMessage(name, subject, data))
				return "Failed to add message!";
		}
		cout << "<SERVER> messages size: "
				<< messages.find(name)->second[0].size() << endl;
		return "OK\n";
	} else if (cmd == "list") {
	} else if (cmd == "get") {
	}
	return "error invalid message\n";
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
