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

/**
* REGEXES
*/
#define URL_RGX "(www.[a-zA-z0-9\\.]*)"
#define FILENAME_RGX "([0-9A-Za-z\\.\\_\\ ]*)$"
#define PORT_NUMBER_RGX "(:{1}[0-9]+)"
#define WHITE_SPACE "[\\ ]"

#define DEFAULT_FILE_NAME "index.html"

#define HTTPV11 "1.1"
#define HTTPV10 "1.0"

struct url_info_t{
  char * basic_url;
  char * filename;
  int port_number;
};

/*
* Fnction returns string matches by regex
*/
char * apply_rgx(char * rgx, char * string)
{
    regex_t r;
    if ((regcomp (&r, rgx, REG_EXTENDED|REG_NEWLINE)) != 0 ) {    // compile regex
      fprintf(stderr, "FAILED\n" );
      return NULL;
    }

    regmatch_t matches[1];
    if (regexec (&r, string, 1, matches, 0)) {                     // try to match
      fprintf(stderr, "NO MATCH\n" );
      return NULL;
    }

    char *result;                                       //alloc space for string
    if (( result = (char*)malloc(matches[0].rm_eo - matches[0].rm_so)) == NULL){
      fprintf(stderr, "Allocation error\n" );
      return NULL;
    }
    //copy result
    strncpy(result, &string[matches[0].rm_so], matches[0].rm_eo - matches[0].rm_so);

    // printf("a matched substring \"%s\" is found at position %d to %d.\n",
    //        result, matches[0].rm_so, matches[0].rm_eo - 1);

    regfree(&r);
    return result;
}

/*
* Parse url string to struct
*/
int parse_url(struct url_info_t * url, char * s_url)
{
  url->basic_url = apply_rgx(URL_RGX, s_url);
  url->filename = apply_rgx(FILENAME_RGX, s_url);


  if (!strcmp(url->filename, "") || !strcmp(url->filename, url->basic_url )) {
    if (( url->filename = (char*)malloc(2)) == NULL){
      fprintf(stderr, "Allocation error\n" );
      return -1;
    }
    strncpy(url->filename, "/", sizeof("/"));
  }
  //char * temp_port = apply_rgx(PORT_NUMBER_RGX,FIT);
  url->port_number = 80;
  return 0;
}

/*
* Get the http protocol version from message
*/
char *get_version (char *reply)
{
  char  *version = (char *)malloc(sizeof(4));
  strncpy(version, &reply[5], 3);
  version[3] = '\0';
  return version;
}

/**
* Detect and repair white spaces
*/
char * whitespaces(char * url)
{
  regex_t r;
  if ((regcomp (&r, WHITE_SPACE, REG_EXTENDED|REG_NEWLINE)) != 0 ) {    // compile regex
    fprintf(stderr, "FAILED\n" );
    return NULL;
  }

  char *result;                                       //alloc space for string
  if (( result = (char*)malloc(sizeof(url)+10)) == NULL){
     fprintf(stderr, "Allocation error\n" );
  return NULL;
   }
  regmatch_t matches[5];
  if(!regexec (&r, url, 1, matches, 0)) {                     // try to match

    printf("lel\n" );
    strncpy(result,&url[0],matches[0].rm_so);
    strcat(result,"%20");
    strcat(result,&url[matches[0].rm_so + 1]);
    printf("%s\n",result);
  }
  return result;
  regfree(&r);
}


int main(int argc, char **argv)
{
  // if (argc != 2) {
  //   fprintf(stderr,"Invalid number of args\n");
  //   return -1;
  // }
  // struct url_info_t  *url;
  //
  // if ((url = (struct url_info_t *)malloc(sizeof(struct url_info_t))) == NULL) {
  //   fprintf(stderr, "Alloc error\n" );
  //   return -1;
  // }
  // printf("%s\n", argv[1] );
  // parse_url(url, argv[1]);
  //
  // printf("%d \n",url->port_number );
  // printf("%s \n",url->basic_url );
  // printf("%s \n",url->filename );
  //
  // free(url->basic_url);
  // free(url->filename);
  // free(url);
  //
  // printf("%s\n", whitespaces (argv[1]));
  //
  // return 0;

  struct hostent *web_address;
  web_address = gethostbyname("www.fit.vutbr.cz");

  if ( web_address == NULL) {                         //check if translation was succesfull
    fprintf(stderr,"ERR: %s\n", strerror(errno));
    return -1;
  }
  // in_addr is struct required by inet_ntoa
  // which is needed to translate from network byte order
  struct in_addr ip_addr;
  memcpy(&ip_addr, web_address->h_addr_list[0], sizeof(struct in_addr));
  printf("%s\n",inet_ntoa(ip_addr));

  int mysocket;
   if((mysocket = socket(AF_INET, SOCK_STREAM, 0)) == -1) { // creating socket
     fprintf(stderr,"ERR: %s\n", strerror(errno));
     return -1;
   }

  struct sockaddr_in dest;
  memset(&dest, 0, sizeof(dest));                       // setting up struct for connect
  dest.sin_family = AF_INET;
  dest.sin_addr.s_addr = inet_addr(inet_ntoa(ip_addr)); // setting properly destination ip address
  dest.sin_port = htons(80);                            // set destination port

  if(connect(mysocket, (struct sockaddr *)&dest, sizeof(struct sockaddr)) == -1 )
  {
    fprintf(stderr,"ERR: %s\n", strerror(errno));
    return -1;
  }

  char reply[1000];         // buffer fo response
  char request[1000];       // buffer which holds message to be sent
  sprintf(request, "HEAD %s HTTP/1.1\r\nHost: %s\r\nConnection: close \r\n\r\n", "/common/img/fit_logo_cz.gif", "www.fit.vutbr.cz");


  if( send(mysocket, request, strlen(request), 0) == -1) //try to send message
  {
    fprintf(stderr,"ERR: %s\n", strerror(errno));
    return -1;
  }

  if ((recv(mysocket, reply, 999, 0)) == -1)            // receive data
  {
    fprintf(stderr,"ERR: %s\n", strerror(errno));
    return -1;
  }

  char * version = get_version(reply);
  printf("%s\n",version);
  printf("%s:%d\n",argv[0],argc );

  free(version);
  return 0;
}
