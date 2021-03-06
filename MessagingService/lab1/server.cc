#include <stdlib.h>
#include <unistd.h>

#include <iostream>

#include "msgd.h"

using namespace std;

int
main(int argc, char **argv)
{
    int option, port;
    bool debug;

    // setup default arguments
    port = 5000;
    debug = false;

    // process command line options using getopt()
    // see "man 3 getopt"
    while ((option = getopt(argc,argv,"p:d")) != -1) {
        switch (option) {
            case 'p':
                port = atoi(optarg);
                break;
            case 'd':
                debug = true;
                break;
            default:
                cout << "server [-p port] [-d]" << endl;
                exit(EXIT_FAILURE);
        }
    }

    Server server = Server(port, debug);
    server.run();
}
