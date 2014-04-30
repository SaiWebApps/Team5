/*****************************************************************************
*                                                                            *
*  OpenNI 2.x Alpha                                                          *
*  Copyright (C) 2012 PrimeSense Ltd.                                        *
*                                                                            *
*  This file is part of OpenNI.                                              *
*                                                                            *
*  Licensed under the Apache License, Version 2.0 (the "License");           *
*  you may not use this file except in compliance with the License.          *
*  You may obtain a copy of the License at                                   *
*                                                                            *
*      http://www.apache.org/licenses/LICENSE-2.0                            *
*                                                                            *
*  Unless required by applicable law or agreed to in writing, software       *
*  distributed under the License is distributed on an "AS IS" BASIS,         *
*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  *
*  See the License for the specific language governing permissions and       *
*  limitations under the License.                                            *
*                                                                            *
*****************************************************************************/
#include <MWClosestPoint.h>
#include <OpenNI.h>
#include <vector> 
#include <cmath> 
#include <math.h> 

#define X_MIN 1
#define X_MAX 150.0
#define Y_MIN 0 
#define Y_MAX 118.0
#define BUFFER_X 20
#define BUFFER_Y 15
#define MIDX X_MAX/2
#define MIDY 65
#define RADIUS_X 35
#define RADIUS_Y 30

#ifdef WIN32
#include <conio.h>
int wasKeyboardHit()
{
        return (int)_kbhit();
}

#else // linux

#include <unistd.h>
#include <stdlib.h>
#include <termios.h>
#include <fcntl.h>

//server libraries
#include <stdio.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

static int newsockfd;

int wasKeyboardHit()
{
        struct termios oldt, newt;
        int ch;
        int oldf;

        // don't echo and don't wait for ENTER
        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
        oldf = fcntl(STDIN_FILENO, F_GETFL, 0);

        // make it non-blocking (so we can check without waiting)
        if (0 != fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK))
        {
                return 0;
        }

        ch = getchar();

        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
        if (0 != fcntl(STDIN_FILENO, F_SETFL, oldf))
        {
                return 0;
        }

        if(ch != EOF)
        {
                ungetc(ch, stdin);
                return 1;
        }

        return 0;
}

void Sleep(int ms)
{
        usleep(ms*1000);
}

#endif // WIN32

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

class MyMwListener : public closest_point::ClosestPoint::Listener
{
public:
	void readyForNextData(closest_point::ClosestPoint* pClosestPoint)
	{
		static bool hasMoved = false;

		openni::VideoFrameRef frame;
		closest_point::IntPoint3D closest;
		openni::Status rc = pClosestPoint->getNextData(closest, frame);

		if (rc == openni::STATUS_OK)
		{
			// At rest inside
			if((std::abs(closest.X - MIDX) <= RADIUS_X) 
				&& (std::abs(closest.Y - MIDY) <= RADIUS_Y)) {
				hasMoved = false;
			}
			else if((std::abs(closest.X - MIDX) > RADIUS_X + BUFFER_X 
				|| std::abs(closest.Y - MIDY) > RADIUS_Y + BUFFER_Y) && !hasMoved) {
				// Horizontal position (needs factor to account for irregular coordinate system)
				if(std::abs(closest.X - MIDX) > std::abs(closest.Y - MIDY) * X_MAX/Y_MAX){
					if(closest.X - MIDX > 0) {
						printf("RIGHT\n");
						// This send() function sends the string to the new socket
						send(newsockfd, "1\n", 1, 0);
					}
					else {
						printf("LEFT\n");
						// This send() function sends the string to the new socket
						send(newsockfd, "3\n", 1, 0);
					}
				}
				// Vertical axis
				else {
					if(closest.Y - MIDY > 0) {
						printf("DOWN\n");
						// This send() function sends the string to the new socket
						send(newsockfd, "2\n", 1, 0);
					}
					else {
						printf("UP\n");
						// This send() function sends the string to the new socket
						send(newsockfd, "0\n", 1, 0);
					}
				}
				hasMoved = true;
			}	
		}
		else
		{
			printf("Update failed\n");
		}
	}
};


int main()
{
    //server setup code
    int sockfd, portno;
    socklen_t clilen;
    char buffer[256];
    struct sockaddr_in serv_addr, cli_addr;
    int n;

    // create a socket
	// socket(int domain, int type, int protocol)
	sockfd =  socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) 
	  error("ERROR opening socket");

	// clear address structure
    bzero((char *) &serv_addr, sizeof(serv_addr));      

    //hardcoded port 2048
	portno = 2048;    

	/* setup the host_addr structure for use in bind call */
	// server byte order
	serv_addr.sin_family = AF_INET;  

	// automatically be filled with current host's IP address
	serv_addr.sin_addr.s_addr = INADDR_ANY;  

	// convert short integer value for port must be converted into network byte order
	serv_addr.sin_port = htons(portno);

	// bind(int fd, struct sockaddr *local_addr, socklen_t addr_length)
	// bind() passes file descriptor, the address structure, 
	// and the length of the address structure
	// This bind() call will bind  the socket to the current IP address on port, portno
	if (bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
	  error("ERROR on binding");

	// This listen() call tells the socket to listen to the incoming connections.
	// The listen() function places all incoming connection into a backlog queue
	// until accept() call accepts the connection.
	// Here, we set the maximum size for the backlog queue to 5.
	listen(sockfd,5);

	// The accept() call actually accepts an incoming connection
    clilen = sizeof(cli_addr);

    // This accept() function will write the connecting client's address info 
	// into the the address structure and the size of that structure is clilen.
	// The accept() returns a new socket file descriptor for the accepted connection.
	// So, the original socket file descriptor can continue to be used 
	// for accepting new connections while the new socker file descriptor is used for
	// communicating with the connected client.
	newsockfd = accept(sockfd, 
	(struct sockaddr *) &cli_addr, &clilen);

	if (newsockfd < 0) 
	  error("ERROR on accept");

	//START original code
	closest_point::ClosestPoint closestPoint;

	if (!closestPoint.isValid())
	{
		printf("ClosestPoint: error in initialization\n");
		return 1;
	}

	MyMwListener myListener;

	closestPoint.setListener(myListener);

	while (!wasKeyboardHit())
	{
		Sleep(1000);
	}

	closestPoint.resetListener();
	//END ORIGINAL CODE
	
	//more server code
	bzero(buffer,256);

	n = read(newsockfd,buffer,255);
	if (n < 0) error("ERROR reading from socket");
	printf("Here is the message: %s\n",buffer);

	close(newsockfd);
	close(sockfd);
	
	return 0;
}
