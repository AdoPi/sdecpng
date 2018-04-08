#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <string.h>

#include <string>
#include <iostream>
#include <fstream>
#include <cstring>
#include <iterator>
#include <vector>
#include <algorithm>

#include "lodepng/lodepng.h"

#define CHUNK_SIZE 1024
#define PNG_SIZE 976 //TODO should be dynamic


// get data from here
std::vector<unsigned char> decodePng(std::vector<unsigned char>& png) {

  std::vector<unsigned char> image; //the raw pixels
  unsigned width, height;

  puts("prepare to decode");
  //decode
  unsigned error = lodepng::decode(image, width, height, png);

  //if there's an error, display it
  if(error) std::cout << "decoder error " << error << ": " << lodepng_error_text(error) << std::endl;

  return image;
}


/*
   Receive data in multiple chunks by checking a non-blocking socket
   Timeout in seconds
   */
int recv_timeout(int s , int timeout, uint8_t* data)
{
  int size_recv , total_size= 0;
  struct timeval begin , now;
  char chunk[CHUNK_SIZE];
  double timediff;

  //make socket non blocking
  fcntl(s, F_SETFL, O_NONBLOCK);

  //beginning time
  gettimeofday(&begin , NULL);

  while(1)
  {
    gettimeofday(&now , NULL);

    //time elapsed in seconds
    timediff = (now.tv_sec - begin.tv_sec) + 1e-6 * (now.tv_usec - begin.tv_usec);

    //if you got some data, then break after timeout
    if( total_size > 0 && timediff > timeout )
    {
      break;
    }

    //if you got no data at all, wait a little longer, twice the timeout
    else if( timediff > timeout*2)
    {
      //      break; // no break here in streamer
    }

    memset(chunk ,0 , CHUNK_SIZE);  //clear the variable
    if((size_recv =  recv(s , chunk , CHUNK_SIZE , 0) ) < 0)
    {
      //if nothing was received then we want to wait a little before trying again, 0.1 seconds
      usleep(100000);
    }
    else
    {
      memcpy(data+total_size,chunk,size_recv);
      total_size += size_recv;
      printf("%s" , chunk);
      //reset beginning time
      gettimeofday(&begin , NULL);
    }
  }

  data[total_size] = '\0';

  return total_size;
}


int main() {
  uint8_t fhd_png[1280*1920];

  //socket logic
  int socket_desc;
  struct sockaddr_in server;
  char *message;

  //Create socket
  socket_desc = socket(AF_INET , SOCK_STREAM , 0);
  if (socket_desc == -1)
  {
    printf("Could not create socket");
  }

  server.sin_addr.s_addr = inet_addr("66.117.151.35");
  server.sin_family = AF_INET;
  server.sin_port = htons( 80 );

  //Connect to remote server
  if (connect(socket_desc , (struct sockaddr *)&server , sizeof(server)) < 0)
  {
    puts("connect error");
    return 1;
  }

  puts("Connected\n");

  //Send some data
  message = "GET /superpng/pnggrad8rgb.png HTTP/1.1\r\nHost: www.fnordware.com\r\n\r\n";
  if( send(socket_desc , message , strlen(message) , 0) < 0)
  {
    puts("Send failed");
    return 1;
  }
  puts("Data Sent\n");


  //receive data
  int nb_received = recv_timeout(socket_desc,1,fhd_png);
  
  printf("received:%d\n",nb_received);

  //remove http headers
  char *content = strstr((char*)fhd_png, "\r\n\r\n");
  if (content != NULL) {
    content += 4; // Offset by 4 bytes to start of content
  }
  else {
    content = (char*)fhd_png; // Didn't find end of header, write out everything
  }

  printf("strlen content:%d\n",strlen(content));
  std::vector<unsigned char> png(content,content+PNG_SIZE);

  //decode data
  std::vector<unsigned char> img = decodePng(png);

  //write results
  std::ofstream outfile ("decode.rgba",std::ofstream::binary);

  outfile.write((char*)&img[0],img.size());
  outfile.close();
  return 0;
}
