#include "buffer.h"

//------------------------------------------------------------------------------
Buffer::Buffer() {
	pthread_mutex_init(&lock, NULL);
	pthread_cond_init(&not_empty, NULL);
}

//------------------------------------------------------------------------------
void Buffer::append(int c) {
	pthread_mutex_lock(&lock);
	buffer.push_back(c);
	setCache(c, "");
	pthread_cond_signal(&not_empty);
	pthread_mutex_unlock(&lock);
}

//------------------------------------------------------------------------------
int Buffer::take() {
	pthread_mutex_lock(&lock);
	while (buffer.empty())
		pthread_cond_wait(&not_empty, &lock);
	int client = buffer.front();
	buffer.erase(buffer.begin());
	pthread_mutex_unlock(&lock);
	return client;
}

//------------------------------------------------------------------------------
void Buffer::setCache(int c, string cache) {
	map<int, string>::iterator it = client_cache.find(c);
	if (it == client_cache.end()) {
		client_cache.insert(pair<int, string>(c, cache));
	} else {
		it->second = cache;
	}
}

//------------------------------------------------------------------------------
string Buffer::getCache(int c) {
	return client_cache.find(c)->second;
}
