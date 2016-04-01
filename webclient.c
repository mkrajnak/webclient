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
#define DEFAULT_PORT_NUMBER 80
#define FIT "www.fit.vutbr.cz"
#define PREFIX "http://"
#define SLASH '/'
#define SLASH_STR "/"

#define HTTPV11 "1.1"
#define HTTPV10 "1.0"

struct url_info_t{
  char * address;
  char * base_address;
  char * path;
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
      //fprintf(stderr, "NO MATCH\n" );
      return NULL;
    }

    char *result;                                       //alloc space for string
    if (( result = (char*)malloc(strlen(&string[matches[0].rm_so])-1)) == NULL){
      fprintf(stderr, "Allocation error\n" );
      return NULL;
    }
    //copy result
    strncpy(result, &string[matches[0].rm_so], matches[0].rm_eo - matches[0].rm_so );

    //  printf("a matched substring \"%s\" is found at position %d to %d.\n",
    //        result, matches[0].rm_so, matches[0].rm_eo - 1);

    regfree(&r);
    return result;
}
/*
* check whether certain character is present within string
*/
int find_char(char *url, char ch)
{
  for (size_t i = 0; i < strlen(url); i++) {
    if (url[i] == ch) {
      return i; //FOUND
    }
  }
  return -1;    //NOT FOUND
}

/*
* Last char
*/
int find_last_char_pos(char *url, char ch)
{
  for (size_t i = strlen(url); i > 0; i--){
    if (url[i] == ch) {
      return i+1; //FOUND
    }
  }
  return -1;    //NOT FOUND
}

/*
* Cut url
*/
char * cut_string(char * old_url, int begin, int size)
{
  char *new_string = (char *) malloc(size);
  memset(new_string,0,size);
  memcpy(new_string, &old_url[begin], size);
  new_string[size] = '\0';
  return new_string;
}


/*
* set the port values properly
*/
void set_port(struct url_info_t * url)
{
  url->port_number = DEFAULT_PORT_NUMBER;
  char * port;
  if (( port = apply_rgx(PORT_NUMBER_RGX, url->address)) != NULL) {
    url->port_number = (int)strtol(&port[1], (char **)NULL, 10);
  }

}

/*
* Set default values for filename and base_address
*/
void set_default_values(struct url_info_t * url)
{
  url->base_address = apply_rgx(URL_RGX,url->address);
  url->path = SLASH_STR ;
  url->filename = DEFAULT_FILE_NAME;
}
/*
* Parse url string to struct
*/
int parse_url(struct url_info_t * url, char * url_str)
{
  // get rid of http
  if (strstr(url_str,PREFIX) == NULL)
      return -1;
  else
    url->address = strstr(url_str,PREFIX) + strlen(PREFIX);

  // shot url without slashes, setting default values
  if (strstr(url->address,SLASH_STR) == NULL){
      set_default_values(url);
      set_port(url);
      return 0;
  }
  //url with path and filename
  url->path = strstr(url->address,SLASH_STR);

  int cut_filename = find_last_char_pos(url->address,SLASH);
  url->filename = cut_string(url->address, cut_filename, strlen(url->address) - cut_filename);

  url->base_address = apply_rgx(URL_RGX,url->address);
  set_port(url);
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
  if (argc != 2) {
    fprintf(stderr,"Invalid number of args\n");
    return -1;
  }
   struct url_info_t  *url;

  if ((url = (struct url_info_t *)malloc(sizeof(struct url_info_t))) == NULL) {
    fprintf(stderr, "Alloc error\n" );
    return -1;
  }
   printf("%d\n", parse_url(url, argv[1]));
   printf("*************************\n" );
   printf("ADD:\t%s \n",url->address );
   printf("DNS:\t%s \n",url->base_address);
   printf("PATH:\t%s \n",url->path );
   printf("FILE\t%s \n",url->filename);
   printf("Port:\t%d\n",url->port_number );
  // free(url->base_address);
  // free(url->filename);
  // free(url);
  // printf("%s\n", whitespaces (argv[1]));

  // printf("%ld\n", (int)strlen(FIT)-strlen(PREFIX));
  // printf("%s\n",cut_string(argv[1],7,strlen(FIT)-strlen(PREFIX)) );
  // printf("%d\n",check_prefix(argv[1]));
  // printf("%d\n",find_char(argv[1],SLASH));
  // return 0;
  printf("%s\n",url->base_address );
  struct hostent *web_address;
  if (!strcmp(url->base_address,FIT)){
    web_address = gethostbyname(FIT);
    printf("match\n" );
    }
  else
    web_address = gethostbyname(url->base_address);


  if ( web_address == NULL) {                         //check if translation was succesfull
    fprintf(stderr,"DNSERR: %s\n", strerror(errno));
    return -1;
  }
  // in_addr is struct required by inet_ntoa
  // which is needed to translate from network byte order
  struct in_addr ip_addr;
  memcpy(&ip_addr, web_address->h_addr_list[0], sizeof(struct in_addr));
  printf("%s\n",inet_ntoa(ip_addr));


  int mysocket;
   if((mysocket = socket(AF_INET, SOCK_STREAM, 0)) == -1) { // creating socket
     fprintf(stderr,"SOCKERR: %s\n", strerror(errno));
     return -1;
   }

  struct sockaddr_in dest;
  memset(&dest, 0, sizeof(dest));                       // setting up struct for connect
  dest.sin_family = AF_INET;
  dest.sin_addr.s_addr = inet_addr(inet_ntoa(ip_addr)); // setting properly destination ip address
  dest.sin_port = htons(url->port_number);                            // set destination port

  if(connect(mysocket, (struct sockaddr *)&dest, sizeof(struct sockaddr)) == -1 )
  {
    fprintf(stderr,"CONNERR: %s\n", strerror(errno));
    return -1;
  }

  char reply[1000];         // buffer fo response
  char request[1000];       // buffer which holds message to be sent
  sprintf(request, "HEAD %s HTTP/1.1\r\nHost: %s\r\nConnection: close \r\n\r\n", url->path, url->base_address);

  if( send(mysocket, request, strlen(request), 0) == -1) //try to send message
  {
    fprintf(stderr,"SENDERR: %s\n", strerror(errno));
    return -1;
  }

  if ((recv(mysocket, reply, 999, 0)) == -1)            // receive data
  {
    fprintf(stderr,"RECVERR: %s\n", strerror(errno));
    return -1;
  }
  printf("***********************************\n" );
  printf("%s\n",request );
  printf("***********************************\n" );
  printf("%s\n",reply );
  char * version = get_version(reply);
  printf("%s\n",version);
  printf("%s:%d\n",argv[0],argc );

  free(version);
  return 0;
}
