#include "../inc/Needs.hpp"
#include "../inc/Server.hpp"

void Server::handlePass(Client *client, const std::vector<std::string> &params) {
	std::string response;
	if (client->hasPassword()) {
		std::string msg = ERR_ALREADYREGISTRED(std::string("*"));
		send(client->getFd(),msg.c_str() , msg.size(), 0);
		return;
	}
	if (params.empty()) {
		response = ERR_NEEDMOREPARAMS(std::string("PASS"));
		sendReplay(client->getFd(), response);
		return;
	}
	if (params[0] != _password) {
		response = ERR_PASSWDMISMATCH(std::string("*"));
		sendReplay(client->getFd(), response);
		return;
	}
	client->setPassword(true);
}