#include <stdlib.h>
#include <unistd.h>

#include <iostream>

#include "msg.h"

using namespace std;

int
main(int argc, char **argv)
{
    int option;

    // setup default arguments
    int port = 5000;
    string host = "localhost";
    bool debug = false;

    // process command line options using getopt()
    // see "man 3 getopt"
    while ((option = getopt(argc,argv,"s:p:d")) != -1) {
        switch (option) {
        	case 's':
                host = optarg;
                break;
            case 'p':
                port = atoi(optarg);
                break;
            case 'd':
            	debug = true;
            	break;
            default:
                cout << "client [-h host] [-p port] [-d]" << endl;
                exit(EXIT_FAILURE);
        }
    }

    Client client = Client(host, port, debug);
    client.run();
}

