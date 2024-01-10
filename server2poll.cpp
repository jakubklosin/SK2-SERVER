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

std::string getGameIdForClient( int deskryptor,const std::unordered_map<std::string, Game>& games);

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

    json j;
    j["action"] = "create";
    j["kod pokoju"] = "777";

    Game testGame;
    for (int i = 0; i < 5; ++i) {
        testGame.questions.push_back({{"pytanie", "przykladowe pytanie"}, {"odpowiedzi", {"A", "B", "C", "D"}}});
    }
    testGame.createGame(j,55);
    
    testGame.shuffle(); 
    testGame.getGameInfo(); 
    games[testGame.id] = testGame;

    std::vector<struct pollfd> fds;
    fds.push_back({server_fd, POLLIN, 0});
    try
    {
    while (true) {
        int ret = poll(fds.data(), fds.size(), -1);
        if (ret < 0) {
            perror("poll");
            break;
        }
        for (auto &fd : fds) {
            // if (fd.revents & POLLHUP){
            //             close(fd.fd);
            //             fd.fd =-1;
            //             std::cout<<"Uzytkownik o deskryptorze: "<<fd.fd << " rozlaczyl sie"<<std::endl;
            //             continue;
            //         } 
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
                    try
                    {
                    clientSocket.readData();
                    }
                    catch (const char* msg) {
                    // std::cerr << "Wyjątek char const*: " << msg << '\n';

                    for (auto& pair: games) {
                        Game& game = pair.second;
                        bool host = game.isHost(clientSocket.sock);
                        std::cout << game.hostSocket<< " " << game.id <<" "<< clientSocket.sock<< std::endl;
                        if(host){
                            std::cout<<"Rozlaczony gracz byl hostem, zamykanie gry"<<std::endl;
                        } 
                        host = false;
                    }    
                    std::cout<<"Uzytkownik o deskryptorze: "<<fd.fd << " rozlaczyl sie"<<std::endl;
                    close(fd.fd);
                    fd.fd =-1;
                    continue;
                    } 
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
                    // std::cout << combinedJson.dump()<<std::endl;

                    if (!combinedJson.empty() && combinedJson.contains("action")) {
                        std::string action = combinedJson["action"];
                        // std::cout << "Akcja: " << action << std::endl;

                        json responseJson; // Obiekt JSON do wysłania odpowiedzi
                        json questions;
                        if (action == "create") {
                            // Logika tworzenia gry
                            Game newGame;

                            newGame.createGame(combinedJson, clientSocket.sock); // Tworzenie gry z otrzymanych danych
                            games[newGame.id] = newGame;
                            newGame.addHost(clientSocket.sock);
                            // newGame.getGameInfo(); 
                            std::cout<<"Host o dekryptorze: "<< clientSocket.sock << " utworzyl gre o id: "<< newGame.id<<std::endl;
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
                                if (combinedJson.contains("nickname")) {
                                    newUser.setNickname(combinedJson["nickname"]);
                                }
                                foundGame.addUserToGame(newUser);
                                foundGame.shuffle();
                                questions = foundGame.getQuestions();
                                responseJson = questions;
                                std::cout<<"Gracz o id: "<< clientSocket.sock<<" dolaczyl do gry o id: "<<foundGame.id<<std::endl;
                                // std::cout << questions<<std::endl;
                                // foundGame.getGameInfo();
                            } else {
                                std::cout<<"nie znaleziono takiej gry"<<std::endl;
                            }
                        }else if(action == "answering"){
                            std::string gameId = getGameIdForClient(clientSocket.sock, games);
                            std::cout<<"gracz o deskryptorze "<<clientSocket.sock<<" przesyla odpowiedz do gry o id: "<<gameId<<std::endl;
                            std::cout<<combinedJson.dump()<< std::endl;
                        } else {
                            responseJson["status"] = "Nieznana akcja";
                        }

                        std::string responseStr = responseJson.dump();
                        
                        clientSocket.writeData(responseStr);
                    }
                    clientSocket.message.clear(); // Czyszczenie listy po przetworzeniu
                }
                // clientSocket.message.clear();  
            } 
            
        // Usuń zamknięte połączenia
        fds.erase(std::remove_if(fds.begin(), fds.end(), [](const struct pollfd &pfd) { return pfd.fd == -1; }), fds.end());
        }

    }
    }catch (const char* msg) {
    std::cerr << "Wyjątek char const*: " << msg << '\n';
    }   


    for (auto &fd : fds) {
        if (fd.fd >= 0) close(fd.fd);
    }

    close(server_fd);
    return 0;
}

std::string getGameIdForClient(int deskryptor, const std::unordered_map<std::string, Game>& games) {
    for (const auto& match : games) {
        const Game& game = match.second;
        for (const auto& uzytkownik : game.users) {
            if (uzytkownik.socket.sock == deskryptor) {
                return game.id;
            }
        }
    }
    return ""; 
}
 
 