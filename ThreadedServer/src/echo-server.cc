#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include "inet-server.h"
#include "buffer.h"

using namespace std;

Buffer b;
pthread_t threads[NUM_THREADS];

void *threadHandle(void*);

int main(int argc, char **argv) {
	int option, port;

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
			exit (EXIT_FAILURE);
		}
	}

	InetServer server = InetServer(port);
	server.run(&b);

	for (int i = 0; i < NUM_THREADS; i++) {
		int rc;
		rc = pthread_create(&threads[i], NULL, &threadHandle, &server);
		if (rc) {
			cerr << "Unable to create thread, " << rc << endl;
			exit(-1);
		}
	}

	server.serve();

	for (int i = 0; i < NUM_THREADS; i++) {
		pthread_join(threads[i], NULL);
	}
}

//-----------------------------------------------------------------------------
void *threadHandle(void *vptr) {
	Server *s = (Server*) vptr;
	do {
		pthread_mutex_lock(&s->server_lock);
			s->handle();
		pthread_mutex_unlock(&s->server_lock);
	} while (1);
}
