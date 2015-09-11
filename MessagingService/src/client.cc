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
	string command;

	// loop to handle user interface
	while (1) {
		prompt();
		getline(cin, command);
		command += "\n";
		if (!parseCommand(command)) {
			cout << "I don't recognize that command." << endl;
		}
	}
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
		try {
			iss >> name;
			iss >> subject;
		} catch (...) {
			return false;
		}
		string data = getMessage();
		sendPut(name, subject, data);
		responseToPut();
		return true;
	} else if (cmd == "list") {
		string name;
		try {
			iss >> name;
		} catch (...) {
			return false;
		}
		sendList(name);
		responseToList();
		return true;

	} else if (cmd == "read") {
		string name;
		int index;
		try {
			iss >> name;
			iss >> index;
		} catch (...) {
			return false;
		}
		sendRead(name, index);
		responseToRead();
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
void Client::sendPut(string name, string subject, string data) {
	stringstream ss;
	ss << "put " << name << " " << subject << " " << " " << data.size() << "\n"
			<< data;
	send_request(ss.str());
}

//-----------------------------------------------------------------------------
void Client::sendRead(string name, int index) {

}

//-----------------------------------------------------------------------------
void Client::sendList(string name) {

}

//-----------------------------------------------------------------------------
void Client::responseToRead() {

}

//-----------------------------------------------------------------------------
void Client::responseToList() {

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
