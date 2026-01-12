#include "../inc/Channel.hpp"
#include "../inc/Needs.hpp"
#include "../inc/Server.hpp"
#include "../inc/Client.hpp"

Channel::Channel(std::string new_name) : name(new_name), password(""), _topic("No topic is set"), inviteOnly(0), limit(0), topicMode(false){}

ChannelManager::~ChannelManager() {
	for (size_t i = 0; i < Channels.size(); ++i) {
        delete Channels[i];
    }
    Channels.clear();
}

bool Channel::checkEmptyOp() {
	if (operators.size() < 2)
		return true;
	else return false;
}

void Channel::setFirtOp() {
	std::string name;
	if (operators.empty() && !members.empty()) {
		operators.push_back(members[0]);
		name = "@" + operators[0]->getNickName();
		operators[0]->setNickName(name);
	}
}

void Channel::addOp(std::string newOp) {
	std::string name;
	Client* client = searchForMember(newOp);
	name = client->getNickName();
	client->setNickName(name);
	operators.push_back(client);
}


Client* Channel::searchForMember(std::string nickname) {
	for (size_t i = 0; i < members.size(); ++i) {
        if (nickname == members[i]->getNickName()) {
            return members[i];
        }
    }
    return NULL;
}

void Channel::setInviteOnly(int flag){
	inviteOnly = flag;
}

int Channel::getInviteOnly(){return inviteOnly;}

void Channel::setLimit(int limit) {
	this->limit = limit;
}

int Channel::getLimit(){return limit;}

bool Channel::isInviteOnly(std::string nickName)
{
	for(size_t i = 0; i < _invitedClients.size(); i++)
	{
		if (_invitedClients[i]->getNickName() == nickName)
			return true;
	}
	return false;
}

void	Channel::addInvitedOnly(Client* invitedClient)
{
	for (size_t i = 0; i < _invitedClients.size(); i++)
	{
		if (_invitedClients[i] == invitedClient)
			return ;
	}
	_invitedClients.push_back(invitedClient);
}

void Channel::removeClient(Client* client) {
    bool wasOp = isOperator(client);

    members.erase(std::remove(members.begin(), members.end(), client), members.end());
    operators.erase(std::remove(operators.begin(), operators.end(), client), operators.end());

    if (wasOp && operators.empty() && !members.empty()) {
        Client* newOp = members.front();
        addOperator(newOp);

        std::string modeMsg = ":FT_irc MODE " + getName() + " +o " + newOp->getNickName() + "\r\n";
        broadcast(modeMsg, "");
    }
}
size_t Channel::getMemrbersNum() {return members.size();}

size_t Channel::getOpNum() {return operators.size();}

void Channel::setTopic(const std::string &newTopic) {
       _topic = newTopic;
}


std::string Channel::getTopic() const {
    return _topic;
}

std::string Channel::getName() {
    return name;
}

std::string Channel::getPassword() {return password;}

void Channel::setPassword(std::string pass) {
    password = pass;
}

void Channel::setModes(char mode)
{
    for (std::vector<char>::iterator it = modes.begin(); it != modes.end(); it++) {
        if (*it == mode)
            return;
    }
	modes.push_back(mode);
}

std::vector<char> Channel::getModes()
{
	return modes;
}

void Channel::removeMode(char mode)
{
	for (std::vector<char>::iterator it = modes.begin(); it != modes.end(); ++it) {
    	if ((*it) == mode) {
            modes.erase(it); return;
		}
    }
}

void Channel::addMember(Client* client) {
    if (std::find(members.begin(), members.end(), client) == members.end()) {
        members.push_back(client);
    }
}

void Channel::addOperator(Client* client) {
    if (!isOperator(client)) {
        operators.push_back(client);
    }
}

bool Channel::isOperator(Client* client) {
    return std::find(operators.begin(), operators.end(), client) != operators.end();
}

bool Channel::isMember(Client* client) {
    return std::find(members.begin(), members.end(), client) != members.end();
}

bool Channel::checkForClient(std::string client_name)
{
    for (size_t i = 0; i < members.size(); ++i) {
        if (client_name == members[i]->getNickName())
            return true;
    }
    return false;
}

bool Channel::isEmpty()
{
	if (!members.size())
		return true;
	return false;
}

void Channel::setTopicMode(bool state) {
    topicMode = state;
}
bool Channel::getTopicMode() {
    return topicMode;
}

void Channel::removeUser(std::string nickname, std::string msg)
{
    for (std::vector<Client*>::iterator it = members.begin(); it != members.end(); ++it) {
        if ((*it)->getNickName() == nickname)
		{
			send((*it)->getFd(), msg.c_str(), msg.size(), 0);
            members.erase(it);
            break;
        }
    }
    for (std::vector<Client*>::iterator it = operators.begin(); it != operators.end(); ++it) {
        if ((*it)->getNickName() == nickname)
		{
			// send((*it)->getFd(), msg.c_str(), msg.size(), 0);
            operators.erase(it);
            break;
        }
    }
}

bool Channel::opIsEmpty() {
    if (operators.empty())
        return true;
    return false;
}

Client * Channel::firstMumber() {
    return members.front();
}

void Channel::removeOp(std::string nickname)
{
    for (std::vector<Client*>::iterator it = operators.begin(); it != operators.end(); ++it) {
        if ((*it)->getNickName() == nickname) {
            operators.erase(it); return;
		}
    }
}

void Channel::broadcast(const std::string &msg, std::string senderNick){
    for (size_t i = 0; i < members.size(); ++i) {
        if (members[i]->getNickName() != senderNick) {
            send(members[i]->getFd(), msg.c_str(), msg.size(), 0);
        }
    }
}

void Channel::modeBroadcast(const std::string &msg){
    for (size_t i = 0; i < members.size(); ++i) {
        send(members[i]->getFd(), msg.c_str(), msg.size(), 0);
    }
}

std::vector<std::string> Channel::getMemberNames() {
    std::vector<std::string> names;
    for (size_t i = 0; i < members.size(); ++i) {
        std::string name = members[i]->getNickName();
        for (size_t j = 0; j < operators.size(); j++) {
            if (operators[j]->getNickName() == members[i]->getNickName()) {
                name = "@" + name;  // Prepend @ for operators
                break;
            }
        }
        names.push_back(name);
    }
    return names;
}

Channel* ChannelManager::search_for_channel(std::string channel_name) {
    for (size_t i = 0; i < Channels.size(); ++i) {
        if (channel_name == Channels[i]->getName()) {
            return Channels[i];
        }
    }
    return NULL;
}

Channel* ChannelManager::CreatChannel(std::string channel_name) {
    Channel* exist_channel = search_for_channel(channel_name);
    if (!exist_channel) {
        Channel* newChannel = new Channel(channel_name);
        Channels.push_back(newChannel);
        return newChannel;
    }
    return exist_channel;
}

void ChannelManager::removeChannel(const std::string& channelName)
{
    for (std::vector<Channel*>::iterator it = Channels.begin(); it != Channels.end(); ++it) {
        if ((*it)->getName() == channelName) {
            delete *it;
            Channels.erase(it);
            return;
        }
    }
}
