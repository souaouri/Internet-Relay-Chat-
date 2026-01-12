#include "../inc/Server.hpp"

void Server::createSock() {
	_serverFd = socket(AF_INET, SOCK_STREAM, 0);
	if (_serverFd == -1)
		throw (std::runtime_error("socket error : " + std::string(strerror(errno))));
	int opt = 1;
	if (setsockopt(_serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
		close(_serverFd), throw (std::runtime_error("setsockopt error : " + std::string(strerror(errno))));
	if (fcntl(_serverFd, F_SETFL, O_NONBLOCK) == -1)
		close(_serverFd), throw (std::runtime_error("fcntl error : " + std::string(strerror(errno))));
}

void Server::setupAddr() {
	memset(&_serverAddress, 0, sizeof(_serverAddress));
	_serverAddress.sin_family = AF_INET;
	_serverAddress.sin_addr.s_addr = INADDR_ANY;
	_serverAddress.sin_port = htons(_port);
}

void Server::bindSock() {
	if (bind(_serverFd, (struct sockaddr *)&_serverAddress, sizeof(_serverAddress)) == -1)
		close(_serverFd), throw (std::runtime_error("bind error : " + std::string(strerror(errno))));
}

void Server::startListening() {
	if (listen(_serverFd, SOMAXCONN) == -1)
		close(_serverFd), throw (std::runtime_error("listen error : " + std::string(strerror(errno))));
	std::cout << GRE << SERV() << END << std::endl;
	std::cout << GRE << "\tServer is running on port " << _port << std::endl;
	std::cout << YEL << "\tWaiting for connections..." << std::endl;
}

void Server::handleNewConnection() {

	struct sockaddr_in 	clientAddress;
	socklen_t			clientLen = sizeof(clientAddress);
	int 				clientFd = accept(_serverFd, (struct sockaddr*)&clientAddress, &clientLen);
	if (clientFd == -1) {
		if (errno == EAGAIN || errno == EWOULDBLOCK) {
			return;
		}
		std::cerr << "accept error : " << strerror(errno) << std::endl;
		return;
	}
	if (fcntl(clientFd, F_SETFL, O_NONBLOCK) == -1) {
		close(clientFd), std::cerr << "fcntl error when add new client : " << strerror(errno) << std::endl;
		return; 
	}
	Client *newClient = new Client(clientFd);
	_clients[clientFd] = newClient;
	_pollFds.push_back((pollfd){clientFd, POLLIN, 0});
	std::cout << BLU << "New client connected with fd: " << clientFd << END << std::endl;
}

void Server::handleClientData(int clientFd) {
    char buffer[1024];
    ssize_t bytesRead = recv(clientFd, buffer, sizeof(buffer) - 1, 0);

    if (bytesRead <= 0) {
        if (bytesRead == 0) {
			Client* client = _clients[clientFd];
	        if (client) {
	            handleQuit(client, std::vector<std::string>(0));
	        }
            return;
        } else if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return;
        } 
        else {
			Client* client = _clients[clientFd];
	        if (client) {
	            handleQuit(client, std::vector<std::string>(0));
	        }
            std::cerr << "recv error: " << strerror(errno) << std::endl;
            return;
        }
    }

    buffer[bytesRead] = '\0';
    Client* client = _clients[clientFd];
	client->appendToBuffer(buffer);
	
	std::string buff = client->getBuffer();
	if (buff.substr(0, 4) != "PING" && buff.substr(0, 4) != "PONG")
		std::cout << GRA << "--> " <<  client->getBuffer().substr(0, client->getBuffer().find_last_of('\n')) << END << std::endl;

    size_t pos;
   	while (true) {
        pos = client->getBuffer().find("\r\n");
        if (pos != std::string::npos) {
            std::string message = client->getBuffer().substr(0, pos);
            client->clearBuffer(pos + 2);
            processCommand(client, message);
            continue;
        }

        pos = client->getBuffer().find('\n');
        if (pos != std::string::npos) {
            std::string message = client->getBuffer().substr(0, pos);
            client->clearBuffer(pos + 1);
            processCommand(client, message);
            continue;
        }
        break;
    }
}

void Server::processCommand(Client* client, const std::string& command) {
	std::istringstream iss(command);
	std::vector<std::string> params;
	std::string cmd;
	std::string response;

	iss >> cmd;
	for (std::string param; iss >> param; ) {
		if (param[0] == ':' && param.size() > 1) {
			std::string lastParam = param.substr(1);
			std::string rest;
			std::getline(iss, rest);
			lastParam += rest;
			params.push_back(lastParam);
			break;
		}
		params.push_back(param);
	}
	std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::toupper);
	if (cmd == "PASS") handlePass(client, params);
	else if (cmd == "NICK") handleNick(client, params);
	else if (cmd == "USER") handleUser(client, params);
	else if (cmd == "JOIN") handleJoin(client, params);
	else if (cmd == "KICK") handleKick(client, params);
	else if (cmd == "TOPIC") handleTopic(client, params);
	else if (cmd == "INVITE") handleInvite(client, params);
	else if (cmd == "PRIVMSG") handlePrivmsg(client, params);
	else if (cmd == "MODE") handleMode(client, params);
	else if (cmd == "QUIT") handleQuit(client, params);
	else if (cmd == "RPS") handleRPS(client,params);
	else if (cmd == "CHOOSE") handleChoose(client, params);
	else if (cmd == "ACCEPT") handleAccept(client);
	else if (cmd != "PONG" && cmd != "PING") {
		response = ERR_UNKNOWNCOMMAND(cmd);
		sendReplay(client->getFd(), response);
	}
}

void Server::closeClient(int clientFd) {
    std::map<int, Client*>::iterator it = _clients.find(clientFd);
    if (it == _clients.end()) return;

    Client* client = it->second;
    std::vector<std::string> channelsToRemove;
    for (std::vector<Channel*>::iterator chanIt = _channelManager.Channels.begin(); chanIt != _channelManager.Channels.end(); ++chanIt) {
        if ((*chanIt)->isMember(client)) {
            (*chanIt)->removeClient(client);
            if ((*chanIt)->isEmpty()) {
                channelsToRemove.push_back((*chanIt)->getName());
            }
        }
    }
    for (size_t i = 0; i < channelsToRemove.size(); ++i) {
        _channelManager.removeChannel(channelsToRemove[i]);
    }
    for (std::vector<pollfd>::iterator pollIt = _pollFds.begin(); pollIt != _pollFds.end(); ++pollIt) {
        if (pollIt->fd == clientFd) {
            _pollFds.erase(pollIt);
            break;
        }
    }
    delete client;
    _clients.erase(clientFd);

    close(clientFd);
}

Server::Server(int port, const std::string &password) : _port(port), _password(password), _serverFd(-1), _running(false) {
	createSock();
	setupAddr();
	bindSock();
	startListening();
}

Server::~Server() {
    for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
        close(it->first);
        delete it->second;
    }
    close(_serverFd);
}


void Server::authenticateClient(Client *client) {
	if (client->hasPassword() && client->hasNickName() && client->hasUserName() && !client->isRegistred()) {
		client->setIsRegistered(true);
		welcomingMessage(client);
	}
}

void Server::welcomingMessage(Client* client) const {
	sendReplay(client->getFd(), RPL_WELCOME(client->getNickName(), client->getUserName(), client->getClientIp()));
	sendReplay(client->getFd(), RPL_YOURHOST(client->getNickName()));
	sendReplay(client->getFd(), RPL_CREATED(client->getNickName()));
	sendReplay(client->getFd(), RPL_MYINFO(client->getNickName()));
}

void Server::sendReplay(int fd, const std::string& response) const {
	std::string formatted = response;
	if (formatted.find("\r\n") == std::string::npos)
		formatted += "\r\n";
    send(fd, formatted.c_str(), formatted.size(), 0);
}
void Server::run() {
	_running = true;
	_pollFds.push_back((pollfd){_serverFd, POLLIN, 0});

	while (_running && g_keepRunning) {
		int state = poll(&_pollFds[0], _pollFds.size(), -1);

		if (state < 0 && errno != EINTR)
			throw (std::runtime_error("poll error : " + std::string(strerror(errno))));
		for (size_t i = 0; i < _pollFds.size(); i++) {
			if (_pollFds[i].fd == _serverFd)
				handleNewConnection();
			else
			 	handleClientData(_pollFds[i].fd);
		}
	}
	_running = false;
}

Client* Server::searchForUser(std::string nickname)
{
    for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
        if (it->second && it->second->getNickName() == nickname) {
            return it->second;
        }
    }
    return NULL;
}

