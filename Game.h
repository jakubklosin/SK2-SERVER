#ifndef GAME_H
#define GAME_H

#include <nlohmann/json.hpp>
#include <vector>
#include <string>
#include <random>
#include <iostream>
#include "User.h"
#include "Socket.h"

using json = nlohmann::json;

class Game{
    public: 
        std::vector<json> questions;
        std::vector<json> shuffledQuestions;
        std::vector<User> users;
        int hostSocket;
        std::string id;
    json gameStart(){
        return json{ {"status", "start"} };
    }
    json gameEnd(Game id){
        return json{ {"status", "end"} };
    }
    json gameNextRound(Game id){
        return json{ {"status", "nextRound"} };
    }
    void createGame(const json &gameData, int hostFd){
        if(gameData.contains("pytania")){
            questions = gameData["pytania"].get<std::vector<json>>();
        }
        if(gameData.contains("kod pokoju")){
            id = gameData["kod pokoju"].get<std::string>();
        }
        addHost(hostFd);
    }
    void shuffle() {
        for(int i = 0; i<questions.size(); i++ ){
        //     for(int j = 0; j<4; j++ ){
        //     std::cout<<questions[i]["odpowiedzi"][j]<<std::endl;
        // }
        shuffledQuestions.push_back(shuffleAnswers(i));
        }
        questions = shuffledQuestions;
    }
    json shuffleAnswers(int i) {
        std::vector<std::string> odpowiedzi = questions[i]["odpowiedzi"].get<std::vector<std::string>>();
        std::string pytanie = questions[i]["pytanie"].get<std::string>();
        std::random_device rd;
        std::mt19937 g(rd());
        std::shuffle(odpowiedzi.begin(), odpowiedzi.end(), g);
        json output;
        output["odpowiedzi"] = odpowiedzi;
        output["pytanie"] = pytanie;

        return output;
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
                std::cout << "Użytkownik Socket ID: " << user.socket.sock << ", Nickname: " << user.nickname << std::endl;
            }
        }
};

#endif 