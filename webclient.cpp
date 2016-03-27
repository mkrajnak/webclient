/**
* Project 1 for IPK course: webclient
* Author: Martin Krajnak xkrajn02@stud.fit.vutbr.cz
*/
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>

using namespace std;

const string DEFAULT_FILE_NAME = "index.html";
const char* FIT = "http://www.fit.vutbr.cz";

// /**
// * Function handle error messages and exit programme with proper error code
// */
// void err (string text, int code)
// {
//   cerr << "ERR" << text <<endl;
//   exit(code);
// }

int main(int argc, char **argv)
{
  // if (argc != 2) {
  //     err("webclient needs exactly one argument - url",-1);
  //

  // hostent returned by gethostbyname contains
  // ip address translated from domain name
  struct hostent *web_address;
  web_address = gethostbyname("www.fit.vutbr.cz");

  if ( web_address == NULL) {                         //check if translation was succesfull
    cout << "err" << endl;
    return -1;
  }

  // in_addr is struct required by inet_ntoa
  // which is needed to translate from network byte order
  struct in_addr ip_addr;
  memcpy(&ip_addr, web_address->h_addr_list[0], sizeof(struct in_addr));
  printf("%s\n",inet_ntoa(ip_addr));

  int mysocket;
   if((mysocket= socket(AF_INET, SOCK_STREAM, 0)) == -1) { // creating socket
     cerr << "Could not create socket" << endl;
     return -1;
   }

  struct sockaddr_in dest;
  memset(&dest, 0, sizeof(dest));                       // setting up struct for connect
  dest.sin_family = AF_INET;
  dest.sin_addr.s_addr = inet_addr(inet_ntoa(ip_addr)); // setting properly destination ip address
  dest.sin_port = htons(80);                            // set destination port

  if(connect(mysocket, (struct sockaddr *)&dest, sizeof(struct sockaddr)) == -1 )
  {
    return -1;
  }

  char reply[1000];
  char request[1000];
  sprintf(request, "Get %s HTTP/1.1\r\n Host: %s\r\n \r\n \r\n", "www.fit.vutbr.cz", "/common/img/fit_logo_cz.gif");
  while(true)
    {
        if( send(mysocket, request, strlen(request), 0) == -1) //try to send message
        {
          cerr << "Could not send message" << endl;
          return -1;
        }

        if( recv(mysocket, reply, 2000, 0) == -1)     //listen for response
        {
          cerr << "No response" << endl;
          return -1;
        }
        cout << reply << endl;
        getchar();
    }

  //close(mysocket);
  cout << argv[0] << ":" << argc <<":" << mysocket << endl; //TODO: DELETE THIS


  return 0;
}
