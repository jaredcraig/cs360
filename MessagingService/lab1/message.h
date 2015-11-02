#pragma once

#include <string>

using namespace std;

class Message {

private:
	string subject;
	int length;
	string value;
    
public:
	Message(){};
	~Message(){};

	string get_subject() {
		return subject;
	}

	int get_length() {
		return length;
	}

	string get_value() {
		return value;
	}

	void set_subject(string subject_) {
		subject = subject_;
	}

	void set_length(int length_) {
		length = length_;
	}

	void set_value(string value_) {
		value = value_;
	}

};
