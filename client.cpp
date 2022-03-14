#include <stdio.h>
#include <stdlib.h>
#include <WINSOCK2.H>
#include <iostream>
#include <string>
#include <chrono>
#include<thread>
// #include <windows.h>
#pragma comment(lib, "wsock32")
// #pragma comment(lib, "ws2_32.lib") 

#define SERVPORT 3333
#define MAXDATASIZE 1460    /*每次最大数据传输量 */
struct STEP
{
    double x;
    double y;
    double z;
    double roll;
    double pitch;
    double yaw;
};

class tcpip_port
{
    private:
        // send message
        char flag_off_ground;
        // receive message
        STEP* receiveSteps = new STEP[4];
        //port ip
        std::string mport_ip;
        //net data
        WSAData wsa;
        SOCKET sock_fd;
        int recvbytes;  
        struct sockaddr_in serv_addr;  
        int timeout = 60;
    public:
        tcpip_port(std::string port_ip, int timeout_ = 60)
        { 
            mport_ip = port_ip; 
            timeout = timeout_;
            std::cout<<"connect ip is "<<mport_ip<<std::endl; 
        }
        ~tcpip_port(){}
        bool initial();
        int sendFlag(char sendFlag);
        int receive_steps();
        void close();
};

void tcpip_port::initial()
{
    WSAStartup(MAKEWORD(2, 2), &wsa);
    if((sock_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_IP)) == INVALID_SOCKET) {
        printf("socket creat error！");
        exit(1);
    }
    serv_addr.sin_family=AF_INET;
    serv_addr.sin_port=htons(SERVPORT);
    serv_addr.sin_addr.s_addr = inet_addr(mport_ip.c_str());
    // serv_addr.sin_addr.s_addr = inet_addr("10.1.76.146");
    memset(serv_addr.sin_zero, 0, 8);
    if(connect(sock_fd, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr)) == -1) {
        printf("connect error！");
        exit(1);
    }
}

bool tcpip_port::initial()
{
    TIMEVAL Timeout;
    Timeout.tv_sec = timeout;
    Timeout.tv_usec = 0;
    struct sockaddr_in address;  /* the libc network address data structure */   

    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    address.sin_addr.s_addr = inet_addr(mport_ip.c_str()); /* assign the address */
    address.sin_port = htons(SERVPORT);            /* translate int2port num */	
    address.sin_family = AF_INET;

    //set the socket in non-blocking
    // If iMode = 0, blocking is enabled; 
    // If iMode != 0, non-blocking mode is enabled
    unsigned long iMode = 1;
    int iResult = ioctlsocket(sock, FIONBIO, &iMode);
    if (iResult != NO_ERROR)
    {	
        printf("ioctlsocket failed with error: %ld\n", iResult);
    }
        
    if(connect(sock,(struct sockaddr *)&address,sizeof(address))==false)
    {	
        return false;
    }	

    // restart the socket mode
    iMode = 0;
    iResult = ioctlsocket(sock, FIONBIO, &iMode);
    if (iResult != NO_ERROR)
    {	
        printf("ioctlsocket failed with error: %ld\n", iResult);
    }

    fd_set Write, Err;
    FD_ZERO(&Write);
    FD_ZERO(&Err);
    FD_SET(sock, &Write);
    FD_SET(sock, &Err);

    // check if the socket is ready
    select(0,NULL,&Write,&Err,&Timeout);			
    if(FD_ISSET(sock, &Write)) 
    {	
        if (FD_ISSET(sock, &Write))
        {
            iMode = 0;
            int iResult = ioctlsocket(sock, FIONBIO, &iMode);
            if (iResult != NO_ERROR)
            {	
                printf("ioctlsocket failed with error: %ld\n", iResult);
                return false;
            }
        }
        return true;
    }
    return false;
}

int tcpip_port::sendFlag(char sendFlag)
{
    if(send(sock_fd, &sendFlag, sizeof(sendFlag), 0) ==-1)
    {
        printf("send error");
        exit(1);
    }
    else
        return 0;
}

int tcpip_port::receive_steps()
{
    // 24 * 8
    char temp[192];   
    memset(temp, 0, sizeof(temp));
    if((recvbytes=recv(sock_fd, temp, sizeof(temp), 0)) == -1) {
        printf("recv error！");
        exit(1);
    }
    double msg[24] = {0.0};
    memcpy(msg,temp,sizeof(msg));
    for(int i = 0; i < 4; i++)
    {
        receiveSteps[i].x = msg[i*6];
        receiveSteps[i].y = msg[i*6 + 1];
        receiveSteps[i].z = msg[i*6 + 2];
        receiveSteps[i].roll = msg[i*6 + 3];
        receiveSteps[i].pitch = msg[i*6 + 4];
        receiveSteps[i].yaw = msg[i*6 + 5];
    }
    printf("Received steps.");
    return 0;
}

void tcpip_port::close()
{
    closesocket(sock_fd);
	WSACleanup();
}

int main(int argc, char *argv[])
{
    tcpip_port tcpipPort("10.1.76.146");
	tcpipPort.initial();
    while(1)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        tcpipPort.sendFlag('A');
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        tcpipPort.sendFlag('B');
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        tcpipPort.receive_steps();
    }
    tcpipPort.close();
    return 0;
}