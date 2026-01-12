#include "../inc/Needs.hpp"
#include "../inc/Server.hpp"

void Server::handleTopic(Client* client, const std::vector<std::string>& params) {
    if (!client->isRegistred()) {
        sendReplay(client->getFd(), ERR_NOTREGISTERED(std::string("*")));
        return;
    }

    if (params.empty()) {
        sendReplay(client->getFd(), ERR_NEEDMOREPARAMS(std::string("TOPIC")));
        return;
    }

    std::string channelName = params[0];
    Channel* channel = _channelManager.search_for_channel(channelName);

    if (!channel) {
        sendReplay(client->getFd(), ERR_NOSUCHCHANNEL(client->getNickName(), channelName));
        return;
    }

    if (!channel->isMember(client)) {
        sendReplay(client->getFd(), ERR_NOTONCHANNEL(channelName));
        return;
    }

    if (params.size() == 1) {
        std::string topic = channel->getTopic();
            sendReplay(client->getFd(), RPL_TOPIC(topic, client->getNickName(), channelName));
    } else {
        if (!channel->isOperator(client) && channel->getTopicMode()) {
            sendReplay(client->getFd(), ERR_CHANOPRIVSNEEDED(channelName));
            return;
        }

        std::string newTopic = (params[1].empty()) ? "" : params[1];
        channel->setTopic(newTopic);

        if (newTopic[0] == ':')
            newTopic = newTopic.substr(1);
        std::string topicMsg = ":" + client->getNickName() + " TOPIC " + channelName + " :" + newTopic + "\r\n";
        channel->broadcast(topicMsg, "");
    }
}
