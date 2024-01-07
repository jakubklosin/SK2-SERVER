#ifndef GAME_H
#define GAME_H

#include <nlohmann/json.hpp>
#include <vector>
#include <string>
#include <random>
#include <iostream>
#include "User.h"
using json = nlohmann::json;

class Game{
    public: 
        std::vector<json> questions;
        std::vector<User> users;
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
    void addUserToGame(User user){
        users.push_back(user);
    }
    void addHost(int sock){
        hostSocket = sock;
    }
    json getQuestions() const {
            return json{ {"pytania", questions} };
        }
    void getGameInfo() const {
            std::cout << "Informacje o grze (ID: " << id << "):\n";
            std::cout << "Deskryptor Hosta: " << hostSocket <<std::endl;
            std::cout << "Pytania:\n";
            for (const auto& question : questions) {
                std::cout << question.dump() << std::endl;
            }

            std::cout << "Użytkownicy w grze:\n";
            for (User user: users) {
                std::cout << "Użytkownik Socket ID: " << user.socket.sock << std::endl;
            }
        }

};

#endif 