#include <SFML/Network.hpp>
#include <fstream>
#include <iostream>
#include <list>

std::string generateID() {
    return std::to_string(rand() % 100000);
}

void receiveFile(sf::TcpSocket* socket, std::ofstream& file) {
    sf::SocketSelector timeoutSelector;
    timeoutSelector.add(*socket);

    while (true)
    {
        char Buffer[1024];
        std::size_t Size = 0;
        if (timeoutSelector.wait(sf::seconds(3))) {
            socket->receive(Buffer, sizeof(Buffer), Size);
            if (Size >= 3 && Buffer[Size-1] == Buffer[Size-2] && Buffer[Size-2] == Buffer[Size-3] && Buffer[Size-3] == '\xFF') {
                std::cout << "Got to the end\n";
                file.write(Buffer, Size-3);
                break;
            }
            std::cout << "Writing...\n";
            file.write(Buffer, Size);
        }
        else {
            std::cout << "File timeout.\n";
            break;
        }
    }

    file.close();
}

int main() {
    srand(time(nullptr));
    std::cout << "Kiwi server v1.0-beta\n";
    std::vector<sf::TcpSocket*> sockets;

    int port = 8888;
    //std::cout << "Enter a port: ";
    //std::cin >> port;

    sf::TcpListener listener;
    sf::SocketSelector selector;

    if (listener.listen(port) != sf::Socket::Done) {
        std::cout << "Couldn't start server on " << port << '\n';
        exit(1);
    }
    std::cout << "Server started on " << port << '\n';
        
    selector.add(listener);

    while (true) {
        if (selector.wait()) {
            if (selector.isReady(listener)) {
                sf::TcpSocket* socket = new sf::TcpSocket;
                if (listener.accept(*socket) != sf::Socket::Done) {
                    std::cout << "Error while accepting new client\n";
                    delete socket;
                }
                else {
                    std::cout << "Adding new client\n";
                    sockets.push_back(socket);
                    selector.add(*socket);
                }
            }
            else {
                for (auto it = sockets.begin(); it != sockets.end(); it++) {
                    sf::TcpSocket* socket = *it;
                    if (selector.isReady(*socket)) {
                        sf::Packet packet;
                        sf::Socket::Status status = socket->receive(packet);
                        if (status == sf::Socket::Done) {
                            std::string request;
                            packet >> request;
                            std::cout << "Got request: " << request << '\n';

                            if (request == "uploadWav") {
                                std::string fileName;
                                packet >> fileName;
                                fileName = fileName.substr(0, fileName.find_last_of('.')) + "_" + std::to_string(time(nullptr)) + ".wav";
                                
                                std::ofstream file("Sounds/" + fileName, std::ofstream::binary);
                                receiveFile(socket, file);
                                packet.clear();
                                packet << "done";
                                socket->send(packet);
                            }
                            else if (request == "uploadKiwi") {
                                std::ifstream tester;
                                std::string fileName;
                                do {
                                    fileName = generateID();
                                    tester.open("Saves/" + fileName);
                                } while (tester);

                                std::ofstream file("Saves/" + fileName, std::ofstream::binary);
                                receiveFile(socket, file);
                                packet.clear();
                                packet << fileName;
                                socket->send(packet);
                            }

                        }
                        else if (status == sf::Socket::Disconnected) {
                            selector.remove(*socket);
                            delete socket;
                            sockets.erase(it);
                            std::cout << "Socket disconnected\n";
                            break;
                        }
                        else {
                            std::cout << "An error occured while trying to receive a packet\n";
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