#include "../inc/Client.hpp"

Client::Client(int fd) : _fd(fd), _hasPassword(false), _hasNickName(false), _hasUserName(false), _isRegistred(false) {
	struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    char clientIp[INET_ADDRSTRLEN];

    if (getpeername(fd, (struct sockaddr *)&client_addr, &client_len) == -1)
		throw std::runtime_error("getpeername failed");
    if (inet_ntop(AF_INET, &client_addr.sin_addr, clientIp, sizeof(clientIp)) == NULL)
		throw std::runtime_error("inet_ntop failed");
    _clientIp = std::string(clientIp);
}

std::string Client::getClientInfo() const {
    return _nickName + "!" + _userName + "@" + _clientIp;
}

Client::~Client() {}

void Client::appendToBuffer(const std::string& data) { _inputBuffer += data;}
std::string& Client::getBuffer() { return _inputBuffer; }
void Client::clearBuffer(size_t pos) { _inputBuffer.erase(0, pos);}

// GETTERS
int Client::getFd() const {return _fd;}
std::string Client::getClientIp() const {return  _clientIp;}
std::string Client::getNickName() const {return _nickName;}
std::string Client::getUserName() const {return _userName;}
std::string Client::getRealName() const {return _realName;}
bool Client::isRegistred() const {return _isRegistred;}
bool Client::hasPassword() const {return _hasPassword;}
bool Client::hasNickName() const {return _hasNickName;}
bool Client::hasUserName() const {return _hasUserName;}


// SETTERS
void Client::setNickName(const std::string& nickName) {
	_nickName = nickName, _hasNickName = true;
}
void Client::setUserName(const std::string& userName) {
	_userName = userName, _hasUserName = true;
}
void Client::setRealName(const std::string& realName) {
	_realName = realName;
}
void Client::setPassword(bool status) {
	_hasPassword = status;
}
void Client::setIsRegistered(bool status) {
	_isRegistred = status;
}