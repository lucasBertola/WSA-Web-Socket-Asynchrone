#include "WebSocket.h"

#define BUFLEN 512

WebSocket::WebSocket(std::string url,unsigned int port)
{
    this->port = port;
    this->url = url;

    WSADATA WSAData;
    if(WSAStartup(MAKEWORD(2,2),&WSAData))
    {
        std::cout<<"Erreur dans le WSAStatrup"<<std::endl;
        exit(-1);
    }

    createSocket();
    ConnectSocket();
    handshake();
}

void WebSocket::createSocket(){
    struct hostent *hostinfo = NULL;
    const char *hostname = this->url.c_str();
    hostinfo = gethostbyname(hostname); /* on récupère les informations de l'hôte auquel on veut se connecter */
    if (hostinfo == NULL) /* l'hôte n'existe pas */
    {
        std::cout<<"hostname est inconnu : "<<hostname<<std::endl;
        exit(-1);
    }

    sock = socket(AF_INET,SOCK_STREAM,0);
    sin.sin_addr = *(IN_ADDR *) hostinfo->h_addr;
    sin.sin_family = AF_INET;
    sin.sin_port = htons(this->port);
}

void WebSocket::ConnectSocket(){
    if ( connect(sock,(SOCKADDR *)&sin,sizeof(sin)) == SOCKET_ERROR)
    {
        std::cout<<"Error in the connection..."<<std::endl;
        exit(-1);
    }
}
void WebSocket::onmessage(void(*fonction)(std::string)) {
    onmessageFonction = fonction;
}
DWORD WINAPI listener(LPVOID lpParameter)
{
    WebSocket *webSocket = (WebSocket*)lpParameter;
    while(1)
        webSocket->onmessageFonction(webSocket->getMessage());

    return 0;
}
void WebSocket::handshake(){
    std::string requete = "";
    requete += "GET / HTTP/1.1\r\n";
    requete += "Host:"+url+"\r\n";
    requete += "User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10.7; rv:27.0) Gecko/20100101 Firefox/27.0\r\n";
    requete += "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n";
    requete += "Accept-Language: fr,fr-fr;q=0.8,en-us;q=0.5,en;q=0.3\r\n";
    requete += "Accept-Encoding: gzip;q=0,deflate;q=0\r\n";
    requete += "Sec-WebSocket-Version: 13\r\n";
    requete += "Origin: http://"+url+"\r\n";
    //TODO : generer aleatoirement
    requete += "Sec-WebSocket-Key: tC7Iyzl5D1/hCYS9/K3BPw==\r\n";
    requete += "Connection: keep-alive, Upgrade\r\n";
    requete += "Pragma: no-cache\r\n";
    requete += "Cache-Control: no-cache\r\n";
    requete += "Upgrade: websocket\r\n";

    requete += "\r\n";

     std::cout<<requete<<std::endl;

     sendMessage(requete);

     std::cout<<getMessage()<<std::endl;
    //TODO VERIFIER LA Sec-WebSocket-Accept

    //on lance le thread qui ecoute.
    DWORD threadID;
    CreateThread(NULL, 0, listener, (LPVOID)this, 0, &threadID);
}


void WebSocket::sendMessage(std::string message) {

    char * bufferOutput = new char [message.length()+1];
    strcpy (bufferOutput, message.c_str());

    unsigned int nbEnvoyer = 0;
    int erreur = -1;

    while( nbEnvoyer < strlen(bufferOutput) )
    {
        erreur = send(sock,bufferOutput+nbEnvoyer,strlen(bufferOutput)-nbEnvoyer,0);
        nbEnvoyer+=erreur;
        if(erreur==0||erreur==-1)
        {
            std::cout<<"Erreur dans l'envois de la requette...La connexion a t elle pas été fermé?. "<<erreur<<std::endl;//TODO GERER UNE RECONNEXION
            exit(-1);
        }
    }

    delete[] bufferOutput;

}



std::string WebSocket::getMessage() {
    std::string chaine = "";
     struct timeval timeout;
     timeout.tv_sec = 200000;
     timeout.tv_usec = 0;

     fd_set readfs;
     FD_ZERO(&readfs);
     FD_SET(sock,&readfs);
     int nb = select(sock+1,&readfs,NULL,NULL,&timeout);

     if(nb==0)
     {
        #ifdef _DEBUG
                    std::cout<<"Connexion fermer car timeout"<<std::endl;
        #endif
         return 0;
     }
     else if(nb==-1)
     {
        #ifdef _DEBUG
                    std::cout<<"Connexion fermer car fermeture connexion"<<std::endl;
        #endif
         //ConnectSocket();
         return 0;
     }else
     {
         char recvbuf[BUFLEN];
         int recvbuflen = BUFLEN;

         int result = recv(sock,recvbuf,recvbuflen,0);
         for(int i = 0 ; i < result ; i++)
            chaine += recvbuf[i];
         return chaine;
     }
}

WebSocket::~WebSocket()
{
    closesocket(sock);

    WSACleanup();
}
