#include <SFML/Network.hpp>
#include <iostream>
#include <list>

int generateID(std::list< std::pair<int, sf::TcpSocket*> >& clients) {
    bool matchFound = false;
    int id;
    do {
        matchFound = false;
        id = rand() % 10000;
        for (auto it = clients.begin(); it != clients.end(); it++) {
            if (it->first == id) {
                matchFound = true;
                break;
            }
        }
    }
    while (matchFound);

    return id;
}

int main() {
    srand(time(nullptr));
    std::cout << "Kiwi server v0.1\n";

    sf::TcpListener listener;
    std::list< std::pair< int, sf::TcpSocket* > > clients;

    sf::SocketSelector selector;

    if (listener.listen(25565) != sf::Socket::Done) {
        std::cout << "Failed to open port\n";
        exit(1);
    }

    selector.add(listener);

    while (true) {
        if (selector.wait()) {
            if (selector.isReady(listener)) {
                sf::TcpSocket* newClient = new sf::TcpSocket;
                if (listener.accept(*newClient) != sf::Socket::Done) {
                    std::cout << "Error while accepting new client\n";
                }
                else {
                    int id = generateID(clients);
                    std::cout << "Adding new client (" << id << ")\n";
                    clients.push_back(std::make_pair(id, newClient));
                    selector.add(*newClient);
                }
            }
            else {
                for (auto it = clients.begin(); it != clients.end(); it++) {
                    if (selector.isReady(*it->second)) {
                        sf::Packet packet, packet_copy;
                        if (it->second->receive(packet) == sf::Socket::Done)
                        {
                            packet_copy = packet;
                            std::string msg;
                            packet_copy >> msg;
                            std::cout << "Got message: " << msg << '\n';
                            if (msg == "disconnect") {
                                selector.remove(*it->second);
                                clients.erase(it);
                                continue;
                            }

                            int idToExclude = it->first;
                            for (auto it2 = clients.begin(); it2 != clients.end(); it2++) {
                                if (it2->first != idToExclude) {
                                    if (it2->second->send(packet) != sf::Socket::Done) {
                                        std::cout << "Sending to " << it2->first << " failed\n";
                                    }
                                }
                            }
                        }
                    }
                }
            }

        }
        else {
            std::cout << "Some error occured while waiting\n";
        }
    }
}