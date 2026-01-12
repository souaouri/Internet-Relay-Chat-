#include "../inc/Server.hpp"

void Server::handleKick(Client* client, const std::vector<std::string>& params)
{
	if (!client->isRegistred()) {
        sendReplay(client->getFd(), ERR_NOTREGISTERED(std::string("*")));
        return;
    }
    if (params.size() < 2)
    {
        sendReplay(client->getFd(), ERR_NEEDMOREPARAMS(std::string("KICK")));
        return;
    }
    std::string channelName = params[0];
    std::string nickname = params[1];
	std::string reason = (params[2].empty()) ? "Kicked by operator" : params[2];
    Channel* channel = _channelManager.search_for_channel(channelName);
    if (!channel)
    {
        sendReplay(client->getFd(), ERR_NOSUCHCHANNEL(client->getNickName(), channelName));
        return;
    }
    if (!channel->isOperator(client))
    {
        sendReplay(client->getFd(), ERR_CHANOPRIVSNEEDED(channelName));
        return;
    }

	if (!channel->checkForClient(nickname))
	{
		sendReplay(client->getFd(), ERR_NOSUCHNICK(client->getNickName()));
		return ;
	}
    std::string kickMsg = ":" + client->getNickName() + " KICK " + channelName + " " + nickname + " :" + reason + "\r\n";

    channel->removeUser(nickname, kickMsg);
     if (channel->opIsEmpty()) {
        Client* newOp = channel->firstMumber();
        channel->addOperator(newOp);

        std::string modeMsg = ":FT_irc MODE " + channel->getName() + " +o " + newOp->getNickName() + "\r\n";
        channel->broadcast(modeMsg, "");
    }
    channel->broadcast(kickMsg, "");


    std::vector<std::string> names = channel->getMemberNames();
    std::string namesStr;
    for (size_t i = 0; i < names.size(); i++)
    {
        if (!namesStr.empty()) namesStr += " ";
        namesStr += names[i];
    }
    std::string namesReply = RPL_NAMREPLY(client->getNickName(), channelName, namesStr);
    std::string endOfNames = RPL_ENDOFNAMES(client->getNickName(), channelName);

    if (channel->isEmpty())
        _channelManager.removeChannel(channelName);
}