#include "../inc/Needs.hpp"
#include "../inc/Server.hpp"

Client* Server::findClientByNickname(const std::string& nickname) {
    for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
        if (it->second->getNickName() == nickname) {
            return it->second;
        }
    }
    return NULL;
}

void Server::handlePrivmsg(Client* client, const std::vector<std::string>& params) {
    if (!client->isRegistred()) {
        sendReplay(client->getFd(), ERR_NOTREGISTERED(std::string("*")));
        return;
    }
    if (params.size() < 2) {
        sendReplay(client->getFd(), ERR_NEEDMOREPARAMS(std::string("PRIVMSG")));
        return;
    }
    
    std::vector<std::string> channels = split(params[0], ',');
    std::string recvmessage = params[1];

    std::string senderInfo = client->getNickName() + "!" + client->getUserName() + "@" + client->getClientIp();
    for (std::vector<std::string>::iterator it = channels.begin(); it != channels.end(); it++) {
        std::string target = *it;
        std::string msgToSend = ":" + senderInfo + " PRIVMSG " + target + " :" + recvmessage + "\r\n";
        if (target[0] == '#') {
            Channel* channel = _channelManager.search_for_channel(target);
            if (!channel) {
                sendReplay(client->getFd(), ERR_NOSUCHNICK(target));
                return;
            }
            if (!channel->isMember(client)) {
                sendReplay(client->getFd(), ERR_NOTONCHANNEL(target));
                return;
            }
            channel->broadcast(msgToSend, client->getNickName());
        } else {
            Client* recipient = findClientByNickname(target);
            if (!recipient) {
                sendReplay(client->getFd(), ERR_NOSUCHNICK(client->getNickName()));
                return;
            }
            send(recipient->getFd(), msgToSend.c_str(), msgToSend.size(), 0);
        }
    }
}