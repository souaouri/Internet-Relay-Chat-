# Internet Relay Chat (IRC) Server

A custom IRC server implementation written in C++98 that supports multiple clients, channels, and includes a built-in Rock-Paper-Scissors bot game.

## Features

### Core IRC Functionality
- **Multi-client support** using non-blocking sockets and poll()
- **Channel management** with proper user modes and permissions
- **User authentication** with password protection
- **Standard IRC commands** implementation
- **Operator privileges** for channel management

### Supported Commands

#### Authentication Commands
- `PASS <password>` - Authenticate with server password
- `NICK <nickname>` - Set or change nickname
- `USER <username> <hostname> <servername> <realname>` - Register user information

#### Channel Commands
- `JOIN <#channel>[,<#channel2>] [key[,key2]]` - Join one or more channels
- `TOPIC <#channel> [new topic]` - View or set channel topic
- `MODE <#channel> <+/-><modes> [parameters]` - Set channel modes
  - `+i/-i` - Invite-only mode
  - `+t/-t` - Topic restriction mode
  - `+k/-k <key>` - Channel key (password)
  - `+l/-l <limit>` - User limit
  - `+o/-o <nickname>` - Operator privileges
- `KICK <#channel> <nickname> [reason]` - Kick user from channel
- `INVITE <nickname> <#channel>` - Invite user to channel

#### Communication Commands
- `PRIVMSG <target> <message>` - Send message to user or channel
- `QUIT [message]` - Disconnect from server

#### Bot Commands (Rock-Paper-Scissors Game)
- `RPS <target_nickname> <#channel>` - Challenge user to RPS game
- `ACCEPT` - Accept pending RPS challenge
- `CHOOSE <rock|paper|scissors> <#channel>` - Make your choice in active game

## Channel Modes

| Mode | Description | Parameter Required |
|------|-------------|--------------------|
| `+i` | Invite-only channel | No |
| `+t` | Topic can only be changed by operators | No |
| `+k` | Channel requires a key to join | Yes (key) |
| `+l` | Limit number of users in channel | Yes (limit) |
| `+o` | Give/take operator privileges | Yes (nickname) |

## Installation and Usage

### Prerequisites
- C++98 compatible compiler (g++, clang++)
- UNIX/Linux system with socket support
- Make utility

### Building the Server
```bash
make
```

### Running the Server
```bash
./ircserv <port> <password>
```

**Parameters:**
- `port`: Port number (1024-65535)
- `password`: Server password (5-9 printable characters)

**Example:**
```bash
./ircserv 6667 mypass123
```

### Connecting to the Server
You can connect using any IRC client (like HexChat, WeeChat, or irssi):

```
Server: localhost (or your server IP)
Port: <your_port>
Password: <your_password>
```

### Quick Start Example
```bash
# Terminal 1: Start server
./ircserv 6667 secret123

# Terminal 2: Connect with netcat (for testing)
nc localhost 6667
PASS secret123
NICK alice
USER alice 0 * :Alice Wonderland
JOIN #general
PRIVMSG #general :Hello everyone!
```

## Project Structure

```
├── Makefile              # Build configuration
├── README.md            # This file
├── ircserv              # Compiled executable
├── inc/                 # Header files
│   ├── Channel.hpp      # Channel and ChannelManager classes
│   ├── Client.hpp       # Client class
│   ├── Needs.hpp        # Common includes and definitions
│   ├── ReplyCodes.hpp   # IRC reply codes and messages
│   └── Server.hpp       # Server class
├── src/                 # Source files
│   ├── Channel.cpp      # Channel implementation
│   ├── Server.cpp       # Server core implementation
│   ├── client.cpp       # Client implementation
│   └── main.cpp         # Entry point
├── cmds/                # Command implementations
│   ├── INVITE.cpp       # INVITE command
│   ├── JOIN.cpp         # JOIN command
│   ├── KICK.cpp         # KICK command
│   ├── MODE.cpp         # MODE command
│   ├── NICK.cpp         # NICK command
│   ├── PASS.cpp         # PASS command
│   ├── PRIVMSG.cpp      # PRIVMSG command
│   ├── QUIT.cpp         # QUIT command
│   ├── TOPIC.cpp        # TOPIC command
│   └── USER.cpp         # USER command
├── bot/                 # Bot functionality
│   └── BOT.cpp          # Rock-Paper-Scissors game
└── .obj/                # Object files (generated)
```

## RPS Bot Game

The server includes a built-in Rock-Paper-Scissors game bot with the following features:

### How to Play
1. **Challenge someone:** `/RPS <nickname> <#channel>`
2. **Accept challenge:** `/ACCEPT` (target user)
3. **Make choices:** `/CHOOSE <rock|paper|scissors> <#channel>` (both players)
4. **Results:** Automatically announced in the channel

### Game Rules
- Challenges timeout after 2 minutes
- Both players must be in the same channel
- Only one active challenge per player at a time
- Results follow standard RPS rules

### Example Game Session
```
<alice> RPS bob #gaming
<GameBot> alice challenged bob to RPS!
<bob> ACCEPT
<GameBot> bob accepted the RPS challenge!
<alice> CHOOSE rock #gaming
<bob> CHOOSE scissors #gaming
<GameBot> RPS Result: alice wins! rock beats scissors
```

## Error Handling

The server implements comprehensive error handling for:
- Invalid commands and parameters
- Authentication failures
- Channel access restrictions
- Network errors
- Resource management

## Technical Details

- **Architecture:** Event-driven using poll() for non-blocking I/O
- **Standard:** C++98 compliant
- **Protocol:** IRC RFC-based implementation
- **Memory Management:** RAII principles with proper cleanup
- **Signal Handling:** Graceful shutdown on SIGINT/SIGTERM

## Building and Testing

```bash
# Build the project
make

# Run with test parameters
make run

# Clean object files
make clean

# Full clean (including executable)
make fclean

# Rebuild everything
make re
```

## Compliance

This implementation follows:
- IRC RFC 1459 (base protocol)
- IRC RFC 2812 (updated protocol)
- C++98 standard
- UNIX socket programming standards

## License

This project is developed as part of a programming curriculum and is intended for educational purposes.