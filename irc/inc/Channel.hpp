#pragma once

#include "Needs.hpp"
#include "Client.hpp"

class Channel {
private:
    std::string name;
    std::string password; //changed
    std::vector<Client*> members;
    std::vector<Client*> operators;
	std::vector<char> 		modes;
    std::string _topic;
	// modes
	int inviteOnly;
	int  limit;
	bool topicMode;

public:
	void setTopicMode(bool state);
	bool getTopicMode();
	std::vector<Client*> 			_invitedClients;
	bool isInviteOnly(std::string nickName);
	bool checkEmptyOp();
	void setFirtOp();
	void setModes(char mode);
	std::vector<char> getModes();
	void removeMode(char mode);
	void setInviteOnly(int flag);
	int getInviteOnly();
	void setLimit(int limit);
	int  getLimit();
    void setTopic(const std::string &newTopic);
	void	addInvitedOnly(Client* invitedClient);
    std::string getTopic() const;
    size_t getMemrbersNum();
    size_t getOpNum();
    Channel(std::string new_name);
    std::string getName();
	std::string getPassword();//changed
	void setPassword(std::string password);//changed
    void addMember(Client* client);
    void addOperator(Client* client);
    bool isOperator(Client* client);
    bool isMember(Client* client);
    bool checkForClient(std::string client_name);
	bool isEmpty();
    void removeUser(std::string nickname, std::string msg);
    void broadcast(const std::string &msg, std::string senderNick);
	void modeBroadcast(const std::string &msg);
    std::vector<std::string> getMemberNames();
    void removeClient(Client* client);
	Client* searchForMember(std::string nickname);
	void addOp(std::string newOp);
	void removeOp(std::string newOp);
	bool opIsEmpty();
	Client *firstMumber();
};

class ChannelManager {
public:
	~ChannelManager();
    std::vector<Channel*> Channels;
    Channel*	search_for_channel(std::string channel_name);
    Channel*	CreatChannel(std::string channel_name);
	void		removeChannel(const std::string& channelName);
};

