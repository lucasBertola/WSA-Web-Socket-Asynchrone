#ifndef WEBSOCKET_H
#define WEBSOCKET_H

#include <iostream>
#include <winsock2.h>
class WebSocket
{
    public:
        //TODO : URL PARSER
        WebSocket(std::string url,unsigned int port);

        void onmessage(void (*f)(std::string));

        virtual ~WebSocket();

        //|!| do not use this fonction |!|
        std::string getMessage();

        void (*onmessageFonction)(std::string);

    private:

        std::string unmask(std::string reponce)

        unsigned int port;
        std::string url;

        SOCKET sock;
        SOCKADDR_IN sin;

        void createSocket();
        void ConnectSocket();
        void handshake();
        void sendMessage(std::string message);

};

#endif // WEBSOCKET_H
