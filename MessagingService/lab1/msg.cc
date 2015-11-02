#include <sstream>

#include "msg.h"

Client::Client(string host, int port, bool debug_) {
    // setup variables
    host_ = host;
    port_ = port;
    buflen_ = 1024;
    buf_ = new char[buflen_+1];
    debug = debug_;
}

Client::~Client() {
}

void Client::run() {
    // connect to the server and run message program
    create();
    message();
}

void
Client::create() {
    struct sockaddr_in server_addr;

    // use DNS to get IP address
    struct hostent *hostEntry;
    hostEntry = gethostbyname(host_.c_str());
    if (!hostEntry) {
        cout << "No such host name: " << host_ << endl;
        exit(-1);
    }

    // setup socket address structure
    memset(&server_addr,0,sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port_);
    memcpy(&server_addr.sin_addr, hostEntry->h_addr_list[0], hostEntry->h_length);

    // create socket
    server_ = socket(PF_INET,SOCK_STREAM,0);
    if (!server_) {
        perror("socket");
        exit(-1);
    }

    // connect to server
    if (connect(server_,(const struct sockaddr *)&server_addr,sizeof(server_addr)) < 0) {
        perror("connect");
        exit(-1);
    }
}

void
Client::close_socket() {
    close(server_);
}

void
Client::message() {
    string line;
    
    // loop to handle user interface
    while (true) {
    	//prompt the user
    	prompt();
    	// get user request
    	getline(cin, line);
        // append a newline
        line += "\n";
        // parse request
        bool result = parse_request(line);
        // prompt correct request if invalid
        if(!result) {
        	cout << "I don't recognize that command." << endl;
        }
    }
    close_socket();
}

void
Client::prompt() {
	cout << "% ";
}

bool
Client::parse_request(string request) {
	string command;
	istringstream input(request);
	input >> command;
	if(command.empty()) {
		return false;
	}
	if(command == "quit") {
		close_socket();
		exit(0);
	}
	if(command == "send") {
		// send message
		string user;
		string subject;
		string message;
		input >> user;
		input >> subject;
		if(user.empty() || subject.empty()) {
			return false;
		}
		message = get_user_message();
		send_put(user, subject, message);
		respond_to_put();
		return true;
	}
	if(command == "list") {
		// list messages
		string user;
		input >> user;
		if(user.empty()) {
			return false;
		}
		send_list(user);
		respond_to_list();
		return true;
	}
	if(command == "read") {
		// read messages
		string user;
		string index;
		input >> user;
		input >> index;
		if(user.empty() || index.empty()) {
			return false;
		}
		send_get(user, index);
		respond_to_get();
		return true;
	}
	return false;
}

// generic message handling
bool
Client::send_request(string request) {
    // prepare to send request
    const char* ptr = request.c_str();
    int nleft = request.length();
    int nwritten;
    // loop to be sure it is all sent
    while (nleft) {
    	if(debug) {
    		cout << "Sent: " << endl;
    		cout << ptr << endl;
    	}
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

string
Client::get_response() {
    string response = "";
    response.append(cache);
    cache = "";
    // read until we get a newline
    while (response.find("\n") == string::npos) {
        int nread = recv(server_,buf_,1024,0);
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
        string tmp = "";
        tmp.append(buf_,nread);
        if(debug) {
        	cout << "Received: " << endl;
    		cout << tmp << endl;
    	}
        response.append(buf_,nread);
    }
    // cut off anything after the newline and save it in cache
    if(response.find("\n") != string::npos) {
      cache = response.substr(response.find("\n")+1);
      response = response.substr(0, response.find("\n"));
    }
    return response;
}

// send handling
string
Client::get_user_message() {
	cout << "- Type your message. End with a blank line -" << endl;
	string message = "";
	while(true) {
		string line = "";
		getline(cin, line);
		line += "\n";
		if(line == "\n") {
			message = message.substr(0, message.size()-1);
			return message;
		}
		message += line;
	}
}

void
Client::send_put(string name, string subject, string message) {
	// load request
	string request = "put ";
	request += name + " ";
	request += subject + " ";
	// convert message size to string
	int length = message.size();
	string str_length;
	ostringstream convert;
	convert << length;
	str_length = convert.str();
	// continue loading request
	request += str_length + "\n";
	request += message;
	send_request(request);
}

void
Client::respond_to_put() {
	string message = get_response();
	if(message != "OK") {
		cout << "Server returned bad message: " << message << endl;
		return;
	}
}

// list handling
void
Client::send_list(string name) {
	string request = "list ";
	request += name + "\n";
	send_request(request);
}

void
Client::respond_to_list() {
	string message = get_response();
	istringstream input(message);
	string command;
	int number;
	input >> command;
	if(command != "list") {
		cout << "Server returned bad message: " << message << endl;
		return;
	}
	if(input.good()) {
		input >> number;
	} else {
		cout << "Server returned bad message: " << message << endl;
		return;
	}
	read_list_response(number);
}

void
Client::read_list_response(int number) {
	int count = 0;
	while(count < number) {
		string message = get_response();
		cout << message << endl;
		count++;
	}
}

// read handling
void
Client::send_get(string name, string index) {
	string request = "get ";
	request += name + " ";
	request += index + "\n";
	send_request(request);
}

void
Client::respond_to_get() {
	string message = get_response();
	istringstream input(message);
	string command;
	string subject;
	int length;
	input >> command;
	if(command != "message") {
		cout << "Server returned bad message: " << message << endl;
	}
	input >> subject;
	if(subject.empty()) {
		cout << "Server returned bad message: " << message << endl;
	}
	if(input.good()) {
		input >> length;
	} else {
		cout << "Server returned bad message: " << message << endl;
	}
	read_message_response(subject, length);
}

void
Client::read_message_response(string subject, int length) {
	string message = "";
	message.append(cache);
	cache = "";
	if(message.size() < length) {
		length -= message.size();
		while(length > 0) {
			int nread = recv(server_,buf_,1024,0);
	        if (nread < 0) {
	            if (errno == EINTR)
	                // the socket call was interrupted -- try again
	                continue;
	            else
	                // an error occurred, so break out
	                return;
	        } else if (nread == 0) {
	            // the socket is closed
	            return;
	        }
	        string tmp = "";
        	tmp.append(buf_,nread);
        	if(debug) {
        		cout << "Received: " << endl;
    			cout << tmp << endl;
    		}
	        int old_length = length;
	        length -= nread;
	        if(length < 0) {
	        	cache.append(buf_+old_length, nread-old_length);
	        	message.append(buf_, nread+length);
	        } else {
	        	message.append(buf_,nread);
	        }
		}
		cout << subject << "\n" << message << endl;
	} else {
		cout << subject << "\n" << message.substr(0, length) << endl;
		cache = message.substr(length);
	}
}