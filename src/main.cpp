#include <iostream>
#include "WebSocket.h"

void My_Fonction_Who_use_messge_server(std::string messageServer) {
    /*
        Do some stuff
    */
    std::cout<<messageServer<<std::endl;
}

int main()
{
    WebSocket websocket ("192.168.1.12",9000);
    //WebSocket websocket ("192.168.1.12",1337);
    websocket.onmessage(My_Fonction_Who_use_messge_server);

    std::string message;
    while(1) {
        std::cin>>message;
        websocket.sendMsg(message);
    }
    return 0;
}
