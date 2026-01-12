#include "../inc/Server.hpp"

volatile sig_atomic_t g_keepRunning = 1;

void handleShutdown(int signum) {
    (void)signum;
    g_keepRunning = 0;
}

int validPort(std::string sPort) {
    size_t i = 0;
    if (sPort[0] == '+')
        i++;
    for (; i < sPort.size(); i++) {
        if (!std::isdigit(sPort[i]))
            return -1;
    }

    int port;
    std::stringstream ss(sPort);
    ss >> port;

    if (ss.fail() || (port < 1024 || port > 65535))
        return -1;
    return port;
}

int checkPassword(std::string password) {
    if (password.size() > 9 || password.size() < 5)
        return -1;
    for (size_t i = 0; i < password.size(); i++) {
        if (!std::isprint(password[i]) || std::isspace(password[i]))
            return -1;
    }
    return 0;
}

int main(int ac, char** av) {
    if (ac != 3) {
        std::cerr << "Usage: " << av[0] << " <port> <password>" << std::endl;
        return 1;
    }
    try {
        signal(SIGPIPE, SIG_IGN);
        signal(SIGINT, handleShutdown);
        signal(SIGTERM, handleShutdown);
        int port = validPort(static_cast<std::string>(av[1]));
        if (port == -1) throw (std::runtime_error(RED"Error : invalid port number  => " + static_cast<std::string>(av[1])));
        if (checkPassword(static_cast<std::string>(av[2])) == -1) throw (std::runtime_error("Error : invalid password please enter a password in range of [5 to 9] printable characters!"));
        Server server(port, av[2]);
        server.run();
    } catch (const std::exception& e) {
        std::cerr << RED << "Error: " << e.what() << END << std::endl;
        return 1;
    }

    return 0;
}
