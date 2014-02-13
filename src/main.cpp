#include <iostream>
#include "WebSocket.h"

int main()
{
    WebSocket websocket ("192.168.1.12:9000/demo/server.php");
    std::cout << "Hello world!" << std::endl;
    return 0;
}
