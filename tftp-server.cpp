#include <iostream>
#include <getopt.h>
#include <string>
using namespace std;

int main(int argc, char **argv) {

    int port = 69;
    string root_dirpath = argv[argc-1];

    int c;

    while ((c = getopt (argc, argv, "p:")) != -1) {
        switch (c) {
            case 'p':
                port = atoi(optarg);
                break;
        }
    }



    cout << port << root_dirpath << endl;

    return 0;
}