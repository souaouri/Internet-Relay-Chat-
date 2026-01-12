#include "../inc/Server.hpp"

void Server::handleQuit(Client* client, const std::vector<std::string>& params) {
    std::string quitMessage = "";
    if (!params.empty()) {
        quitMessage = params[0];
    }
    for (std::vector<Channel*>::iterator it = _channelManager.Channels.begin(); it != _channelManager.Channels.end(); ++it) {
        if ((*it)->isMember(client)) {
            (*it)->broadcast(RPL_QUIT(client->getNickName(), quitMessage), quitMessage), client->getNickName();
			if ((*it)->isOperator(client))
				(*it)->setFirtOp();
			
        }
    }
    std::cout << RED << "Client with fd : " << client->getFd() << " disconnected" << END << std::endl;
    closeClient(client->getFd());
}
