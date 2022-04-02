#include <vector>
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

#include "potato.h"

#define ADDR_SIZE 50

class Player {
public:
    int fd;
    int port;
    std::string addr;
};

int parsePlayersConnection(const char * port_num, int num_players, vector<Player> & players) {
    int fd = startServer(port_num);
    // upon connection with a player
    // output: Player + "num" + is ready to play\n
    int num = 0;
    for(int i = 0; i < num_players; i++) {
        Player * player = new Player;
        std::string addr;
        int player_fd = serverAcceptFd(fd, &addr);
        player->addr = addr;
        //
        send(player_fd, &num_players, sizeof(num_players), 0);
        send(player_fd, &num, sizeof(num), 0);
        int player_port;
        recv(player_fd, &player_port, sizeof(player_port), 0);

        player->fd = player_fd;
        player->port = player_port;
        players.push_back(*player);
        cout << "Player " << i << " is ready to play" << endl;
        num++;
    }
    return fd;
}

void playersConnectedinRing(int num_players, vector<Player> & players) {
    for(int i = 0; i < num_players; i++) {
        int neighbor_id = (i + 1) % num_players;
        int neighbor_port = players[neighbor_id].port;
        char neighbor_addr[ADDR_SIZE];
        memset(&neighbor_addr, 0, sizeof(neighbor_addr));
        int player_fd = players[i].fd;
        send(player_fd, &neighbor_port, sizeof(neighbor_port), 0);
        strcpy(neighbor_addr, players[neighbor_id].addr.c_str());
        send(player_fd, &neighbor_addr, sizeof(neighbor_addr), 0);
    }
}

int main(int argc, char * argv[]) {
    if (argc != 4) {
        cout << "Please input: ringmaster <port_num> <num_players> <num_hops>" << endl;
        return EXIT_FAILURE;
    }
    const char * port_num = argv[1];
    int num_players = atoi(argv[2]);
    int num_hops = atoi(argv[3]);
    // make sure to validate cmd line arg!
    if(num_players <= 1) {
        cout << "num_players must be greater than 1" << endl;
        return EXIT_FAILURE;
    }
    // [0, 512]
    if(num_hops < 0 || num_hops > 512) {
        cout << "num_hops must be greater than or equal to zero and less than or equal to 512" << endl;
        return EXIT_FAILURE;
    }
    // Initially
    cout << "Potato Ringmaster" << endl;
    cout << "Players = " << num_players << endl;
    cout << "Hops = " << num_hops << endl;

    // create a potato
    Potato potato;
    potato.num_hops = num_hops;
    vector<Player> players;

    // start server and upon connection with players
    // output: Player + "num" + is ready to play\n
    int fd = parsePlayersConnection(port_num, num_players, players);

    // connecting players in the ring
    playersConnectedinRing(num_players, players);

    if(potato.num_hops != 0) {
        // set random number
        srand((unsigned int)time(NULL)); //
        int random_start = rand() % num_players;
        send(players[random_start].fd, &potato, sizeof(potato), 0);
        cout << "Ready to start the game, sending potato to player " << random_start << endl;

        fd_set rfds;
        FD_ZERO(&rfds);
        int max = -1;

        for(int i = 0; i < num_players; i++) {
            FD_SET(players[i].fd, &rfds);
            if(players[i].fd > max) {
                max = players[i].fd;
            }
        }
        select(max + 1, &rfds, NULL, NULL, NULL);
        
        for(int i = 0; i < num_players; i++) {
            if(FD_ISSET(players[i].fd, &rfds) != 0) {
                recv(players[i].fd, &potato, sizeof(potato), 0);
                break;
            }
        }
    }

    for(int i = 0; i < num_players; i++) {
        if(send(players[i].fd, &potato, sizeof(potato), 0) != sizeof(potato)) {
            cerr << "Error: send potato with an error" << endl;
            return EXIT_FAILURE;
        }
    }
    // output the trace of potato
    potato.trace();

    for(int i = 0; i < num_players; i++) {
        close(players[i].fd);
    }
    close(fd);

    return EXIT_SUCCESS;
}
