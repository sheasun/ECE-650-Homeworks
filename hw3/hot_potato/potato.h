#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <string.h>
#include <cstring>
#include <string.h>
#include <iostream>
#include <vector>

#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define MAX_HOPS 512

using namespace std;

class Potato {
public:
    int num_hops;  // [0, 512]
    int count;
    int path[MAX_HOPS];
    
    Potato(): num_hops(0), count(0) {
        memset(&path, 0, sizeof(path));
    }
    Potato(const Potato &rhs):  num_hops(rhs.num_hops), count(rhs.count) {
        memset(&path, 0, sizeof(path));
    }
    ~Potato() {};
    void trace() {
        cout << "Trace of potato:" << endl;
        for(int i = 0; i < count; i++) {
            cout << path[i];
            if(i == count - 1) {
                cout << endl;
            }
            else {
                cout << ",";
            }
        }
    }
    void updatePotato(int id) {
        num_hops--;
        path[count] = id;
        count++;
    }
};


int startServer(const char * port) {
    int status;
    int socket_fd;
    struct addrinfo host_info;
    struct addrinfo * host_info_list;
    const char * hostname = NULL;

    memset(&host_info, 0, sizeof(host_info));

    host_info.ai_family   = AF_UNSPEC;
    host_info.ai_socktype = SOCK_STREAM;
    host_info.ai_flags    = AI_PASSIVE;

    status = getaddrinfo(hostname, port, &host_info, &host_info_list);
    if (status != 0) {
        cerr << "Error: cannot get address info for host" << endl;
        cerr << "  (" << hostname << "," << port << ")" << endl;
        return -1;
    } //if

    socket_fd = socket(host_info_list->ai_family, 
		     host_info_list->ai_socktype, 
		     host_info_list->ai_protocol);
    if (socket_fd == -1) {
        cerr << "Error: cannot create socket" << endl;
        cerr << "  (" << hostname << "," << port << ")" << endl;
        return -1;
    } //if

    int yes = 1;
    status = setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    status = bind(socket_fd, host_info_list->ai_addr, host_info_list->ai_addrlen);
    if (status == -1) {
        cerr << "Error: cannot bind socket" << endl;
        cerr << "  (" << hostname << "," << port << ")" << endl;
        return -1;
    } //if

    status = listen(socket_fd, 100);
    if (status == -1) {
        cerr << "Error: cannot listen on socket" << endl; 
        cerr << "  (" << hostname << "," << port << ")" << endl;
        return -1;
    } //if

    freeaddrinfo(host_info_list);
    return socket_fd;
}

int serverAcceptFd(int socket_fd, std::string * ip_addr) {
    struct sockaddr_storage socket_addr;
    socklen_t socket_addr_len = sizeof(socket_addr);
    int client_connection_fd;
    client_connection_fd = accept(socket_fd, (struct sockaddr *)&socket_addr, &socket_addr_len);
    if (client_connection_fd == -1) {
        cerr << "Error: cannot accept connection on socket" << endl;
        return -1;
    } //if

    struct sockaddr_in * addr = (struct sockaddr_in *)&socket_addr;
    *ip_addr = inet_ntoa(addr->sin_addr);
    return client_connection_fd;
}

int startClient(const char * hostname, const char * port) {
    int status;
    int socket_fd;
    struct addrinfo host_info;
    struct addrinfo * host_info_list;
    //const char *hostname = argv[1];
    //const char *port     = "4444";
    
    memset(&host_info, 0, sizeof(host_info));
    host_info.ai_family   = AF_UNSPEC;
    host_info.ai_socktype = SOCK_STREAM;

    status = getaddrinfo(hostname, port, &host_info, &host_info_list);
    if (status != 0) {
        cerr << "Error: cannot get address info for host" << endl;
        cerr << "  (" << hostname << "," << port << ")" << endl;
        return -1;
    } //if

    socket_fd = socket(host_info_list->ai_family, 
                host_info_list->ai_socktype, 
                host_info_list->ai_protocol);
    if (socket_fd == -1) {
        cerr << "Error: cannot create socket" << endl;
        cerr << "  (" << hostname << "," << port << ")" << endl;
        return -1;
    } //if

    status = connect(socket_fd, host_info_list->ai_addr, host_info_list->ai_addrlen);
    if (status == -1) {
        cerr << "Error: cannot connect to socket" << endl;
        cerr << "  (" << hostname << "," << port << ")" << endl;
        return -1;
    } //if

    freeaddrinfo(host_info_list);
    return socket_fd;
}