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
    websocket.onmessage(My_Fonction_Who_use_messge_server);
    //provisional

    /*std::string message = websocket.getMessage();

    std::cout<<message<<std::endl;*/
    while(1);

    return 0;
}
