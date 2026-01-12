#include "../inc/Server.hpp"

std::vector<std::string> Server::split(const std::string& str, char delimiter)
{
    std::vector<std::string> tokens;
    size_t start = 0;
    size_t end;

    while ((end = str.find(delimiter, start)) != std::string::npos)
	{
        tokens.push_back(str.substr(start, end - start));
        start = end + 1;
	}
    tokens.push_back(str.substr(start));

    return tokens;
}

void Server::handleJoin(Client* client, const std::vector<std::string>& params) {
    if (!client->isRegistred()) {
        sendReplay(client->getFd(), ERR_NOTREGISTERED(std::string("*")));
        return;
    }
    if (params.empty()) {
        sendReplay(client->getFd(), ERR_NEEDMOREPARAMS(std::string("JOIN")));
        return;
    }
	std::vector<std::string> channels = split(params[0], ',');
	std::vector<std::string> passwords;
	if (params.size() > 1) {
	    std::string passParam = params[1];
	    if (!passParam.empty() && passParam[0] == ':') {
	        passParam = passParam.substr(1);
	    }

	    std::istringstream iss(passParam);
	    std::string token;
	    while (std::getline(iss, token, ' ')) {
	        if (!token.empty()) {
	            std::istringstream tokenStream(token);
	            std::string subToken;
	            while (std::getline(tokenStream, subToken, ',')) {
	                if (!subToken.empty())
	                    passwords.push_back(subToken);
	            }
	        }
	    }
	}
	for(size_t i = 0; i < channels.size(); i++)
	{
		if (channels[i] == "#") {
	        sendReplay(client->getFd(), ERR_NEEDMOREPARAMS(std::string("JOIN")));
	        return;
		}
		std::string channelName = channels[i];
		if (channelName[0] != '#') {
			sendReplay(client->getFd(), ERR_NOSUCHCHANNEL(client->getNickName(), channelName));
			continue;
		}
		Channel* channel = _channelManager.search_for_channel(channelName);
		std::string password = (i < passwords.size()) ? passwords[i] : "";
		if (channel && !channel->getPassword().empty())
		{
			if (channel->getPassword() != password)
			{
				sendReplay(client->getFd(), ERR_BADCHANNELKEY(client->getNickName(), channelName));
				continue;
			}
		}
		if (channel && channel->getLimit() != 0)
		{
			if (channel->getMemrbersNum() == (size_t)channel->getLimit())
			{
				sendReplay(client->getFd(), ERR_CHANNELISFULL(client->getNickName(),channelName));
				continue;
			}
		}
		if (channel && channel->getInviteOnly() == 1)
		{
			if (!channel->isInviteOnly(client->getNickName()))
			{
				sendReplay(client->getFd(), ERR_INVITEONLYCHAN(client->getNickName(),channelName));
				continue;
			}
		}
		if (channel && channel->isMember(client)) {
            sendReplay(client->getFd(), ERR_USERONCHANNEL(client->getNickName(), client->getNickName(), channelName));
            continue;
        }
		if (!channel)
		{
			channel = _channelManager.CreatChannel(channelName);
			channel->addOperator(client);
		}
		channel->addMember(client);

		sendReplay(client->getFd(), RPL_JOIN(client->getNickName(), client->getUserName(), channelName, client->getClientIp()));
		channel->broadcast(RPL_JOIN(client->getNickName(), client->getUserName(), channelName, client->getClientIp()), client->getNickName());

		std::vector<std::string> names = channel->getMemberNames();
        std::string namesStr;
        for (size_t j = 0; j < names.size(); j++) {
            if (!namesStr.empty()) namesStr += " ";
            namesStr += names[j];
        }
		std::string topic = channel->getTopic();
		if (topic[0] == ':')
			topic = topic.substr(1);
		sendReplay(client->getFd(), RPL_TOPIC(topic,client->getNickName(), channelName));
        sendReplay(client->getFd(), RPL_NAMREPLY(client->getNickName(), channelName, namesStr));
        sendReplay(client->getFd(), RPL_ENDOFNAMES(client->getNickName(), channelName));
	}
}
