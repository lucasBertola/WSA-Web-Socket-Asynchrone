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

        void sendMessage(char bufferOutput[] ,  int size);
        void sendMsg(std::string message);

        virtual ~WebSocket();

        //|!| do not use this fonction |!|
        std::string getMessage();

        void (*onmessageFonction)(std::string);

    private:

        unsigned int port;
        std::string url;

        SOCKET sock;
        SOCKADDR_IN sin;

        int lengthData(std::string reponce , unsigned int* indexBeginData);
        std::string getUpgrade();

        void createSocket();
        void ConnectSocket();
        void handshake();


};

#endif // WEBSOCKET_H
