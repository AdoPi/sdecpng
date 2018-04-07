#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <string>
#include <iostream>
#include <sys/time.h>
#include "lodepng/lodepng.h"
#include<arpa/inet.h>


#define CHUNK_SIZE 1024


// get data from here
void decodePng() {

  std::vector<unsigned char> png;
  std::vector<unsigned char> image; //the raw pixels
  unsigned width, height;

  //decode
  unsigned error = lodepng::decode(image, width, height, png);

  //if there's an error, display it
  if(error) std::cout << "decoder error " << error << ": " << lodepng_error_text(error) << std::endl;

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
      total_size += size_recv;
      printf("%s" , chunk);
      //reset beginning time
      gettimeofday(&begin , NULL);
    }
  }

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
  recv_timeout(socket_desc,5,fhd_png);

  //decode data
  decodePng();

  return 0;
}
