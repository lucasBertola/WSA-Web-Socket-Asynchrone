#include "WebSocket.h"
#include <time.h>
#include <stdlib.h>

#define BUFLEN 512

//Web socket protocole : http://tools.ietf.org/html/rfc6455
/*
Base Framing Protocol

0                   1                   2                   3
      0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
     +-+-+-+-+-------+-+-------------+-------------------------------+
     |F|R|R|R| opcode|M| Payload len |    Extended payload length    |
     |I|S|S|S|  (4)  |A|     (7)     |             (16/64)           |
     |N|V|V|V|       |S|             |   (if payload len==126/127)   |
     | |1|2|3|       |K|             |                               |
     +-+-+-+-+-------+-+-------------+ - - - - - - - - - - - - - - - +
     |     Extended payload length continued, if payload len == 127  |
     + - - - - - - - - - - - - - - - +-------------------------------+
     |                               |Masking-key, if MASK set to 1  |
     +-------------------------------+-------------------------------+
     | Masking-key (continued)       |          Payload Data         |
     +-------------------------------- - - - - - - - - - - - - - - - +
     :                     Payload Data continued ...                :
     + - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
     |                     Payload Data continued ...                |
     +---------------------------------------------------------------+
*/


WebSocket::WebSocket(std::string url,unsigned int port)
{
    //init random
    srand (time(NULL));

    this->port = port;
    this->url = url;

    //init socket on Windows
    WSADATA WSAData;
    if(WSAStartup(MAKEWORD(2,2),&WSAData))
    {
        std::cout<<"Erreur dans le WSAStatrup"<<std::endl;
        exit(-1);
    }
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

    createSocket();

    ConnectSocket();

    handshake();
}

int WebSocket::lengthData(std::string frame,unsigned int* indexBeginData){
     /**
    The length of the "Payload data", in bytes: if 0-125, that is the
      payload length.  If 126, the following 2 bytes interpreted as a
      16-bit unsigned integer are the payload length.  If 127, the
      following 8 bytes interpreted as a 64-bit unsigned integer (the
      most significant bit MUST be 0) are the payload length.
    **/


    //Todo : Take in order the first byte of masking
    unsigned int length = ((unsigned char) frame.c_str()[1]) & 127;
    (*indexBeginData) = 2 ;

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
        webSocket->getMessage();

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
    const char * tampon = requete.c_str();

    char bufferOutput[requete.size()];

    for(int i = 0 ; i < requete.size() ; i++) bufferOutput[i] = tampon[i];

    sendMessage(bufferOutput,requete.size());

    //std::cout<<bufferOutput<<std::endl;

    //getUpgrade();
    getMessage(1);
    //TODO VERIFIER LA Sec-WebSocket-Accept

    //launch listener thread.
    DWORD threadID;
    CreateThread(NULL, 0, listener, (LPVOID)this, 0, &threadID);
}

void WebSocket::sendMsg(std::string msg) {

    /**
        See the Base Framing Protocol ( on the top)
    **/

    unsigned int length = msg.size();

    unsigned int nbOctet = 0;
    unsigned int maskIndex = 0;
    char *message = NULL ;

    /**
        The length of the "Payload data", in bytes: if 0-125, that is the
        payload length.  If 126, the following 2 bytes interpreted as a
        16-bit unsigned integer are the payload length.  If 127, the
        following 8 bytes interpreted as a 64-bit unsigned integer (the
        most significant bit MUST be 0) are the payload length.

    */

	if(length <= 125) {
        nbOctet = 1+1+4+ length;
        message = new char[nbOctet];

        //one byte which contains the length
        message[1] = length ;

	}else if(length > 125 && length < 65536) {
        nbOctet = 1+3+4+length;

        message = new char[nbOctet];

        message[1] = 126 ;

        message[2] = 0;
        message[3] = length;

	}else if(length >= 65536){
        nbOctet =1+9+4+ length;

        message = new char[nbOctet];

        message[1] = 127 ;
	}

	/**
	 Mask:  1 bit

      Defines whether the "Payload data" is masked.  If set to 1, a
      masking key is present in masking-key, and this is used to unmask
      the "Payload data" as per Section 5.3.  All frames sent from
      client to server have this bit set to 1.
    **/
     message[1] =  message[1] | 128;



    /**
        FIN:  1 bit
            Indicates that this is the final fragment in a message.  The first
                fragment MAY also be the final fragment.

        RSV1, RSV2, RSV3:  1 bit each

            MUST be 0 unless an extension is negotiated that defines meanings
            for non-zero values.  If a nonzero value is received and none of
            the negotiated extensions defines the meaning of such a nonzero
            value, the receiving endpoint MUST _Fail the WebSocket
            Connection_.

   Opcode:  4 bits

      Defines the interpretation of the "Payload data".  If an unknown
      opcode is received, the receiving endpoint MUST _Fail the
      WebSocket Connection_.  The following values are defined.

      *  %x1 denotes a text frame

   **/
    message[0] =129;




    /**
        The masking key is a 32-bit value chosen at random by the client.
    */

    maskIndex = nbOctet - length - 4;

    for(unsigned int i = maskIndex ; i < maskIndex+4 ; i++)
        message[i] = rand() % 255 ;


    /**
        Masking :
        Octet i of the transformed data ("transformed-octet-i") is the XOR of
        octet i of the original data ("original-octet-i") with octet at index
        i modulo 4 of the masking key ("masking-key-octet-j"):

         j                   = i MOD 4
         transformed-octet-i = original-octet-i XOR masking-key-octet-j
     */

    //the actual data
    for(unsigned int i = 0 ; i < msg.size() ; i++)
            message[maskIndex+4+i] = (msg.c_str()[i] ^ message[i%4+maskIndex]);

    sendMessage(message , nbOctet);

    delete[] message;

}
void WebSocket::sendMessage(char bufferOutput[] ,unsigned int size) {

    unsigned int nbEnvoyer = 0;
    int erreur = -1;

    while( nbEnvoyer < size )
    {
        erreur = send(sock,bufferOutput+nbEnvoyer,size-nbEnvoyer,0);
        nbEnvoyer += erreur;
        if(erreur==0||erreur==-1)
        {
            std::cout<<"Erreur dans l'envois de la requette...La connexion a t elle pas été fermé?. "<<erreur<<std::endl;//TODO GERER UNE RECONNEXION
            exit(-1);
        }
    }
}


bool WebSocket::checkUpgrade(std::string reponce) {
    //std::cout<<reponce<<std::endl;
    //TODO
    return true;
}
void WebSocket::getMessage(int type) {

    std::string chaine = "";
    fd_set readfs;
    FD_ZERO(&readfs);
    FD_SET(sock,&readfs);
    int nb = select(sock+1,&readfs,NULL,NULL,NULL);

    if(nb == -1)
    {
        #ifdef _DEBUG
                    std::cout<<"Connexion fermer car fermeture connexion"<<std::endl;
        #endif
         //ConnectSocket();
         return ;
    }else
    {
        int result = -1;

        unsigned int nbDonnerRecu = 0;
        unsigned int lengtData = 1;
        unsigned int indexBeginData = 0;

        while(1){
            char recvbuf[BUFLEN];
            unsigned int recvbuflen = BUFLEN;

            result = recv(sock,recvbuf,recvbuflen,0);

            //get null message
            if (result == 0) return;

            //Si we want the upgrade
            if(type == 1) {
                for(int i = 0 ; i < result ; i++)
                    chaine += recvbuf[i];

                size_t positionStop = chaine.find("\r\n\r\n");
                if(positionStop != std::string::npos) {
                    //if we are get only the upgrade
                    if( (positionStop + 4) == chaine.size()) {
                        checkUpgrade(chaine);
                        return;

                    }else {
                        std::string stringTampon = chaine;
                        stringTampon.replace(positionStop,stringTampon.size(),"");
                        checkUpgrade(stringTampon);

                        //change the type for that requet get catch by the if ( if type == 0)
                        //and the nb of octet get.(result)

                        type = 0;
                        result = chaine.size() - positionStop -4 ;

                        chaine.replace(0,positionStop+4,"");

                        const char *  tampon = chaine.c_str();

                        for(int i = 0 ; i < result ; i++)
                            recvbuf[i] = tampon[i];

                        chaine = "";
                    }

                }

            }

            //for know if it's message or ping
            if(type == 0) {
                /*
                %x0 denotes a continuation frame

              *  %x1 denotes a text frame

              *  %x2 denotes a binary frame

              *  %x3-7 are reserved for further non-control frames

              *  %x8 denotes a connection close

              *  %x9 denotes a ping

              *  %xA denotes a pong

              *  %xB-F are reserved for further control frames

              */

                unsigned int opcode = ((unsigned int) (unsigned char )recvbuf[0] ) & 0xF;

                //text frame
                if(opcode == 0x1)
                    type = 2;
                //ping
                else if (opcode == 0x9) {
                    type = 3;
                }
            }
            //Si we want the message
            if(type == 2) {
                if (transformeRequetteMsg(result,chaine, recvbuf,nbDonnerRecu,lengtData,indexBeginData)) return;
            }

            if(type == 3) {
                sendPong();
                return;
            }


        }

        return ;
    }
}
void WebSocket::sendPong() {
    char pong[2];
    pong[0] = 0x8A;
    pong[1] = 0x0;
    sendMessage(pong,2);
}
bool WebSocket::transformeRequetteMsg(int &result,std::string & chaine,char recvbuf[],unsigned int &nbDonnerRecu ,unsigned int &lengtData,unsigned int &indexBeginData){

    for(int i = 0 ; i < result ; i++)
        chaine += recvbuf[i];

    //If it's the first frame
     if(nbDonnerRecu == 0) {
        //check the length of input data
        lengtData = lengthData(chaine , &indexBeginData);
        //and cut before the data
        chaine.erase(0,indexBeginData);
     }
     nbDonnerRecu += result;
    while((nbDonnerRecu >= (lengtData + indexBeginData))) {

         if(nbDonnerRecu == (lengtData + indexBeginData)) {
            //TODO ouvrir un thread expres pour lui
            onmessageFonction(chaine);
            return true;

         }else if (nbDonnerRecu > (lengtData + indexBeginData)){
            std::string stringTampon = chaine;
            stringTampon.replace(lengtData ,stringTampon.size(),"");
            onmessageFonction(stringTampon);

            //and the nb of octet get.(result)
            result = chaine.size() - stringTampon.size() ;

            chaine.replace(0,stringTampon.size(),"");

            const char *  tampon = chaine.c_str();

            for(int i = 0 ; i < result ; i++)
                recvbuf[i] = tampon[i];


            chaine = "";
            nbDonnerRecu = 0;
            lengtData = 1;
            indexBeginData = 0;

            return transformeRequetteMsg(result,chaine, recvbuf,nbDonnerRecu,lengtData,indexBeginData);
         }
    }
    return false;
}

WebSocket::~WebSocket()
{
    closesocket(sock);

    WSACleanup();
}
