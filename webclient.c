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
#include <ctype.h>
#include <unistd.h>

/**
* REGEXES
*/
#define URL_RGX "(www.[a-zA-z0-9\\.]*)"
#define FILENAME_RGX "[0-9A-Za-z\\.\\_\\ ]*$"
#define PORT_NUMBER_RGX ":{1}[0-9]+"
#define WHITE_SPACE "[\\ ~]+"

#define DEFAULT_FILE_NAME "index.html"
#define DEFAULT_PORT_NUMBER 80
#define FIT "www.fit.vutbr.cz"
#define PREFIX "http://"
#define SLASH '/'
#define SLASH_STR "/"
#define CHUNKED "Transfer-Encoding: chunked"
#define HTTPV11 "1.1"
#define HTTPV10 "1.0"

#define ESCAPE_WHITESPACE "%20"
#define ESCAPE_TILDE "%7E"

struct url_info_t{
  char * address;       // whole address
  char * base_address;  //address which will by used to DNS server
  char * path;          // path to file (default /)
  char * filename;      // file name (default index.html)
  int port_number;      // default (80)
  int http_code;
  char *http_version;
  int chunked;
};

int download(struct url_info_t **url, int mysocket);

/*
* INIT struct
*/
int init(struct url_info_t * url){

  url->base_address = NULL;
  url->path = NULL;
  url->http_version = NULL;
  url->filename = NULL;
  url->port_number = 0;
  url->chunked = 0;
  url->http_code = 0;

  return 0;
}
/*
* INITIOALIZING connection
*/
int init_connection(struct url_info_t *url)
{
  if ( url == NULL) {
    return -1;
  }
  // printf("**********CONNECTING***************\n" );
  //    printf("ADD:\t%s \n",url->address );
  //    printf("DNS:\t%s \n",url->base_address);
  //    printf("PATH:\t%s \n",url->path );
  //    printf("FILE\t%s \n",url->filename);
  //    printf("Port:\t%d\n",url->port_number );

  struct hostent *web_address;
  if (!strcmp(url->base_address,FIT)){ //hack for fit url
    web_address = gethostbyname(FIT);
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

  int mysocket;
   if((mysocket = socket(AF_INET, SOCK_STREAM, 0)) == -1) { // creating socket
     fprintf(stderr,"SOCKERR: %s\n", strerror(errno));
     return -1;
   }

  struct sockaddr_in dest;
  memset(&dest, 0, sizeof(dest));                       // setting up struct for connect
  dest.sin_family = AF_INET;
  dest.sin_addr.s_addr = inet_addr(inet_ntoa(ip_addr)); // setting properly destination ip address
  dest.sin_port = htons(url->port_number);              // set destination port

  if(connect(mysocket, (struct sockaddr *)&dest, sizeof(struct sockaddr)) == -1 )
  {
    fprintf(stderr,"CONNERR: %s\n", strerror(errno));
    return -1;
  }
  return mysocket;
}

/*
* Fnction returns string matches by regex
*/
char * apply_rgx(char * rgx, char * string)
{
    regex_t r;
    if ((regcomp (&r, rgx, REG_EXTENDED|REG_NEWLINE)) != 0 ) {   // compile regex
      fprintf(stderr, "FAILED\n" );
      return NULL;
    }

    regmatch_t matches[1];
    if (regexec (&r, string, 1, matches, 0)) {                   // try to match
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
    //  result, matches[0].rm_so, matches[0].rm_eo - 1);
    regfree(&r);
    return result;
}

/*
* check whether certain character is present within string
*/
int find_char(char *url, char ch)
{
  for (size_t i = 0; i < strlen(url) ; i++) {
    if (url[i] == ch)
      return i; //FOUND
  }
  return -1;    //NOT FOUND
}

/*
* Find last char
*/
int find_last_char_pos(char *url, char ch)
{
  for (size_t i = strlen(url); i > 0; i--){
    if (url[i] == ch)
      return i+1; //FOUND
  }
  return -1;    //NOT FOUND
}

/*
* Cut url
*/
char * cut_string(char * old_string, int begin, int size)
{
  char *new_string = (char *) malloc(size+1);
  memset(new_string,0,size);
  memcpy(new_string, &old_string[begin], size);
  new_string[size] = '\0';
  return new_string;
}

/*
* Set default values for filename and base_address
*/
void set_default_values(struct url_info_t * url)
{
  if( (url->base_address = (char *) malloc(strlen(url->address)+1)) == NULL){
     fprintf(stderr, "MALLOC ERR\n" );
  }
  memcpy(url->base_address,url->address,strlen(url->address));
  url->base_address[strlen(url->address)] = '\0';
  url->path = SLASH_STR ;
  url->filename = DEFAULT_FILE_NAME;
}

/*
* escape wrong characters
*/
char * escape(char * url, char c, char * escape)
{
  int i = 0; int alloc = 0;
  while ( (i = find_char(url,c)) != -1 ) {  //find character position

    char * new_url = malloc(strlen(url) + strlen(escape));  //alloc new
    memcpy(new_url,&url[0],i);
    memcpy(&new_url[i],escape,strlen(escape));
    memcpy(&new_url[i + strlen(escape)],&url[i+1],strlen(url) - i);
    if (alloc) {  //do not try to unnaloc first string
        free(url);
        url = (char *)realloc(url,strlen(url)+strlen(escape));
        alloc = 1;
    }
    url = new_url;  //point back
  }
  return url;
}

/*
* Parse url string to struct
*/
int parse_url(struct url_info_t * url, char * url_str)
{
  url->http_version = "1.1";
  if ((url->address = (char *)malloc(strlen(url_str)+1)) == NULL) {
    fprintf(stderr, "Alloc error\n" );
    return -1;
  }
  memcpy(url->address,url_str,strlen(url_str));
  // get rid of http
  if (strstr(url->address,PREFIX) == NULL)
      return -1;
  else
    url->address = strstr(url->address,PREFIX) + strlen(PREFIX);
    //url->address[strlen(url->address)] = '\0';

  // shot url without slashes, setting default values
  if (strstr(url->address,SLASH_STR) == NULL){
      set_default_values(url);
      url->port_number = DEFAULT_PORT_NUMBER;
      return 0;
  }
  //url with path and filename
  url->path = strstr(url->address,SLASH_STR);

  if (strcmp(url->path,SLASH_STR) == 0) { // /
    url->filename = DEFAULT_FILE_NAME;
  }
  else{                                   // full address
    int cut_filename = find_last_char_pos(url->address,SLASH);
    url->filename = cut_string(url->address, cut_filename, strlen(url->address) - cut_filename);
  }
  //cutting port
  int port_len = 0;
  char * port = NULL;
  if (( port = apply_rgx(PORT_NUMBER_RGX, url->address)) != NULL){
    url->port_number = (int)strtol(&port[1], (char **)NULL, 10);
    port_len=strlen(port);
  }
  else
    url->port_number = DEFAULT_PORT_NUMBER;
    //cutting base address
  url->base_address = malloc(strlen(url->address) - strlen(url->path) - port_len + 1 );
  strncpy(url->base_address,url->address,strlen(url->address) - strlen(url->path) - port_len);
  url->base_address[strlen(url->address) - strlen(url->path)- port_len] = '\0';

  url->path = escape(url->path,' ',ESCAPE_WHITESPACE);
  url->path = escape(url->path,'~',ESCAPE_TILDE);


  //url->path[strlen(url->path)] = '\0';
  //printf("as%s\n",url->address );
  return 0;
}


/*
* get http code from HEAD response
*/
int http_info(struct url_info_t ** url, char * reply)
{
  (*url)->http_version = cut_string(reply,5,3);
  (*url)->http_code = (int)strtol(&reply[9], (char **)NULL, 10);
  if (strstr(reply,CHUNKED))
    (*url)->chunked = 1;
  else
    (*url)->chunked = 0;

  return 0;

}

/*
* READ data until you find needle in it ("\r\n")
*/
int read_until(int mysocket, char  * buffer, char * needle, int buffer_size)
{
  int position = 0;
  while (strstr(buffer,needle) == NULL)            // HEADER
  {
    if ((read(mysocket,&buffer[position], 1)) == -1) {
      return -1;
    }
    if (position > buffer_size) {   // prevent leaking
      fprintf(stderr, "ERR:%d\n",position );
      return -1;
    }
    position++;
  }
  return 0;
}

/*
* read from socket and write
*/
int write_to_file(struct url_info_t **url, int mysocket,unsigned int buffer_size)
{
  FILE *f;
  if ((f = fopen((*url)->filename,"w")) == NULL) {
    return -1;
  }
  int zero_counter = 0;
  char buffer[buffer_size];
  while (read(mysocket, buffer, buffer_size) > 0) { //read data
    if ((*url)->chunked){
      if (buffer[0] == '\r'){ //trasnfer \r to \0
        buffer[0] = '\0';
        zero_counter = 0;
      }
      if (zero_counter == 2 && buffer[0] == '0') { //cut zero at the end
        fclose(f);
        return 0;
      }
      zero_counter++;
    }
    fwrite(buffer, sizeof(char), buffer_size, f);
    memset(buffer, 0, buffer_size);
  }
  fclose(f);
  return 0;
}

/*
* redturn new struct on redirest
*/
struct url_info_t * redirect(char * url_str)
{
  if ( url_str == NULL) {
    fprintf(stderr,"ERROR NO URL\n" );
    return NULL;
  }
  for (unsigned int i = 0; i < strlen(url_str); i++) {
    if (url_str[i] == '\r') {
        url_str[i] = '\0';
    }
  }
  struct url_info_t  *new_url = NULL;
  if ((new_url = (struct url_info_t *)malloc(sizeof(struct url_info_t))) == NULL) {
    fprintf(stderr, "Alloc error\n" );
    return NULL;
  }
  init(new_url);
  int p = parse_url(new_url,url_str);
  if (p) {
    fprintf(stderr, "ERR:Problem with parsing entered URL\n" );
    free(new_url);
    return NULL;
  }

  return new_url;
}

/* One does not simply
 write code to main :D */
int main(int argc, char **argv)
{
  if (argc != 2) {
    fprintf(stderr,"ERR:Invalid number of args\n");
    return -1;
  }

  struct url_info_t  *url = NULL;
  if ((url = (struct url_info_t *)malloc(sizeof(struct url_info_t))) == NULL) {
    fprintf(stderr, "Alloc error\n" );
    return -1;
  }
  init(url);
  if (parse_url(url,argv[1])) {
    fprintf(stderr, "ERR:Problem with parsing entered URL\n" );
    free(url);
    return -1;
  }
  int d = 1; int version = 0;
  while(d != 0 ) //all prepared try to download image
  {
    int mysocket = init_connection(url);
    d = download(&url,mysocket);
    if (d == -1 || mysocket == -1)
      return -1;
    if ( d == 400) {
      if (version) {
        return -1;
      }
      url->http_version = "1.0";
      version = 1;
    }
  }
  free(url);
  return 0;
}

/*
* Function is able to download file given by information in url struct and socket
*/
int download(struct url_info_t **url, int mysocket)
{
  static int redirect_count = 0;
  char reply[1024];         // buffer fo response
  char request[1024];       // buffer which holds message to be sent
  memset(reply, 0, 1024);
  memset(request, 0, 1024);

  sprintf(request, "GET %s HTTP/%s\r\nHost: %s\r\nConnection: close \r\n\r\n", (char *)(*url)->path, (*url)->http_version ,(char *)(*url)->base_address);
  if( send(mysocket, request, strlen(request), 0) == -1) //try to send message
  {
    fprintf(stderr,"SENDERR: %s\n", strerror(errno));
    return -1;
  }
  //printf("REUQEST\n%s\n",request );
  read_until(mysocket, reply, "\r\n\r\n",1024);
  //printf("REPLY:\n%s\n",reply );
  http_info(url, reply);   // provide http version and code from reply

  if ((*url)->http_code >= 400) {
    fprintf(stderr, "HTTP ERROR:%d\n",(*url)->http_code );
    return 400;
  }
  //printf("%d\n", (*url)->http_code);
  if ((*url)->http_code >= 301) {
    struct url_info_t *old_url;
    old_url = (*url);
    (*url) = redirect(strstr(reply, "Location: ") + strlen("Location: ")); //parse new location
    free(old_url);                                          // get rid of old
    if ((*url) == NULL) {
      return -1;
    }
    if (redirect_count < 5){
      return 300;
    }
  }

  if ((*url)->chunked) { // determine chunk size
    memset(reply, 0, 1024);
    read_until(mysocket, reply, "\r\n",1024);
  }
  write_to_file(url, mysocket, 1);

 return 0;
}
