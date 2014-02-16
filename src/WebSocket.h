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

        void sendMessage(char bufferOutput[] ,unsigned int size);
        void sendMsg(std::string message);

        virtual ~WebSocket();

        //|!| do not use this fonction |!|
       void getMessage(int type = 0);


        void (*onmessageFonction)(std::string);



    private:

        //mutex
        HANDLE ghMutex;

        //thread;
        HANDLE threadListener;


        void sendPong();
        bool transformeRequetteMsg(int &result , std::string & chaine , char *a ,unsigned int &nbDonnerRecu , unsigned int& lengtData,unsigned int &indexBeginData);



        bool checkUpgrade(std::string);
        unsigned int port;
        std::string url;


        SOCKET sock;
        SOCKADDR_IN sin;

        int lengthData(std::string reponce , unsigned int* indexBeginData);

        void createSocket();
        void ConnectSocket();
        void handshake();


};

#endif // WEBSOCKET_H
