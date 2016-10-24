/*
**  Author: kiky.jiang@gmail.com
**  Date: 2016-10-20
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
#include "request_kit.h"
#include <sys/stat.h>
#include <fcntl.h>
#include<string>

using namespace std;

#define STATUS_OK 1
#define STATUS_NOTFOUND 2
#define STDIN   0
#define STDOUT  1
#define STDERR  2


void handle_request(int p_sock);

void return_file(int p_sock, char *path);

void execute(int p_sock, char *path, char *method, char *param);

void not_found(int p_sock, char *path);

void send_header(int c_sock, int stat);

void print_name(int p_sock);

int main(int argc, char const *argv[]) {
  /* code */
  int s_sock;
  int p_sock;
  struct addrinfo hints;
  struct addrinfo *servinfo;
  struct addrinfo *tmp;
  struct sockaddr_storage p_addr;
  socklen_t sin_size;
  struct sigaction sa;
  int yes;
  int rv;
  char s[INET6_ADDRSTRLEN];

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;
  if((rv = getaddrinfo(NULL, argv[1], &hints, &servinfo)) !=0){
    cout<<"Fail to get server information!"<<endl;
    exit(1);
  }
  for(tmp = servinfo; tmp != NULL; tmp = tmp->ai_next){
    if((s_sock = socket(tmp->ai_family, tmp->ai_socktype, tmp->ai_protocol)) == -1){
      continue;
    }
    if(setsockopt(s_sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1){
      cout<<"Fail to set socket option!"<<endl;
      exit(1);
    }
    if(bind(s_sock, tmp->ai_addr, tmp->ai_addrlen) == -1){
      close(s_sock);
      continue;
    }
    break;
  }

  freeaddrinfo(servinfo);

  if(listen(s_sock, 10) == -1){
    cout<<"Fail to listen!"<<endl;
    exit(1);
  }

  cout<<"Server start, listening at port: "<<argv[1]<<endl;

  while(true){
    sin_size = sizeof(p_addr);
    p_sock = accept(s_sock, (struct sockaddr *)&p_addr, &sin_size);
    if(p_sock == -1){
      cout<<"Fail to accept peer's request!"<<endl;
      exit(1);
    }
    /*inet_ntop(p_addr.ss_family, (struct sockaddr*)&p_addr, s, sizeof(s));
    cout<<"Server got connection from "<<s<<endl;*/

    handle_request(p_sock);
  }
  return 0;
}

void handle_request(int p_sock){
  char method[128];
  char url[1024];
  char version[256];
  char query[1024] = "\0";
  char path[1024] = "\0";
  struct stat st;
  char tmp[128];
  get_requestline(p_sock, method, url, query, version);
  sprintf(path, "www%s", url);
  if(lstat(path, &st) < 0){
    sprintf(tmp, "www/404.html");
    not_found(p_sock, tmp);
  }
  if(strcasecmp(method, "GET") == 0){
    if(S_ISDIR(st.st_mode)){
      strcat(path, "/index.html");
      return_file(p_sock, path);
    }
    else if(S_ISREG(st.st_mode)){
      if(strlen(query) != 0){
        execute(p_sock, path, method, query);
      }
      else{
        return_file(p_sock, path);
      }
    }
  }
  else if(strcasecmp(method, "POST") == 0){
    print_name(p_sock);
  }

}

void return_file(int c_sock, char *path){
  int doc = open(path, O_RDONLY);
  char buf[1024];
  size_t size;
  send_header(c_sock, STATUS_OK);
  while(size = read(doc, buf,sizeof(buf))){
    send(c_sock, buf, size, 0);
  }
}
void not_found(int c_sock, char *path){
  int doc = open(path, O_RDONLY);
  char buf[1024];
  size_t size;
  while(size = read(doc, buf,sizeof(buf))){
    send(c_sock, buf, size, 0);
  }
}

void execute(int p_sock, char *path, char *method, char *query){
  pid_t pid;
  int cgi_msg1[2];
  char buf[1024];
  int status;
  char c;
  if(pipe(cgi_msg1) < 0){
    cout<<"Failt to create pipe1"<<endl;
    exit(1);
  }
  if((pid = fork()) < 0){
    cout<<"Fail to fork thread"<<endl;
    exit(1);
  }
  if(pid == 0){
    char method_env[128];
    char query_env[2048];
    char length_env[128];
    dup2(cgi_msg1[1], STDOUT);
    close(cgi_msg1[0]);
    send_header(p_sock, STATUS_OK);
    sprintf(method_env, "REQUEST_METHOD=%s", method);
    putenv(method_env);
    if(strcasecmp(method, "GET") == 0){
      sprintf(query_env, "QUERY_STRING=%s", query);
      putenv(query_env);
    }
    execl(path, query, NULL);
  }
  else{
    close(cgi_msg1[1]);
    while(read(cgi_msg1[0], &c, 1) > 0){
      send(p_sock, &c, 1, 0);
    }
    close(cgi_msg1[0]);
    waitpid(pid, &status, 0);
  }
}

void send_header(int c_sock, int stat){
  char buf[1024];
  switch(stat){
    case 1:        //ok
      strcpy(buf, "HTTP/1.0 200 OK\r\n");
      send(c_sock, buf, strlen(buf), 0);
      sprintf(buf, "Content-Type: text/html\r\n");
      send(c_sock, buf, strlen(buf), 0);
      strcpy(buf, "\r\n");
      send(c_sock, buf, strlen(buf), 0);
     break;
    case 2:       // not found
      strcpy(buf, "HTTP/1.0 404 Not Found\r\n");
      send(c_sock, buf, strlen(buf), 0);
      sprintf(buf, "Content-Type: text/html\r\n");
      send(c_sock, buf, strlen(buf), 0);
      strcpy(buf, "\r\n");
      send(c_sock, buf, strlen(buf), 0);
      break;
  }
}

void print_name(int p_sock){
  char data[1024];
  char *ptr;
  char name[128];
  char tmp[256];
  vector<header> headers;
  int len;
  headers = get_header(p_sock);
  for(int i = 0; i < headers.size(); i++){
    if(strcasecmp(headers[i].name, "Content-Length") == 0){
      len = atoi(headers[i].value);
    }
  }
  get_requestdata(p_sock, data, len);
  ptr = strchr(data, '=');
  strcpy(name, ptr + 1);
  send_header(p_sock, STATUS_OK);
  sprintf(tmp, "<html>");
  send(p_sock, tmp, sizeof(tmp), 0);
  sprintf(tmp, "<head>");
  send(p_sock, tmp, sizeof(tmp), 0);
  sprintf(tmp, "<meta charset=\"utf-8\">");
  send(p_sock, tmp, sizeof(tmp), 0);
  sprintf(tmp, "<title>Welcome to my server.</title>");
  send(p_sock, tmp, sizeof(tmp), 0);
  sprintf(tmp, "</head>");
  send(p_sock, tmp, sizeof(tmp), 0);
  sprintf(tmp, "<body>");
  send(p_sock, tmp, sizeof(tmp), 0);
  sprintf(tmp, "<h2>Hello, %s ,now I know your name!</h2>", name);
  send(p_sock, tmp, sizeof(tmp), 0);
  sprintf(tmp, "</body>");
  send(p_sock, tmp, sizeof(tmp), 0);
  sprintf(tmp, "</html>");
  send(p_sock, tmp, sizeof(tmp), 0);
}
