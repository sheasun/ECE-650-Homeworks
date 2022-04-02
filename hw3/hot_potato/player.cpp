#include <cstdio>
#include <cstdlib>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>
#include <vector>

#include "potato.h"

#define NUM 3
#define PORT ""
#define PORT_SIZE 10
#define ADDR_SIZE 50

class Player {
public:
    int id;
    int fd;
    int port;
};

int getPort(int socket_fd) {
  struct sockaddr_in addr;
  socklen_t len = sizeof(addr);
  if (getsockname(socket_fd, (struct sockaddr *)&addr, &len) == -1) {
    cerr << "Error: cannot get sockname" << endl;
    return EXIT_FAILURE;
  }
  return ntohs(addr.sin_port);
}

int main(int argc, char * argv[]) {
    if (argc != 3) {
        cout << "Please input: player <machine_name> <port_num>" << endl;
        return EXIT_FAILURE;
    }
    const char * machine_name = argv[1];
    const char * port_num = argv[2];
    int connections[NUM];
    
    // player connections
    int master_fd = startClient(machine_name, port_num);
    //
    int num_players;
    recv(master_fd, &num_players, sizeof(num_players), MSG_WAITALL);
    // current player's id
    int player_id;
    recv(master_fd, &player_id, sizeof(player_id), MSG_WAITALL);


    int player_fd = startServer(PORT);
    int player_port = getPort(player_fd);
    send(master_fd, &player_port, sizeof(player_port), 0);
    cout << "Connected as player " << player_id << " out of " << num_players << " total players";
    cout << endl;

    int right_port_int;
    recv(master_fd, &right_port_int, sizeof(right_port_int), MSG_WAITALL);
    // translate its format
    //itoa(right_port_int, right_port, 10);
    vector<char> right_port(PORT_SIZE);
    sprintf(right_port.data(), "%d", right_port_int);
    vector<char> right_addr(ADDR_SIZE);
    recv(master_fd, &right_addr.data()[0], right_addr.size(), MSG_WAITALL);
    connections[0] = master_fd;

    int right_id = (player_id + 1) % num_players;
    int right_fd = startClient(right_addr.data(), right_port.data());
    connections[1] = right_fd;

//

    int left_id = (player_id - 1 + num_players) % num_players;
    std::string left_addr;
    int left_fd = serverAcceptFd(player_fd, &left_addr);
    connections[2] = left_fd;

    Potato potato;
    while(1) {
        fd_set readfds;
        FD_ZERO(&readfds);

        int max = -1;
        for (int i = 0; i < NUM; i++) {
            if (connections[i] > max) {
                max = connections[i];
            }
        }
        for(int i = 0; i < NUM; i++) {
            FD_SET(connections[i], &readfds);
        }
        select(max + 1, &readfds, NULL, NULL, NULL);

        for(int i = 0; i < NUM; i++) {
            if(FD_ISSET(connections[i], &readfds) != 0) {
                recv(connections[i], &potato, sizeof(potato), MSG_WAITALL);
                break;
            }
        }
        if(potato.num_hops == 0) {
            break;
        }
        else {
            potato.updatePotato(player_id);
            if (potato.num_hops == 0) {
                send(master_fd, &potato, sizeof(potato), 0);
                cout << "I'm it" << endl;
            }
            else {
                // set random number
                srand((unsigned int)time(NULL) + player_id);
                int random = rand() % 2;
                int choice_fd;
                int choice_id;
                if (random == 0) {
                    choice_fd = left_fd;
                    choice_id = left_id;
                }
                else {
                    choice_fd = right_fd;
                    choice_id = right_id;
                }
                send(choice_fd, &potato, sizeof(potato), 0);
                cout << "Sending potato to " << choice_id << endl;;
            }
        }
    }

    close(master_fd);
    close(left_fd);
    close(right_fd);

    return EXIT_SUCCESS;
}
