#include "../inc/Needs.hpp"
#include "../inc/Server.hpp"

void Server::handleUser(Client *client, const std::vector<std::string> &params) {
	if (!client->hasPassword()) {
        sendReplay(client->getFd(), ERR_NOTREGISTERED(std::string("*")));
        return;
    }
	if (client->isRegistred()) {
		sendReplay(client->getFd(), ERR_ALREADYREGISTRED(std::string("*")));
		return;
	}
	if (params.size() < 4) {
		sendReplay(client->getFd(), ERR_NEEDMOREPARAMS(std::string("USER")));
		return;
	}
	client->setUserName(params[0]);
	client->setRealName(params[3].substr(1));
	authenticateClient(client);
}