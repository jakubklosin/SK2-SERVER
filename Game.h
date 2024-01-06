#ifndef GAME_H
#define GAME_H

#include <nlohmann/json.hpp>
#include <vector>
#include <string>
#include <random>
#include <iostream>

using json = nlohmann::json;

class Game{
    private: 
        std::vector<json> questions;
        std::vector<int> userSockets;
        int hostSocket;
    public:
        std::string id;


    void createGame(const json &gameData, int hostFd){
        if(gameData.contains("pytania")){
            questions = gameData["pytania"].get<std::vector<json>>();
        }
        if(gameData.contains("kod pokoju")){
            id = gameData["kod pokoju"].get<std::string>();
        }
        addHost(hostFd);
    }
    void addUserToGame(int userSocket){
        userSockets.push_back(userSocket);
    }
    void addHost(int sock){
        hostSocket = sock;
    }

    void getGameInfo() const {
            std::cout << "Informacje o grze (ID: " << id << "):\n";
            std::cout << "Deskryptor Hosta: " << hostSocket <<std::endl;
            std::cout << "Pytania:\n";
            for (const auto& question : questions) {
                std::cout << question.dump() << std::endl;
            }

            std::cout << "Użytkownicy w grze:\n";
            for (int socket : userSockets) {
                std::cout << "Użytkownik Socket ID: " << socket << std::endl;
            }
        }

};

#endif 