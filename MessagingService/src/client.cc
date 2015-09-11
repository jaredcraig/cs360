#include "client.h"

Client::Client() {
	// setup variables
	buflen_ = 1024;
	buf_ = new char[buflen_ + 1];
}

Client::~Client() {
}

//-----------------------------------------------------------------------------
void Client::run() {
	create();
	string command;

	// loop to handle user interface
	while (1) {
		prompt();
		getline(cin, command);
		command += "\n";
		if (parseCommand(command) == false) {
			cout << "I don't recognize that command!." << endl;
		}
	}
}

//-----------------------------------------------------------------------------
void
Client::create() {
}

//-----------------------------------------------------------------------------
void
Client::close_socket() {
}

//-----------------------------------------------------------------------------
bool Client::parseCommand(string command) {
	string cmd = "";
	istringstream iss;
	iss.str(command);
	iss >> cmd;
	if (cmd == "quit") {
		exit(0);
	} else if (cmd == "send") {
		string name;
		string subject;
		iss >> name;
		iss >> subject;
		if (iss.fail())
			return false;
		string data = getMessage();
		sendPut(name, subject, data);
		responseToPut();
		return true;
	} else if (cmd == "list") {
		string name;
		iss >> name;
		if (iss.fail())
			return false;
		sendList(name);
		responseToList();
		return true;

	} else if (cmd == "read") {
		string name;
		int index;
		iss >> name;
		iss >> index;
		if (iss.fail())
			return false;
		sendRead(name, index);
		responseToRead();
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
void Client::prompt() {
	cout << "% ";
}

//-----------------------------------------------------------------------------
string Client::getMessage() {
	string line;
	stringstream ss;
	cout << "- Type your message. End with a blank line -\n";
	while (getline(cin, line)) {
		if (line == "") {
			break;
		}
		ss << line << "\n";
	}
	return ss.str();
}

//-----------------------------------------------------------------------------
void Client::sendPut(string name, string subject, string data) {
	ostringstream oss;
	oss << "put " << name << " " << subject << " " << data.size() << endl
			<< data;
	send_request(oss.str());
}

//-----------------------------------------------------------------------------
void Client::sendRead(string name, int index) {
	ostringstream oss;
	oss << "get " << name << " " << index << endl;
	send_request(oss.str());
}

//-----------------------------------------------------------------------------
void Client::sendList(string name) {
	ostringstream oss;
	oss << "list " << name << endl;
	send_request(oss.str());
}

//-----------------------------------------------------------------------------
void Client::responseToRead() {
	string response = get_response();
	string word;
	istringstream iss;
	iss.str(response);
	iss >> word;
	if (word != "read") {
		cout << "Server returned bad message: " << response;
		return;
	}
	
	cout << iss.str();
}

//-----------------------------------------------------------------------------
void Client::responseToList() {
	string response = get_response();
	string word;
	istringstream iss;
	iss.str(response);
	iss >> word;
	if (word != "list") {
		cout << "Server returned bad message: " << response;
		return;
	}
	int i;
	iss >> i;
	string data;
	while (iss >> i) {
		cout << i << " ";
		iss.clear();
		iss >> data;
		cout << data << endl;
	}
}

//-----------------------------------------------------------------------------
void Client::responseToPut() {
	string response = get_response();
	if (response != "OK\n") {
		cout << "Server returned bad message: " << response << endl;
		return;
	}
}

//-----------------------------------------------------------------------------
bool Client::send_request(string request) {
	// prepare to send request
	const char* ptr = request.c_str();
	int nleft = request.length();
	int nwritten;
	// loop to be sure it is all sent
	while (nleft) {
		if ((nwritten = send(server_, ptr, nleft, 0)) < 0) {
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

//-----------------------------------------------------------------------------
string Client::get_response() {
	string response = "";
	// read until we get a newline
	while (response.find("\n") == string::npos) {
		int nread = recv(server_, buf_, 1024, 0);
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
		response.append(buf_, nread);
	}
	// a better client would cut off anything after the newline and
	// save it in a cache
	return response;
}
