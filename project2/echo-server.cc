#include <stdlib.h>
#include <unistd.h>
#include <iostream>

#include "server.h"

using namespace std;

void * threadHandle(void *arg);

//-----------------------------------------------------------------------------
int main(int argc, char **argv) {
	int option, port;
	pthread_t threads[NUM_THREADS];

	// setup default arguments
	port = 5000;

	// process command line options using getopt()
	// see "man 3 getopt"
	while ((option = getopt(argc, argv, "p:")) != -1) {
		switch (option) {
		case 'p':
			port = atoi(optarg);
			break;
		default:
			cout << "server [-p port]" << endl;
			exit(EXIT_FAILURE);
		}
	}

	Server server = Server(port);

	// PTHREADS ===============================================================
	for (int i = 0; i < NUM_THREADS; i++)
		pthread_create(&threads[i], NULL, &threadHandle, (void*) &server);

	server.run();

	for (int i = 0; i < NUM_THREADS; i++)
		pthread_join(threads[i], NULL);
}

// HANDLE THREADS =============================================================
void * threadHandle(void *arg) {
	Server *server = (Server*) arg;
	while (1) {
		pthread_mutex_lock(&server->lock);
		server->handle();
		pthread_mutex_unlock(&server->lock);
	}
	return NULL;
}

