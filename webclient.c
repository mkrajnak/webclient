/**
* Project 1 for IPK course: webclient
* Author: Martin Krajnak xkrajn02@stud.fit.vutbr.cz
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include <regex.h>

// http:\/\/.+:([[:digit:]]+)          port number
// ^http:\/\/.+\/(.+)$                 some\ text.txt
// http:\/\/([\w\.]*)\/                www.fit.vutbr.cz
// http:\/\/.*(\/[\w\.\/]*\/)          /study/courses/IPK/public/


#define URL_RGX "http:\\/\\/([\\w\\.]*)\\/"
#define PATH_RGX "http:\\/\\/.*(\\/[\\w\\.\\/]*\\/)"
#define FILENAME_RGX "^http:\\/\\/.+\\/(.+)$"
#define PORT_NUMBER_RGX "http:\\/\\/.+:([[:digit:]]+)"

#define DEFAULT_FILE_NAME = "index.html";
#define FIT = "http://www.fit.vutbr.cz";

int main(int argc, char **argv)
{
  // if (argc != 2) {
  //   fprintf(stderr,"Invalid number of args\n");
  //   return -1;
  // }

  struct hostent *web_address;
  web_address = gethostbyname("www.fit.vutbr.cz");

  if ( web_address == NULL) {                         //check if translation was succesfull
    fprintf(stderr,"DNS error\n");
    return -1;
  }

  // in_addr is struct required by inet_ntoa
  // which is needed to translate from network byte order
  struct in_addr ip_addr;
  memcpy(&ip_addr, web_address->h_addr_list[0], sizeof(struct in_addr));
  printf("%s\n",inet_ntoa(ip_addr));

  int mysocket;
   if((mysocket= socket(AF_INET, SOCK_STREAM, 0)) == -1) { // creating socket
     fprintf(stderr,"Could not create socket\n");
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

  char reply[1000];         // buffer fo response
  char request[1000];       // buffer which holds message to be sent
  sprintf(request, "HEAD %s HTTP/1.1\r\nHost: %s\r\nConnection: close \r\n\r\n", "/common/img/fit_logo_cz.gif", "www.fit.vutbr.cz");


  if( send(mysocket, request, strlen(request), 0) == -1) //try to send message
  {
    fprintf(stderr,"Could not send message\n");
    return -1;
  }

  if ((recv(mysocket, reply, 999, 0)) == -1)            // receive data
  {
    fprintf(stderr,"Nothing received\n");
    return -1;
  }
  //close(mysocket);
  printf("%s\n",reply);
  printf("%s:%c\n",argv[0],argc );

  return 0;
}
