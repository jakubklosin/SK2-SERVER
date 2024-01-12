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
        Socket hostSocket;
        std::string id;
        int answers = 0;
        bool isStarted=false;
    void gameStart(){
        isStarted = true;
    }
    void incermentAnswers(){
        answers++;
    }
     void resetAnswers(){
        answers=0;
    }
    json gameEnd(Game id){
        return json{ {"status", "end"} };
    }
    void gameNextRound5(Socket hostSocket){
        json data  ;
        data["status"]="nextRound5";
        hostSocket.writeData(data.dump());
        std::cout<<"wyslano dane do:"<<hostSocket.sock<<std::endl;
    }
    void gameNextRoundRN(Socket hostSocket){
        json data  ;
        data["status"]="nextRoundRN";
        hostSocket.writeData(data.dump());
        std::cout<<"wyslano dane do:"<<hostSocket.sock<<std::endl;
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
        if(fd == hostSocket.sock){
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
        hostSocket.sock = sock;
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
    usernames["usersJoined"]   = json::array();
    for (User* user : users) {
        usernames["usersJoined"].push_back(user->nickname);
    }
    return usernames;
    }

    void getGameInfo() const {
    std::cout << "Informacje o grze (ID: " << id << "):\n";
    std::cout << "Deskryptor Hosta: " << hostSocket.sock << std::endl;
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