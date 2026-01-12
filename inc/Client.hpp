#pragma once

#include "Needs.hpp"

class Client {
	int 		_fd;
	std::string _nickName;
	std::string _userName;
	std::string _realName;
	std::string _clientIp;
	std::string _inputBuffer;

	bool _hasPassword;
	bool _hasNickName;
	bool _hasUserName;
	bool _isRegistred;

public:
	Client(int fd);
	~Client();

	// GETTERS
	int getFd() const;
	std::string getClientIp() const;
	std::string getNickName() const;
	std::string getUserName() const;
	std::string getRealName() const;
	bool isRegistred() const;
	bool hasPassword() const;
	bool hasNickName() const;
	bool hasUserName() const;
	std::string getClientInfo() const;

	// SETTERS
	void setNickName(const std::string& nickName);
    void setUserName(const std::string& userName);
    void setRealName(const std::string& realName);
    void setPassword(bool status);
    void setIsRegistered(bool status);

	void appendToBuffer(const std::string& data);
    std::string& getBuffer();
    void clearBuffer(size_t pos);
};