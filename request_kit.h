/*
**
**  Author: kiky.jiang@gmail.com
**  Description:  some function to resovle the HTTP request.
**
*/

#include<vector>
#include<stddef.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<stdlib.h>

using namespace std;

/* store the headers */
struct header
{
    char name[128];  // name of headers
    char value[2048]; //value of headers
};

/* read a line from the request , the data is stored in buf */

size_t read_line(int c_sock, char *buf, size_t size);

/*
** read the 1st line of the request and extract the method, url and http version
** the c_sock parameter is the client socket
*/
void get_requestline(int c_sock, char *method, char *url, char *query_string, char *http_version);

/*
** read the header of the request via the socket of the client
*/
vector<header> get_header(int c_sock);

/*
** read the request data (for POST request)
*/
void get_requestdata(int c_sock, char *data, int len);
