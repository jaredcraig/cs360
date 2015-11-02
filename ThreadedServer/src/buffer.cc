#include "buffer.h"

Buffer::Buffer() {
	 pthread_mutex_init(&lock, NULL);
	 pthread_cond_init(&not_empty, NULL);
}

void Buffer::append(int c) {
	pthread_mutex_lock (&lock);
		buffer.push_back(c);
		pthread_cond_signal(&not_empty);
	pthread_mutex_unlock (&lock);
}
int Buffer::take() {
	pthread_mutex_lock (&lock);
		while (buffer.size() == 0) {
			cout << pthread_self() << "\t<BUFFER> waiting for a client" << endl;
			pthread_cond_wait(&not_empty, &lock);
		}
		int c = buffer.at(0);
		buffer.erase(buffer.begin());
		cout << pthread_self() << "\t<BUFFER> taking client: " << c << endl;
	pthread_mutex_unlock (&lock);
	return c;
}
