#pragma once

#include <pthread.h>
#include <vector>
#include <map>
#include <string>

using namespace std;

//------------------------------------------------------------------------------
class Buffer {
public:
	Buffer ();
	void append(int c);
	int take();
	void setCache(int c, string cache);
	string getCache(int c);

private:
	map<int, string> client_cache;
	vector<int> buffer;
	pthread_mutex_t lock;
	pthread_cond_t not_empty;
};
