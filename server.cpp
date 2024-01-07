#include <iostream>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <nlohmann/json.hpp>
#include "Game.h"
#include "Socket.h"

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
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    if ((clientSocket.sock = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }
    
    std::unordered_map<std::string, Game> games;

    json j;
    j["action"] = "create";
    j["kod pokoju"] = "777";

    Game testGame;
    for (int i = 0; i < 5; ++i) {
        testGame.questions.push_back({{"pytanie", "przykladowe pytanie"}, {"odpowiedzi", {"A", "B", "C", "D"}}});
    }

    testGame.createGame(j,55);
    games[testGame.id] = testGame;

    try {
        while (true) {
            clientSocket.readData();
                
            json combinedJson; // Główny obiekt JSON do przechowywania połączonych danych
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
                    // Dodaj inne potrzebne informacje do responseJson
                } else if (action == "join") {
                    std::string id ;
                    if(combinedJson.contains("kod pokoju")){
                    id = combinedJson["kod pokoju"];
                    }
                    auto gameIter = games.find(id);
                    if (gameIter != games.end()) {
                        // Gra o danym ID została znaleziona w mapie
                        Game &foundGame = gameIter->second; // Referencja do znalezionej gry
                        // Możesz teraz wykonać operacje na znalezionej grze, np.:
                        foundGame.addUserToGame(clientSocket.sock);
                        questions = foundGame.getQuestions();
                        std::cout << questions<<std::endl;
                        foundGame.getGameInfo();
                    } else {
                        std::cout<<"nie znaleziono takiej gry"<<std::endl;
                    }
                    // Logika dołączania do gry
                     responseJson = questions;
                    // Dodaj inne potrzebne informacje do responseJson
                } else {
                    responseJson["status"] = "Nieznana akcja";
                }

                // Konwersja responseJson na string i wysyłanie
                std::string responseStr = responseJson.dump();
                
                //Wysyłanie wiadomosci
                clientSocket.writeData(responseStr);
            }

            clientSocket.message.clear(); // Czyszczenie listy po przetworzeniu
        }
    } catch (const char* msg) {
        std::cerr << "Error: " << msg << std::endl;
        clientSocket.closeSocket();
    }




    // // Tworzenie danych JSON
    // json j;
    // j["pytania"] = json::array({{"Pytanie 1", "Odpowiedz 1"}, {"Pytanie 2", "Odpowiedz 2"}});

    // // Wysyłanie danych JSON
    // std::string jsonStr = j.dump();
    // send(new_socket, jsonStr.c_str(), jsonStr.length(), 0);



    // Konwersja na string i deserializacja JSON
    // std::string jsonStr(buffer, bytesReceived);
    // json j = json::parse(jsonStr);

    // // Wyświetlanie odebranych danych
    // std::cout << "Odebrano dane: " << j.dump(4) << std::endl;

    close(server_fd);

    return 0;
}
