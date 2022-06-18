/*
 * @Author: MRL Liu
 * @Date: 2022-04-05 22:55:31
 * @Description: ��
 * @LastEditTime: 2022-04-06 14:11:05
 * @FilePath: \C++\NetCode\socket_epoll.cpp
 */
#include<unistd.h>
#include<errno.h> 
#include<string.h> 
#include<fcntl.h> 
#include<stdlib.h> 
#include<assert.h>
#include<stdio.h> 
#include<sys/epoll.h>

#define MAX_EVENT_NUMBER 1024 
#define BUFFER_SIZE 10 
/*���ļ����������óɷ�������*/
int setnonblocking(int fd) {
    int old_option=fcntl(fd,F_GETFL); 
    int new_option=old_option|O_NONBLOCK; 
    fcntl(fd,F_SETFL,new_option); 
    return old_option; 
}

/*���ļ�������fd�ϵ�EPOLLINע�ᵽepollfdָʾ��epoll�ں��¼����У����� enable_etָ���Ƿ��fd����ETģʽ*/ 
void addfd(int epollfd,int fd,bool enable_et) {
    epoll_event event; 
    event.data.fd=fd; 
    event.events=EPOLLIN; 
    if(enable_et) {event.events|=EPOLLET; }
    epoll_ctl(epollfd,EPOLL_CTL_ADD,fd,&event); 
    setnonblocking(fd); 
}
/*LTģʽ�Ĺ�������*/ 
void lt(epoll_event*events,int number,int epollfd,int listenfd) {
    char buf[BUFFER_SIZE]; 
    for(int i=0;i<number;i++) {
        int sockfd=events[i].data.fd; 
        if(sockfd==listenfd) {
            struct sockaddr_in client_address; 
            socklen_t client_addrlength=sizeof(client_address); 
            int connfd=accept(listenfd,(struct sockaddr*)&client_address, &client_addrlength); 
            addfd(epollfd,connfd,false);
            /*��connfd����ETģʽ*/ 
        }else if(events[i].events &EPOLLIN){
            /*ֻҪsocket�������л���δ���������ݣ���δ���ͱ�����*/ 
            printf("event trigger once\n"); 
            memset(buf,'\0',BUFFER_SIZE); 
            int ret=recv(sockfd,buf,BUFFER_SIZE-1,0); 
            if(ret<=0) {
                close(sockfd); 
                continue; 
            }
            printf("get%d bytes of content:%s\n",ret,buf); 
        }else {
            printf("something else happened\n"); 
        }
    }
}
/*ETģʽ�Ĺ�������*/ 
void et(epoll_event*events,int number,int epollfd,int listenfd) {
    char buf[BUFFER_SIZE]; 
    for(int i=0;i<number;i++) {
        int sockfd=events[i].data.fd; 
        if(sockfd==listenfd) {
            struct sockaddr_in client_address; 
            socklen_t client_addrlength=sizeof(client_address); 
            int connfd=accept(listenfd,(struct sockaddr*)&client_address,& client_addrlength); 
            addfd(epollfd,connfd,true);
            /*��connfd����ETģʽ*/ 
        }else if(events[i].events&EPOLLIN) {
            /*��δ��벻�ᱻ�ظ���������������ѭ����ȡ���ݣ���ȷ����socket�������е����� ���ݶ���*/ 
            printf("event trigger once\n"); 
            while(1) {
                memset(buf,'\0',BUFFER_SIZE); 
                int ret=recv(sockfd,buf,BUFFER_SIZE-1,0); 
                if(ret<0) {
                    /*���ڷ�����IO�����������������ʾ�����Ѿ�ȫ����ȡ��ϡ��˺�epoll�����ٴδ� ��sockfd�ϵ�EPOLLIN�¼�����������һ�ζ�����*/ 
                    if((errno==EAGAIN)||(errno==EWOULDBLOCK)) {
                        printf("read later\n");
                        break; 
                    }
                    close(sockfd); 
                    break; 
                }else if(ret==0) {
                    close(sockfd); 
                }else {
                    printf("get%d bytes of content:%s\n",ret,buf); 
                }
            }
        }else {
            printf("something else happened\n"); 
        }
    }
}


int main(int argc,char*argv[]) {
    if(argc<=2) {
        printf("usage:%s ip_address port_number\n",basename(argv[0])); 
        return 1; 
    }
    
    return 0; 
}
        