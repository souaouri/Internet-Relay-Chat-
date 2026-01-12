#include "../inc/Server.hpp"

//********************//
//        BOT         //
//********************//

void Server::checkChallengeCompletion(Challenge& challenge) {
    if (challenge.challenger_choice.empty() || challenge.target_choice.empty()) {
        return;
    }

    Channel* channel = _channelManager.search_for_channel(challenge.channel);
    if (!channel) return;

    std::string result;
    if (challenge.challenger_choice == challenge.target_choice) {
        result = "Draw! Both chose " + challenge.challenger_choice;
    } else if (
        (challenge.challenger_choice == "rock" && challenge.target_choice == "scissors") ||
        (challenge.challenger_choice == "paper" && challenge.target_choice == "rock") ||
        (challenge.challenger_choice == "scissors" && challenge.target_choice == "paper")
    ) {
        result = challenge.challenger + " wins! " + challenge.challenger_choice + 
                " beats " + challenge.target_choice;
    } else {
        result = challenge.target + " wins! " + challenge.target_choice + 
                " beats " + challenge.challenger_choice;
    }

    channel->broadcast(_NOTICE(channel->getName(), "RPS Result: " + result), "");
    activeChallenges.erase(challenge.challenger);
}


void Server::checkChallengeTimeouts() {
    time_t now = time(NULL);
    std::vector<std::string> to_remove;

    std::map<std::string, Challenge>::iterator it = activeChallenges.begin();
    for (; it != activeChallenges.end(); ++it) {
        const std::string& challenger = it->first;
        const Challenge& challenge = it->second;

        if (now - challenge.created_at > 120) {
            to_remove.push_back(challenger);
            Channel* channel = _channelManager.search_for_channel(challenge.channel);
            if (channel)
                channel->broadcast(_NOTICE(channel->getName(), "RPS challenge from " + challenger + " timed out"), "");
        }
    }

    std::vector<std::string>::iterator vIt = to_remove.begin();
    for (; vIt != to_remove.end(); vIt++) {
        activeChallenges.erase(*vIt);
    }
}

void Server::handleChoose(Client* client, const std::vector<std::string>& params) {
    checkChallengeTimeouts();
    if (params.size() < 2) {
        sendReplay(client->getFd(), _NOTICE(client->getNickName(), "Syntax: /choose <rock|paper|scissors> <channel>"));
        return;
    }
    
    std::string choice = params[0];
    std::string channelName = params[1];
    std::transform(choice.begin(), choice.end(), choice.begin(), ::tolower);

    // check for valid choices
    if (choice != "rock" && choice != "paper" && choice != "scissors") {
        sendReplay(client->getFd(), _NOTICE(client->getNickName(), "Invalid choice. Valid options: rock, paper, scissors"));
        return;
    }
    // both opponents choices indicator
    bool found = false;
    std::map<std::string, Challenge>::iterator it = activeChallenges.begin();
    // search for active challenges
    for (; it != activeChallenges.end(); it++) {
        // make sure we are in the correct channel
        if (it->second.channel != channelName) continue;
        // if  the chooser is the challenger
        if (client->getNickName() == it->second.challenger) {
            it->second.challenger_choice = choice;
            found = true;
            //if the target not choose yet
			if (it->second.target_choice.empty()) {
                // search for target
                Client* target = findClientByNickname(it->second.target);
                //if the target found :  his turn 
                if (target)
                    sendReplay(target->getFd(), _NOTICE(target->getNickName(), "[RPS] " + it->second.challenger + " has chosen. Your turn!"));
            }
        // if  the chooser is the target
        } else if (client->getNickName() == it->second.target) {
            if (!it->second.accepted) {
                sendReplay(client->getFd(), _NOTICE(client->getNickName(), "You must accept the challenge before choosing"));
                return;
            }
            it->second.target_choice = choice;
            found = true;
			if (it->second.challenger_choice.empty()) {
    			Client* challenger = findClientByNickname(it->second.challenger);
    		    if (challenger)
                    sendReplay(challenger->getFd(), _NOTICE(challenger->getNickName(), it->second.target + " has chosen. Your turn!"));
			}
        }
        if (found) {
            checkChallengeCompletion(it->second);
            break;
        }
    }
    if (!found)
        sendReplay(client->getFd(), _NOTICE(client->getNickName(), "No active RPS challenge in " + channelName));
}


void Server::handleAccept(Client* client) {
    //for not wasting server perfermonce
    checkChallengeTimeouts();
    //active challenges container
    std::map<std::string, Challenge>::iterator it = activeChallenges.begin();
    //iterate through active chalenges
    for (; it != activeChallenges.end(); ++it) {
        //if the client have a pending challenge
        if (it->second.target == client->getNickName()) {
            it->second.accepted = true;

            Channel * channel = _channelManager.search_for_channel(it->second.channel);
            //making sure channel exist and broadcast the battle.
            if (channel)
                channel->broadcast(_NOTICE(channel->getName(), client->getNickName() + " accepted the RPS challenge!"), "");
            return;
        }
    }
    //if the client has no pending challenges "chill",
    sendReplay(client->getFd(), _NOTICE(client->getNickName(), "No pending challenges to accept"));
}

void Server::handleRPS(Client* client, const std::vector<std::string>& params) {
    //making sure that we have the target and the channel where the challenge is gonna happen
    if (params.size() < 2) {
        sendReplay(client->getFd(), _NOTICE(client->getNickName(), " :Syntax: /rps <nickname> <channel>\r\n"));
        return;
    }

    std::string targetNick = params[0];
    std::string channelName = params[1];
    
    //making sure the challenger is not the target itself
    if (client->getNickName() == targetNick) {
        sendReplay(client->getFd(), _NOTICE(client->getNickName(), "You can't challenge yourself"));
        return;
    }
    //making sure the target exists
    Client* target = findClientByNickname(targetNick);
    if (!target) {
        sendReplay(client->getFd(), _NOTICE(client->getNickName(), "User " + targetNick + " not found"));
        return;
    }
    //making sure the channel exists
    Channel* channel = _channelManager.search_for_channel(channelName);
    if (!channel) {
        sendReplay(client->getFd(), _NOTICE(client->getNickName(), "Channel " + channelName + " doesn't exist"));
        return;
    }
    //create a challenge instance
    Challenge newChallenge;
    newChallenge.challenger = client->getNickName();
    newChallenge.target = targetNick;
    newChallenge.challenger_choice = "";
    newChallenge.target_choice = "";
    newChallenge.accepted = false;
    newChallenge.created_at = time(NULL);
    newChallenge.channel = channelName;
    
    activeChallenges[client->getNickName()] = newChallenge;
    //send notices to 1- challenger, 2- target, 3-channel broadcast
    sendReplay(client->getFd(), _NOTICE(client->getNickName(), "Challenge sent to " + targetNick + " in " + channelName));
    sendReplay(target->getFd(), _NOTICE(target->getNickName(), "You've been challenged to RPS by " + client->getNickName() + " in " + channelName + ". Use /accept to confirm"));
    channel->broadcast(_NOTICE(channel->getName(), client->getNickName() + " challenged " + targetNick + " to RPS!"), "");
}
