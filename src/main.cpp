#include <iostream>
#include "WebSocket.h"

void My_Fonction_Who_use_messge_server(std::string messageServer) {
    /*
        Do some stuff
    */
    #ifdef _DEBUG
                    std::cout<<"on rentre dans la My_Fonction_Who_use_messge_server"<<std::endl;
    #endif
    std::cout<<messageServer<<std::endl;
}

int main()
{
    WebSocket websocket ("192.168.1.12",9000);
    websocket.onmessage(My_Fonction_Who_use_messge_server);

    int a;
    std::cin>>a;

    return 0;
}
