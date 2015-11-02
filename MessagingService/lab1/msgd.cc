#include "msgd.h"
#include <iterator>

Server::Server(int port, bool debug_) {
    // setup variables
    port_ = port;
    buflen_ = 1024;
    buf_ = new char[buflen_+1];
    cache = "";
    debug = debug_;
}

Server::~Server() {
    delete buf_;
}

void
Server::run() {
    // create and run the server
    create();
    serve();
}

void
Server::create() {
    struct sockaddr_in server_addr;

    // setup socket address structure
    memset(&server_addr,0,sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port_);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // create socket
    server_ = socket(PF_INET,SOCK_STREAM,0);
    if (!server_) {
        perror("socket");
        exit(-1);
    }

    // set socket to immediately reuse port when the application closes
    int reuse = 1;
    if (setsockopt(server_, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        perror("setsockopt");
        exit(-1);
    }

    // call bind to associate the socket with our local address and
    // port
    if (bind(server_,(const struct sockaddr *)&server_addr,sizeof(server_addr)) < 0) {
        perror("bind");
        exit(-1);
    }

    // convert the socket to listen for incoming connections
    if (listen(server_,SOMAXCONN) < 0) {
        perror("listen");
        exit(-1);
    }
}

void
Server::close_socket() {
    close(server_);
}

void
Server::serve() {
    // setup client
    int client;
    struct sockaddr_in client_addr;
    socklen_t clientlen = sizeof(client_addr);

      // accept clients
    while ((client = accept(server_,(struct sockaddr *)&client_addr,&clientlen)) > 0) {

        handle(client);
    }
    close_socket();
}

void
Server::handle(int client) {
    // loop to handle all requests
    while (true) {
        // get a request
        string request = get_request(client);
        // break if client is done or an error occurred
        if (request.empty())
            break;
        // parse request
        string response = parse_request(client, request);
        // send response
        bool success = send_response(client, response);
        // break if an error occurred
        if (not success)
            break;
    }
    close(client);
}

string
Server::get_request(int client) {
    string request = "";
    request.append(cache);
    cache = "";
    // read until we get a newline
    while (request.find("\n") == string::npos) {
        int nread = recv(client,buf_,1024,0);

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
        string tmp = "";
        tmp.append(buf_,nread);
        if(debug) {
            cout << "Received:" << endl;
        	cout << tmp << endl;
        }
        // be sure to use append in case we have binary data
        request.append(buf_,nread);
    }
    // server cuts off anything after the newline and
    // saves it in a cache
    if(request.find("\n") != string::npos) {
      cache = request.substr(request.find("\n")+1);
      request = request.substr(0, request.find("\n"));
    }
    return request;
}

string
Server::parse_request(int client, string request) {
    string command;
    istringstream input(request);
    input >> command;
    if(command.empty()) {
        return "error invalid message\n";
    }
    if(command == "reset") {
        messages.clear();
        return "OK\n";
    }
    if(command == "put") {
        // handle put
        Message message;
        string name;
        string subject;
        int length;
        string value;
        bool needed = false;
        input >> name;
        input >> subject;
        if(name.empty() || subject.empty()) {
            return "error invalid message\n";
        }
        if(input.good()) {
            input >> length;
        } else {
            return "error invalid message\n";
        }
        if(cache.size() != length) {
            needed = true;
        }
        message.set_subject(subject);
        message.set_length(length);
        message.set_value(cache);
        cache = "";
        if(needed) {
            read_put(client, message);
        }
        if(message.get_value().empty() && length > 0) {
            return "error could not read entire message\n";
        }
        if(cache.size() > 0) {
            cache = "";
            return "error read more than needed for message\n";
        }
        store_message(name, message);
        return "OK\n";
    }
    if(command == "list") {
        // handle list
        string name;
        input >> name;
        if(name.empty()) {
            return "error invalid message\n";
        }
        vector<Message> user_messages = get_subjects(name);
        string response = "";
        response += command + " ";
        int size = user_messages.size();
        string str_size;
        ostringstream convert;
        convert << size;
        str_size = convert.str();
        response += str_size + "\n";
        for(int i=0; i<user_messages.size(); i++) {
            int index = i+1;
            string str_index;
            ostringstream index_convert;
            index_convert << index;
            str_index = index_convert.str();
            response += str_index + " ";
            response += user_messages[i].get_subject() + "\n";
        }
        return response;
    }
    if(command == "get") {
        // handle get
        string name;
        int index;
        input >> name;
        if(name.empty()) {
            return "error invalid message\n";
        }
        if(input.good()) {
            input >> index;
        } else {
            return "error invalid message\n";
        }
        Message message = get_message(name, index);
        string response = "";
        response += "message ";
        if(message.get_subject().empty()) {
            return "error invalid message\n";
        }
        response += message.get_subject() + " ";
        int length = message.get_length();;
        string str_length;
        ostringstream convert;
        convert << length;
        str_length = convert.str();
        response += str_length + "\n";
        if(message.get_value().empty()) {
            return "error invalid message\n";
        }
        response += message.get_value();
        return response;
    }
    return "error invalid message\n";
}

void
Server::read_put(int client, Message& message) {
    string value = message.get_value();
    int length = message.get_length();
    if(value.size() < length) {
        length -= value.size();
        while(length > 0) {
            int nread = recv(client,buf_,1024,0);
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
                cout << "Received:" << endl;
            	cout << tmp << endl;
    		}
            // be sure to use append in case we have binary data
            int old_length = length;
            length -= nread;
            if(length < 0) {
                cache.append(buf_+old_length, nread-old_length);
                value.append(buf_, nread+length);
            } else {
                value.append(buf_,nread);
            }
        }
        message.set_value(value);
    } else {
        message.set_value(value.substr(0, length));
        cache = value.substr(length);
    }
}

void
Server::store_message(string name, Message& message) {
    vector<Message> user_messages;
    for(map<string, vector<Message> >::iterator iter = messages.begin();
    iter != messages.end(); iter++) {
        if(name.compare((*iter).first) == 0) {
            user_messages = (*iter).second;
        }
    }
    user_messages.push_back(message);
    messages[name] = user_messages;
}

vector<Message>
Server::get_subjects(string name) {
    for(map<string, vector<Message> >::iterator iter = messages.begin(); 
    iter != messages.end(); iter++) {
        if(name.compare((*iter).first) == 0) {
            return (*iter).second;
        }
    }
    vector<Message> empty;
    return empty;
}

Message
Server::get_message(string name, int index) {
    vector<Message> user_messages;
    for(map<string, vector<Message> >::iterator iter = messages.begin();
    iter != messages.end(); iter++) {
        if(name.compare((*iter).first) == 0) {
            user_messages = (*iter).second;
        }
    }
    Message user_message;
    if(index <= user_messages.size() && index != 0) {
        user_message = user_messages[index-1];
    }
    return user_message;
}

bool
Server::send_response(int client, string response) {
    // prepare to send response
    const char* ptr = response.c_str();
    int nleft = response.length();
    int nwritten;
    // loop to be sure it is all sent
    while (nleft) {
    	if(debug) {
            cout << "Sent: " << endl;
        	cout << ptr << endl;
    	}
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
