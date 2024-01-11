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
        std::vector<User*> users;
        int hostSocket;
        std::string id;
        bool isStarted=false;
    void gameStart(){
        isStarted = true;
    }
    json gameEnd(Game id){
        return json{ {"status", "end"} };
    }
    json gameNextRound(Game id){
        return json{ {"status", "nextRound"} };
    }
    void sendToAllClients(std::string message){
        for (User* user : users) {
            user->socket.writeData(message);
        }
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
    bool isHost(int fd){
        if(fd == hostSocket){
            return true;
        }else{
            return false;
        }
    }
    void shuffle() {
        std::vector<json> shuffledQuestions;
        for(int i = 0; i<questions.size(); i++ ){
        shuffledQuestions.push_back(shuffleAnswers(i));
        }
        questions = shuffledQuestions;
    }
    json shuffleAnswers(int i) {
        std::vector<json> odpowiedzi = questions[i]["odpowiedzi"].get<std::vector<json>>();
        std::string pytanie = questions[i]["pytanie"].get<std::string>();
        std::random_device rd;
        std::mt19937 g(rd());
        std::shuffle(odpowiedzi.begin(), odpowiedzi.end(), g);
        json output;
        output["odpowiedzi"] = odpowiedzi;
        output["pytanie"] = pytanie;

        return output;
    }
   
    void addUserToGame(User* user){
        users.push_back(user);
    }
    void addHost(int sock){
        hostSocket = sock;
    }
    json getQuestions() const {
        return json{ {"pytania", questions} };
    }
    json getScoreboard() {
    json scoreboard;
    scoreboard["data"] = "scoreboard";
    scoreboard["users"] = json::array();
    for (User* user : users) {
        json userJson;
        userJson["user"] = user->nickname;
        userJson["score"] = user->score;
        scoreboard["users"].push_back(userJson);
    }
    return scoreboard;
    }
    json getUsers() {
    json usernames;
    usernames["usersJoined"] = "users";
    usernames["users"] = json::array();
    for (User* user : users) {
        json userJson;
        userJson["user"] = user->nickname;
        usernames["users"].push_back(userJson);
    }
    return usernames;
    }

    void getGameInfo() const {
    std::cout << "Informacje o grze (ID: " << id << "):\n";
    std::cout << "Deskryptor Hosta: " << hostSocket << std::endl;
    std::cout << "Pytania:\n";
    for (const auto& question : questions) {
        std::cout << question.dump() << std::endl;
    }

    std::cout << "Użytkownicy w grze:\n";
    for (User* user : users) {
        std::cout << "Użytkownik Socket ID: " << user->socket.sock << ", Nickname: " << user->nickname << "punkty: " << user->score << std::endl;
    }
    }

};

#endif 