/*
**
**  Author: kiky.jiang@gmail.com
**  Date: 2016-10-20
**
*/

#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<errno.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h>
#include<arpa/inet.h>
#include<sys/wait.h>
#include<signal.h>
#include<iostream>

using namespace std;

int main(int argc, char const *argv[]) {
  /* code */
  int c_sock;
  int numbytes;
  char buf[100];
  struct addrinfo hints;
  struct addrinfo *servinfo;
  struct addrinfo *tmp;
  int rv;
  char s[INET6_ADDRSTRLEN];
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  if((rv = getaddrinfo("localhost", argv[1], &hints, &servinfo)) != 0){
    cout<<"Fail to get target address information"<<endl;
    exit(1);
  }
  for(tmp = servinfo; tmp != NULL;tmp = tmp->ai_next){
    if((c_sock = socket(tmp->ai_family, tmp->ai_socktype, tmp->ai_protocol)) == -1){
      cout<<"Fail to construct the socket for peer!"<<endl;
      exit(1);
    }
    if(connect(c_sock, tmp->ai_addr, tmp->ai_addrlen) == -1){
      close(c_sock);
      cout<<"Fail to connect the server!"<<endl;
      continue;
    }
    break;
  }
  inet_ntop(tmp->ai_family, (struct sockaddr *)tmp->ai_addr, s, sizeof(s));
  cout<<"Connect to :"<<s<<endl;
  freeaddrinfo(servinfo);
  if((numbytes = recv(c_sock, buf, 99, 0)) == -1){
    cout<<"Fail to recieve msg!"<<endl;
    exit(1);
  }
  buf[numbytes] = '\0';
  cout<<"Recerved: "<<buf<<endl;
  close(c_sock);
  return 0;
}
