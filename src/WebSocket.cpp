#include "WebSocket.h"

#define BUFLEN 2048

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

int WebSocket::lengthData(std::string frame,unsigned int* indexBeginData){
     /**
        The frames you obtain are in the following format:
        one byte which contains the type of data
        one byte which contains the length
        either two or eight additional bytes if the length did not fit in the second byte
        four bytes which are the masks (= decoding keys)
        the actual data
    **/
    //fire byte of lenth :
    unsigned int length =(unsigned char) frame.c_str()[1];
    (*indexBeginData) = 2 ;
    //TODO AMELIORER
     if (length == 126 ) {// if a special case, change indexFirstMask

        length = (((unsigned char) frame.c_str()[2]) <<8)+(((unsigned char)frame.c_str()[3]) <<0);

        length = 0;

         for(int i = 0 ; i < 2 ; i++) {
            length += (((unsigned char)frame.c_str()[3-i]) << (8*i)) ;
         }

         (*indexBeginData) = 4;

     }else if (length == 127){
         length = 0;

         for(int i = 0 ; i < 8 ; i++) {
            length += (((unsigned char)frame.c_str()[9-i]) << (8*i)) ;
         }

         (*indexBeginData) = 10 ;
     }


    return length;
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

     std::cout<<getUpgrade()<<std::endl;
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


std::string WebSocket::getUpgrade() {
    std::string chaine = "";

     fd_set readfs;
     FD_ZERO(&readfs);
     FD_SET(sock,&readfs);
     int nb = select(sock+1,&readfs,NULL,NULL,NULL);

     if(nb == 0)
     {
        #ifdef _DEBUG
                    std::cout<<"Connexion fermer car timeout"<<std::endl;
        #endif
         return "";
     }
     else if(nb == -1)
     {
        #ifdef _DEBUG
                    std::cout<<"Connexion fermer car fermeture connexion"<<std::endl;
        #endif
         //ConnectSocket();
         return "";
     }else
     {
        int result = -1;
        char recvbuf[BUFLEN];
        int recvbuflen = BUFLEN;

        result = recv(sock,recvbuf,recvbuflen,0);

        for(int i = 0 ; i < result ; i++)
            chaine += recvbuf[i];

        return chaine;
    }

}

std::string WebSocket::getMessage() {
    std::string chaine = "";

     fd_set readfs;
     FD_ZERO(&readfs);
     FD_SET(sock,&readfs);
     int nb = select(sock+1,&readfs,NULL,NULL,NULL);

     if(nb==0)
     {
        #ifdef _DEBUG
                    std::cout<<"Connexion fermer car timeout"<<std::endl;
        #endif
         return "";
     }
     else if(nb==-1)
     {
        #ifdef _DEBUG
                    std::cout<<"Connexion fermer car fermeture connexion"<<std::endl;
        #endif
         //ConnectSocket();
         return "";
     }else
     {
         int result = -1;
         int nbDonnerRecu = 0;
         unsigned int lengtData = 1;
         unsigned int indexBeginData = 0;

         while(result != 0 && nbDonnerRecu < (lengtData + indexBeginData)) {
             char recvbuf[BUFLEN];
             int recvbuflen = BUFLEN;

             result = recv(sock,recvbuf,recvbuflen,0);

             for(int i = 0 ; i < result ; i++)
                chaine += recvbuf[i];

            //If it's the first Iteration
             if(nbDonnerRecu == 0) {

                lengtData = lengthData(chaine , &indexBeginData);
                chaine.erase(0,indexBeginData);
             }
             nbDonnerRecu = result;
         }
         return chaine;
     }
}

WebSocket::~WebSocket()
{
    closesocket(sock);

    WSACleanup();
}
