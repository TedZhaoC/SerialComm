///////////////////////////////////////////////
// Company : Farmersedge
// Author : Keith Y, Ted Zhao
// Purpose: To implement serial port communication
// Date : 2017, 2018
//
///////////////////////////////////////////////
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/types.h> 
#include <sys/stat.h> 
#include <stdlib.h>
#include <math.h>
#include <time.h>


struct serial
{
    struct termios Termios; // saved old settings
    int            Stream;  // handle
};


//--------------------------------------------

int kbhit(void)
{
	// returns 1 if user pressed  key on keyboard (linux does not have kbhit(), windows does)
    struct termios oldt, newt;
    int ch;
    int oldf;
 
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
 
    ch = getchar();
 
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);
 
    if  (ch != EOF)
    {
        ungetc(ch, stdin);
        return 1;
    }
 
    return 0;
}


void SerialFree(struct serial *sn)
{
    if (sn->Stream)
    {
        // restore old settings 
        tcsetattr(sn->Stream, TCSANOW, &sn->Termios);
        close(sn->Stream);

        sn->Stream = 0;
    }
}

int SerialInit(struct serial *sn,char *ttyName,int baud,int stopBits,int useHwFlow,int parity,int dataBits)
{
    // baud: B50,B110,B300,B600,B1200,B2400,B4800,B9600,B19200,B38400,B57600,B115200 
    // databits: CS5,CS6,CS7,CS8 
    // stop bits:  one = 0, two = CSTOPB
    // parity: none = 0, odd = PARODD | PARENB, even = PARENB 
    struct termios newtermios;


    if ((sn->Stream = open(ttyName, O_RDWR | O_NOCTTY | O_NONBLOCK)) <= 0)
      { printf("\nCould not open serial port '%s'\n\n",ttyName); return 0; }

    tcgetattr(sn->Stream, &sn->Termios);

    memset(&newtermios, 0, sizeof(struct termios));

    newtermios.c_cflag = baud | stopBits | parity | dataBits | CLOCAL | CREAD;
    if (useHwFlow)
      newtermios.c_cflag |= CRTSCTS;
    newtermios.c_cc[VMIN] = 1;

    tcflush(sn->Stream, TCIOFLUSH);
    tcsetattr(sn->Stream, TCSANOW, &newtermios);
    tcflush(sn->Stream, TCIOFLUSH);
    fcntl(sn->Stream, F_SETFL, O_NONBLOCK);

    return 1;
}

int SerialRead(struct serial *sn, char *buffer, size_t size)
{
    int j = read(sn->Stream, buffer, size);


    if (j < 0)
      {
        if (errno == EAGAIN)
          return 0;
        else
          return j;
      }
    return j;
}

int SerialWrite(struct serial *sn, const char *buffer, size_t size)
{
    int j = write(sn->Stream, buffer, size);


    if (j < 0)
      {
        if(errno == EAGAIN)
          return 0;
        else
          return j;
      }
    return j;
}


//------------------------------------------

 
void main(int argc,char **argv)
{
    struct serial  port; 
    unsigned char  serialBuf[100];
    unsigned char  serial_write_Buf[100];
    int      i,j,n=0,baud,ch;
    time_t   t1,t2;

	int len = 0;
   
 
    if (argc != 3)
      { printf("\nUsage: %s <tty port> <baud>\n\n",argv[0]); exit(1); }

    j = atoi(argv[2]);
    if      (j == 50) baud = B50;
    else if (j == 110) baud = B110;
    else if (j == 300) baud = B300;
    else if (j == 600) baud = B600;
    else if (j == 1200) baud = B1200;
    else if (j == 2400) baud = B2400;
    else if (j == 4800) baud = B4800;
    else if (j == 9600) baud = B9600; 		// 
    else if (j == 19200) baud = B19200;
    else if (j == 38400) baud = B38400;
    else if (j == 57600) baud = B57600;
    else if (j == 115200) baud = B115200; 	// 
    else
    { printf("\nError: invalid baud rate (%d)\n",atoi(argv[3])); exit(1); }


    // initializer serial port
    if (!SerialInit(&port,argv[1], baud,0, 0,0, CS8))
    { printf("\nError: cannot open serial port %s\n\n",argv[1]); return ; }

    time(&t1);
    printf("\nwaiting for serial data on %s at %d baud, press ESC to exit....\n\n",argv[1],atoi(argv[2])); fflush(stdout); 

    for (;;)
    {
        // check for incoming data
        if ((j = SerialRead(&port,serialBuf,95)) > 0)
        { 
            // print out hex value of received bytes
            for (i = 0; i < j; i++)
			{
				printf("Receiving data : 0x%02x\n",(int)serialBuf[i]);
				printf("Receiving data : %c\n",(int)serialBuf[i]);
			}
			//Ted TODO
			//2. save read into a file
			//save received to permanent storage, i.e. file or database  
			//
			//...
			//...

            fflush(stdout);
        }

       
        // do something every 5 seconds... 
        time(&t2);
        if ((t2 - t1) >= 5)
        {
            t1 = t2;

            printf("(timer)\n"); fflush(stdout); 

            // put code here to send a serial command or whatever

			strcpy(serial_write_Buf,"AT\r"); // ascii or binary command data
			j = 3; // length of command to send
			len = SerialWrite(&port,serial_write_Buf,j); 
			printf("Send 2222 %d\n", len);

       	}

        if (kbhit())
        {
            ch = getchar();
            if (ch == 27) // ESC
              break; // break out of loop and exit

            // or test for other keypress like '0' thru '9', 'a' through 'z', '\r', etc and do somthing
            
			//Ted mark 
			//1. write data as well
			//	SerialWrite(&port,(const char*) ch,1); 

			switch(ch) {
				case 'A'  :
				case 'a'  :
					strcpy(serial_write_Buf, "Send command A\n"); // ascii or binary command data
					if (len = SerialWrite(&port,serial_write_Buf,strlen(serial_write_Buf)) > 0)
						printf("Send 1111 %d\n", len);
					else 
						printf("Fail to send 111 %d\n", len);

	  				break;

		        case 'B'  :
		        case 'b'  :
					strcpy(serial_write_Buf, "Send command BBBB\n"); // ascii or binary command data
					if (len = SerialWrite(&port,serial_write_Buf,strlen(serial_write_Buf)) > 0)
						printf("Send 222 %d\n", len);
					else 
						printf("Fail to send 222 %d\n", len);

	  				break; 
				 
			//	default : /* Optional */
			}

        }
		//TODO
		//1. write data as well
		//2. save read into a file
		//3. Antoine requirement for special key for special purposes?
    }

    printf("\nexiting program\n\n");

    // restore original serial port settings
    SerialFree(&port);
}
