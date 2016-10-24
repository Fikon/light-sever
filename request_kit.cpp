/*
** Author:  kiky.jiang@gmail.com
** Date:  2016-10-21
*/

#include "request_kit.h"
#include<iostream>
#include<ctype.h>
#include<string.h>


size_t read_line(int c_sock, char *buffer, size_t buf_size){
  size_t i=0;
  char c;
  size_t n;
  while((i < buf_size - 1)){
    recv(c_sock, &c, 1, 0);
    if(c == '\r'){
      buffer[i++] = c;
      recv(c_sock, &c, 1, 0);
      if(c == '\n'){
        buffer[--i] = '\0';
        return i;
      }
    }
    buffer[i++] = c;
  }
  return i;
}

void get_requestline(int c_sock, char *method, char *url, char *query_string, char *http_version){
  char buffer[4096];
  size_t size;
  size_t i;
  size_t j;
  char *ptr = NULL;
  size = read_line(c_sock, buffer, sizeof(buffer));
  if(size == 0){
    std::cout<<"Fail to read the first line of the request."<<std::endl;
    exit(1);
  }
  i = 0;
  while(i < size){
    method[i] = buffer[i];
    i++;
    if(isblank(buffer[i])){
      break;
    }
  }
  method[i] = '\0';
  i++;
  j = 0;
  while(i < size){
    url[j] = buffer[i];
    j++;
    i++;
    if(isblank(buffer[i])){
      break;
    }
  }
  url[i] = '\0';
  ptr = strchr(url, '?');
  if(ptr != NULL){
    int len = strlen(url);
    int tmp = ptr - url;
    url[ptr - url] = '\0';
    strcpy(query_string, ptr + 1);
  }
  i++;
  j = 0;
  while(i < size){
    if(buffer[i] == '/'){
      i++;
      while(i < size){
        http_version[j] = buffer[i];
        i++;
        j++;
      }
      break;
    }
    i++;
  }
  http_version[j] = '\0';
}

vector<header> get_header(int c_sock){
  vector<header> headers;
  char buf[2048];
  size_t size;
  char *ptr = NULL;
  struct header data;
  char name[128];
  char value[2048];

  size = read_line(c_sock, buf, sizeof(buf));
  while(size){
    ptr = strchr (buf, ':');
    if(ptr == NULL){
      std::cout<<"Fail to resovle header."<<std::endl;
      exit(1);
    }
    strncpy(name, buf, ptr - buf);
    name[ptr - buf] = '\0';
    strncpy(value, ptr + 1, strlen(buf) - (ptr - buf) - 1);
    value[strlen(buf) -(ptr - buf) - 1] = '\0';
    strcpy(data.name, name);
    strcpy(data.value, value);
    headers.push_back(data);
    ptr = NULL;
    size = read_line(c_sock, buf, sizeof(buf));
  }
  return headers;
}

void get_requestdata(int c_sock, char *data, int len){
  size_t size;
  char c;
  int i;
  i = 0;
  while(i < len){
    recv(c_sock, &c, 1, 0);
    data[i++] = c;
  }
  data[i] = '\0';
}
