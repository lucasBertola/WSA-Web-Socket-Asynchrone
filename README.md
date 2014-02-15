# WSA : Web Socket Asynchrone

WSA is an implementation of an Web Socket client in c++.

Basic Usage
===============
```c++
	#include "WebSocket.h"

	void My_Fonction_Who_use_messge_server(std::string messageServer) {
	
		/*
			Do some stuff
		*/
		std::cout<<messageServer<<std::endl;
	}

	int main()
	{
		WebSocket websocket ("127.0.0.1",9000);
		websocket.onmessage(My_Fonction_Who_use_messge_server);

		std::string message;
		while(1) {
			std::cin>>message;
			websocket.sendMsg(message);
		}
		return 0;
	}


```

Advanced Usage
===============

