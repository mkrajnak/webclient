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
#include <regex>
using namespace std;


// http:\/\/.+:([[:digit:]]+)          port number
// ^http:\/\/.+\/(.+)$                 some\ text.txt
// http:\/\/([\w\.]*)\/                www.fit.vutbr.cz
// http:\/\/.*(\/[\w\.\/]*\/)          /study/courses/IPK/public/


regex URL_RGX ("http:\\/\\/([\\w\\.]*)\\/");
regex PATH_RGX ("http:\\/\\/.*(\\/[\\w\\.\\/]*\\/)");
regex FILENAME_RGX ("^http:\\/\\/.+\\/(.+)$");
regex PORT_NUMBER_RGX ("http:\\/\\/.+:([[:digit:]]+)");

const string DEFAULT_FILE_NAME = "index.html";
const string FIT = "http://www.fit.vutbr.cz";

int main(int argc, char **argv)
{
  // if (argc != 2) {
  //   cerr << "invalid number if arguments" << endl;
  //   return -1;
  // }

  cmatch m;

  if(regex_match ("http://www.fit.vutbr.cz:80/common/img/fit_logo_cz.gif",m,PORT_NUMBER_RGX)) {
    for (unsigned int i = 0; i < m.size(); i++) {
      cout << m.str(i) << endl;
    }
  }
  else
    cout << "no match" << endl;

  return 0;


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

  char reply[1000];         // buffer fo response
  char request[1000];       // buffer which holds message to be sent
  sprintf(request, "HEAD %s HTTP/1.1\r\nHost: %s\r\nConnection: close \r\n\r\n", "/common/img/fit_logo_cz.gif", "www.fit.vutbr.cz");


  if( send(mysocket, request, strlen(request), 0) == -1) //try to send message
  {
    cerr << "Could not send message" << endl;
    return -1;
  }

  if ((recv(mysocket, reply, 999, 0)) == -1)            // receive data
  {
    cerr << "No message received" << endl;
    return -1;
  }
  //close(mysocket);
  cout << reply << endl;
  cout << argv[0] << ":" << argc <<":" << mysocket << endl; //TODO: DELETE THIS


  return 0;
}
