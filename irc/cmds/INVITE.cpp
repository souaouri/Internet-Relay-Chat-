#include "../inc/Server.hpp"

void	Server::handleInvite(Client* client, const std::vector<std::string>& params)
{
	std::string nickName = params[0];
	std::string channelName = params[1];
	std::string reason = "";


	if (params.size() > 2 && params[2][0] == ':')
		reason = params[2];

	if (!client->isRegistred())
	{
        sendReplay(client->getFd(), ERR_NOTREGISTERED(std::string("*")));
        return;
    }
	if (params.empty() || params.size() < 2)
	{
		sendReplay(client->getFd(), ERR_NEEDMOREPARAMS(std::string("INVITE")));
		return ;
	}
	Client* invitedClient = searchForUser(nickName);

	if (!invitedClient)
	{
		sendReplay(client->getFd(), ERR_NOSUCHNICK(client->getNickName()));
		return ;
	}
	Channel* channel = _channelManager.search_for_channel(channelName);
	if (!channel)
	{
		sendReplay(client->getFd(), ERR_NOSUCHCHANNEL(client->getNickName(), channelName));
		return ;
	}
	if (!channel->isOperator(client))
    {
        sendReplay(client->getFd(), ERR_CHANOPRIVSNEEDED(channelName));
        return;
    }
	if (channel->isMember(invitedClient))
	{
		sendReplay(client->getFd(), ERR_USERONCHANNEL(client->getNickName(), invitedClient->getNickName(), channelName));
		return ;
	}

	sendReplay(client->getFd(), RPL_INVITING(client->getNickName(), nickName, channelName));
	sendReplay(invitedClient->getFd(), RPL_INVITED(client->getNickName(), nickName, channelName, client->getClientInfo()));

	channel->addInvitedOnly(invitedClient);
}
