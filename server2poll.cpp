#include <iostream>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <nlohmann/json.hpp>
#include "Game.h"
#include "Socket.h"
#include "User.h"
#include  <poll.h>
using json = nlohmann::json;

int main() {
    int server_fd;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    Socket clientSocket;

    // Tworzenie gniazda serwera
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

     if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt SO_REUSEADDR failed");
        exit(EXIT_FAILURE);
    }
    // Przypisywanie adresu do gniazda
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(5555);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 10) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    int flags = fcntl(server_fd, F_GETFL, 0);
    if (flags == -1) {
        perror("fcntl F_GETFL");
        exit(EXIT_FAILURE);
    }
    if (fcntl(server_fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        perror("fcntl F_SETFL O_NONBLOCK");
        exit(EXIT_FAILURE);
    }

    std::unordered_map<int, Socket> socketMap;
    std::unordered_map<std::string, Game> games;

    std::vector<struct pollfd> fds;
    fds.push_back({server_fd, POLLIN, 0});

    while (true) {
        int ret = poll(fds.data(), fds.size(), -1);
        if (ret < 0) {
            perror("poll");
            break;
        }

        for (auto &fd : fds) {
            if (fd.revents & POLLIN) {
                if (fd.fd == server_fd) {
                    // Akceptacja nowego połączenia
                    int new_socket = accept(server_fd, nullptr, nullptr);
                    if (new_socket < 0) {
                        perror("accept");
                        continue;
                    }else{
                        int flags = fcntl(new_socket, F_GETFL, 0);
                        if (flags == -1) {
                            perror("fcntl F_GETFL");
                            exit(EXIT_FAILURE);
                        }
                        if (fcntl(new_socket, F_SETFL, flags | O_NONBLOCK) == -1) {
                            perror("fcntl F_SETFL O_NONBLOCK");
                            exit(EXIT_FAILURE);
                        }
                        Socket newSocket;
                        newSocket.sock = new_socket;
                        socketMap[new_socket] = newSocket;
                    }

                    fds.push_back({new_socket, POLLIN, 0});
                } else {
                    // Obsługa danych od klienta
                    Socket& clientSocket = socketMap[fd.fd];
                    clientSocket.readData();
                    json combinedJson;  
                    for (auto& message : clientSocket.message) {
                        try {
                            json j = json::parse(message);
                            for (auto& [key, value] : j.items()) {
                                combinedJson[key] = value;
                            }
                        } catch (const json::exception& e) {
                            std::cerr << "Błąd parsowania JSON: " << e.what() << '\n';
                        }
                    }
                    std::cout << combinedJson.dump()<<std::endl;

                    if (!combinedJson.empty() && combinedJson.contains("action")) {
                std::string action = combinedJson["action"];
                std::cout << "Akcja: " << action << std::endl;

                json responseJson; // Obiekt JSON do wysłania odpowiedzi
                json questions;
                if (action == "create") {
                    // Logika tworzenia gry
                    Game newGame;

                    newGame.createGame(combinedJson, clientSocket.sock); // Tworzenie gry z otrzymanych danych
                    games[newGame.id] = newGame;
                    newGame.getGameInfo(); 
                    responseJson["status"] = "Gra utworzona";
                } else if (action == "join") {
                    std::string id ;
                    if(combinedJson.contains("kod pokoju")){
                    id = combinedJson["kod pokoju"];
                    }
                    auto gameIter = games.find(id);
                    if (gameIter != games.end()) {
                        // Gra o danym ID została znaleziona w mapie
                        Game &foundGame = gameIter->second; // Referencja do znalezionej gry

                        User newUser;
                        newUser.socket = clientSocket;
                        newUser.socket = clientSocket;
                        if (combinedJson.contains("nickname")) {
                            newUser.setNickname(combinedJson["nickname"]);
                        }
                        foundGame.addUserToGame(newUser);

                        foundGame.addUserToGame(newUser);
                        questions = foundGame.getQuestions();
                        // std::cout << questions<<std::endl;
                        foundGame.getGameInfo();
                    } else {
                        std::cout<<"nie znaleziono takiej gry"<<std::endl;
                    }
                     responseJson = questions;
                    } else {
                        responseJson["status"] = "Nieznana akcja";
                    }

                    std::string responseStr = responseJson.dump();
                    
                    clientSocket.writeData(responseStr);
                }
                clientSocket.message.clear(); // Czyszczenie listy po przetworzeniu
                }
            } 
            
        }

        // Usuń zamknięte połączenia
        fds.erase(std::remove_if(fds.begin(), fds.end(), [](const struct pollfd &pfd) { return pfd.fd == -1; }), fds.end());
    }

    for (auto &fd : fds) {
        if (fd.fd >= 0) close(fd.fd);
    }

    close(server_fd);
    return 0;
}
