#include "../inc/Server.hpp"

void Server::handleMode(Client* client, const std::vector<std::string>& params)
{
	if (!client->isRegistred()) {
        sendReplay(client->getFd(), ERR_NOTREGISTERED(std::string("*")));
        return;
    }
    if (params.empty())
	{
        sendReplay(client->getFd(), ERR_NEEDMOREPARAMS(std::string("MODE")));
        return;
    }

	std::string channelName = params[0];
	Channel *channel = _channelManager.search_for_channel(channelName);
	if (!channel)
	{
		sendReplay(client->getFd(), ERR_NOSUCHCHANNEL(client->getNickName(), channelName));
		return ;
	}
	if (params.size() == 1)
	{
		std::string modes = "";
		for (size_t i = 0; i < channel->getModes().size(); i++)
		{
			modes += channel->getModes()[i];
		}
		std::string modeMsg = ": 324 " + client->getNickName() + " " + channelName + " " + (modes.empty() ? "(no modes set)" : "+" + modes) + "\r\n";
		sendReplay(client->getFd(), modeMsg);


		return;
	}
	if (!channel->isOperator(client))
    {
        sendReplay(client->getFd(), ERR_CHANOPRIVSNEEDED(channelName));
        return;
    }
	if (params[1].size() == 2 && (params[1][0] == '-' || params[1][0] == '+'))
	{
		char sign = params[1][0];
		if (params[1][1] == 'i')
		{
			if (params.size() > 2) {
				sendReplay(client->getFd(), ERR_INVALIDMODEPARAM(client->getNickName(), channelName, params[2]));
				return;
			}
			if (sign == '+')
			{
				if (channel->getInviteOnly()) {
					sendReplay(client->getFd(), ERR_KEYSET(channelName));
					return;
				}
				channel->setInviteOnly(1);
				channel->setModes('i');
			}
			else
			{
				if (!channel->getInviteOnly()) {
					sendReplay(client->getFd(), ERR_KEYSET(channelName));
					return;
				}
				channel->setInviteOnly(0);
				channel->removeMode('i');
			}
		}
		else if (params[1][1] == 't')
		{
			if (sign == '+')
			{
				if (params.size() > 2) {
					sendReplay(client->getFd(), ERR_INVALIDMODEPARAM(client->getNickName(), channelName, params[2]));
					return;
				}	
				channel->setTopicMode(true);
			}
			else
			{
				if (params.size() > 2) {
					sendReplay(client->getFd(), ERR_INVALIDMODEPARAM(client->getNickName(), channelName, params[2])); return;
				}	
				channel->setTopicMode(false);
			}
		}
		else if (params[1][1] == 'k')
		{
			if (sign == '+')
			{
				if (params.size() < 3) {
					sendReplay(client->getFd(), ERR_NEEDMOREPARAMS(std::string("MODE"))); return;
				}
				if (params.size() > 3) {
					sendReplay(client->getFd(), ERR_INVALIDMODEPARAM(client->getNickName(), channelName, params[2])); return;
				}
				channel->setPassword(params[2]);
				channel->setModes('k');
			}
			else
			{
				if (params.size() > 2){
					sendReplay(client->getFd(), ERR_INVALIDMODEPARAM(client->getNickName(), channelName, params[2])); return;
				}
				channel->setPassword("");
				channel->removeMode('k');
			}
		}
		else if (params[1][1] == 'o')
		{
			if (params.size() < 3)
			{
				sendReplay(client->getFd(), ERR_NEEDMOREPARAMS(std::string("MODE")));
				return;
			}
			if (params.size() > 3)
			{
				sendReplay(client->getFd(), ERR_INVALIDMODEPARAM(client->getNickName(), channelName, params[2]));
				return;
			}
			Client* mem = searchForUser(params[2]);
			if (sign == '+')
			{
				if (!channel->isMember(mem))
				{
					sendReplay(client->getFd(), ERR_USERNOTINCHANNEL(client->getNickName()      ,channelName));
					return ;
				}
				if (channel->isOperator(mem))
				{
					sendReplay(client->getFd(), ERR_OPSET(client->getNickName()));
					return ;
				}
				channel->addOp(mem->getNickName());
			}
			else 
			{
				if (params.size() > 3)
				{
					sendReplay(client->getFd(), ERR_INVALIDMODEPARAM(client->getNickName(), channelName, params[2]));
					return ;
				}
				if (!channel->isOperator(mem))
				{
					sendReplay(client->getFd(), RPL_ISNOTOP(mem->getNickName()));
					return ;
				}
				if (channel->getOpNum() == 1)
				{
					sendReplay(client->getFd(), ERR_OP(mem->getNickName()));
					return;
				}
				channel->removeOp(mem->getNickName());
			}

		}
		
		else if (params[1][1] == 'l')
		{
			if (sign == '+')
			{
				if (params[2].empty())
				{
					sendReplay(client->getFd(), ERR_NEEDMOREPARAMS(std::string("MODE")));
					return ;
				}
				if (params.size() > 3)
				{
					sendReplay(client->getFd(), ERR_INVALIDMODEPARAM(client->getNickName(), channelName, params[2]));
					return;
				}
				int lim = 0;
				std::stringstream iss(params[2]);

				iss >> lim;
				for (size_t i = 0; i < params[2].size(); i++)
				{
					if (params[2].empty() || params[2][i] < '1' || params[2][i] > '9'){
						sendReplay(client->getFd(), ERR_INVALIDMODEPARAM(client->getNickName(), channelName, params[2]));
						return;
					}
				}
				if (lim < static_cast<int>(channel->getMemrbersNum())) {
					sendReplay(client->getFd(), ERR_INVALIDLIMIT(client->getNickName()));
					return;
				}
				channel->setLimit(lim);
				channel->setModes('l');
			}
			else
			{
				if (params.size() > 2)
				{
					sendReplay(client->getFd(), ERR_INVALIDMODEPARAM(client->getNickName(), channelName, params[2]));
					return;
				}
				channel->setLimit(0);
				channel->removeMode('l');
			}
		}
		else
		{
			sendReplay(client->getFd(), ERR_NOCHANMODES(client->getNickName(), channelName));
			return;
		}
	}
	else if (!params[2].empty() && params[2] != "+sn")
	{
		sendReplay(client->getFd(), ERR_NOCHANMODES(client->getNickName(), channelName));
		return ;
	}
	if (params.size() > 2)
		channel->modeBroadcast(RPL_CHANGEMODE(channelName, params[1], params[2]));
	else
	 	channel->modeBroadcast(RPL_CHANGEMODE(channelName, params[1], ""));
}