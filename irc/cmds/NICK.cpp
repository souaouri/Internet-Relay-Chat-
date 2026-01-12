#include "../inc/Needs.hpp"
#include "../inc/Server.hpp"


void Server::handleNick(Client* client, const std::vector<std::string>& params) {
    if (!client->hasPassword()) {
        sendReplay(client->getFd(), ERR_NOTREGISTERED(std::string("*")));
        return;
    }
    if (params.size() != 1) {
        sendReplay(client->getFd(), ERR_NEEDMOREPARAMS(std::string("NICK")));
        return;
    }
    if (params.empty()) {
        sendReplay(client->getFd(), ERR_NONICKNAMEGIVEN(client->getNickName()));
        return;
    }

    std::string nickname = params[0];
    std::transform(nickname.begin(), nickname.end(), nickname.begin(), ::tolower);

    if (client->getNickName() == nickname) return;

    if (!isValidNickname(nickname)) {
        sendReplay(client->getFd(), ERR_ERRONEUSNICKNAME(client->getNickName(), nickname));
        return;
    }
    if (findClientByNickname(nickname)) {
        sendReplay(client->getFd(), ERR_NICKNAMEINUSE(client->getNickName(), nickname));
        return;
    }

    std::string oldNick = client->getNickName();
    client->setNickName(nickname);

    std::string msg = ":" + oldNick + " NICK :" + nickname + "\r\n";
    
    for (std::vector<Channel*>::iterator it = _channelManager.Channels.begin(); it != _channelManager.Channels.end(); ++it) {
        Channel* channel = *it;
        if (channel->isMember(client))
            channel->broadcast(msg, oldNick);
    }
	authenticateClient(client);
}

bool Server::isValidNickname(const std::string& nickname) {
    if (nickname.empty() || nickname.length() > 9)
        return false;
    const std::string special = "-[]\\`^{}";
    for (size_t i = 0; i < nickname.length(); ++i) {
        char c = nickname[i];
        if (i == 0 && !isalpha(c) && special.find(c) == std::string::npos)
            return false;
        if (!isalnum(c) && special.find(c) == std::string::npos)
            return false;
    }
    return true;
}