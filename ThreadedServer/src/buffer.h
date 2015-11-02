#pragma once

#include <pthread.h>
#include <vector>
#include <map>
#include <iostream>
#include <stdio.h>
using namespace std;

#define NUM_THREADS 10

//------------------------------------------------------------------------------
class Buffer {
public:
	
	Buffer ();

	void append(int c);
	int take();

	bool find(int c) {
		return client_cache.find(c) != client_cache.end();
	}
	void set_cache(int c, string s) {
		map<int, string>::iterator it;
		it = client_cache.find(c);
		if (it == client_cache.end()) {
			client_cache.insert(pair<int, string>(c, s));
		} else {
			it->second = s;
		}
	}
	string get_cache(int c) {
		map<int, string>::iterator it;
		it = client_cache.find(c);
		if (it != client_cache.end()) {
			string cache = it->second;
			return cache;
		}
		return NULL;
	}
	void erase(int c) {
		if (client_cache.find(c) != client_cache.end()) {
			client_cache.erase(c);
		}
	}

	int size() {
		return buffer.size();
	}

	vector<int> buffer;
	map<int, string> client_cache;
	pthread_mutex_t lock;
	pthread_cond_t not_empty;
};
