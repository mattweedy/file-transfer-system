# File Transfer System
A lightweight client-server application for transferring files over TCP. Designed for simplicity and reliability, this system enables users to send and receive files between machines on a local network.
## Features
- **Client-Server Architecture:** Structured with a clear separation between client and server components.
- **TCP-Based Communication:** Ensures reliable file transfers over TCP sockets.
- **Directory Management:** Organised directories for source code (`src/`), server files (`server/`), and transferred files (`files/`).
- **Cross-Platform Compatibility:** Developed in C for use on Unix-based systems.
## Prerequisites
- **Operating System:** Unix-based system (e.g., Linux, macOS).
- **Compiler:** GCC or any standard C compiler.
## Installation & Usage
1. Clone the repository:
```
git clone https://github.com/mattweedy/file-transfer-system.git
cd file-transfer-system
```
2. Compile the source code:
```
gcc src/server.c -o server
gcc src/client.c -o client
```
3. Run the server:
```
./server
```
4. Run the client:
```
./client
```
Ensure the server is running before starting the client.
## Project Structure
```
file-transfer-system/
├── src/            # Source code for client and server
├── server/         # Server-related files and configurations
├── files/          # Directory for storing transferred files
├── .vscode/        # Visual Studio Code configurations
├── .gitignore      # Git ignore file
└── README.md       # Project documentation
```
## Future Enhancements
- Implement encryption for secure file transfers.
- Add support for concurrent client connections.
- Introduce a graphical user interface (GUI) for ease of use.
## License
This project is licensed under the MIT License.
