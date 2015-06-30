/********************************************************************************************************************\
													Disney Puck 
	Author(s):
		Matthew Connolly
	Description:
		Welcome to the Left Hand Path.... that sucks
\********************************************************************************************************************/
//__________________________________________________________________________________________________________Inclusions
#include <iostream>
#include <fstream>		//Included to allow for file writing
#include <tchar.h>
#include <windows.h>
#include "Serial.h"		//Serial library, wrapper around Windows API
#include <ctime>		//Used to get current date & time data
#include <string.h>
//_________________________________________________________________________________________________________Definitions
#define DATA_SAMPLE_RX	0x92
#define PUCK_A	0x1E
#define PUCK_B	0xC8
//___________________________________________________________________________________________________________Namespace
using namespace std;


//________________________________________________________________________________________________________Enumerations
enum {EOF_Char = 27};
/* run this program using the console pauser or add your own getch, system("pause") or input loop */
//__________________________________________________________________________________________________________Prototypes
int ShowError (LONG lError, LPCTSTR lptszMessage);

//________________________________________________________________________________________________________________Main
/****************************************************************************************************************\
tWinMain
parameters
	HINSTANCE	hInst
	HINSTANCE	hInstPrev
	LPSTR		lptszCmdLine
	int			nCmdShow
\****************************************************************************************************************/
int WINAPI _tWinMain(HINSTANCE, HINSTANCE, LPTSTR, int){
	//Timer Garbage
	LARGE_INTEGER frequency;        // ticks per second
    LARGE_INTEGER t1, t2;           // ticks
    double elapsedTime;
    
	int count=0;
	int i=0;
	int statTrack[12] = {};	//Initialized to zero
	CSerial serial;
	LONG    lLastError = ERROR_SUCCESS;	//Keeps track of last error as long
	DWORD dwBytesRead = 0;	//Used to track number of bytes read during read
	BYTE abBuffer[1];		//Temp Buffer to populate during read
	string myString;
	FILE * logFile;		//csv file to record data.
	char filename[20];
	
	//_______________________________________________________________________________________________High Res Time
	// get ticks per second
    QueryPerformanceFrequency(&frequency);
	
	//________________________________________________________________________________________________File IO Date
	time_t now = time(0);
	char* dateTime = ctime(&now);
	printf("The local date and time is: %s \n\r", dateTime);

	// convert now to tm struct for UTC
	tm *gmtm = gmtime(&now);
	dateTime = asctime(gmtm);
	printf("The UTC date and time is: %s \n\r", dateTime);
	//_________________________________________________________________________________________________Init FileIO
	//		Of note, during a CSV file write, only a \r or a \n are required to make the appropriate
	//	new line happen.  doubling them as one would do in a terminal application is not necessary
	//	and serves only to cause blank lines to occur.
	//		The header of the logFile will consist solely of the time stamp of the received packet
	//	the ID of the source, and the X,Y and Z readings from the specified puck.
	logFile = fopen("SCREAMIN.csv","w");
	fprintf(logFile,"ID,milliseconds,Ax,Ay,Az,Bx,By,Bz,Cx,Cy,Cz,Dx,Dy,Dz\r");

	//_________________________________________________________________________________________________Init Serial
	// Attempt to open the serial port (COM1)
    lLastError = serial.SetMask(CSerial::EEventBreak |
				   				CSerial::EEventError |
				   				//CSerial::EEventRecv  |
				   				CSerial::EEventRcvEv);	/*Includes Event Gen on byte occurance*/

    serial.SetEventChar(0x7E);	/*Identifies the Xbee Start Byte as an event generator.*/

	//Setup Serial Port to COM5
	lLastError = serial.Open(_T("COM5"));
	if (lLastError != ERROR_SUCCESS){
		return ::ShowError(serial.GetLastError(), _T("Unable to open COM-port"));
	}
	
	lLastError = serial.SetupReadTimeouts(CSerial::EReadTimeoutNonblocking);	/*Use non-blocking reads because we 
		don't know how many bytes will be received.  This is normally the most convenient mode (and also the default 
		mode for reading data)*/
    if(lLastError != ERROR_SUCCESS){
    	return ::ShowError(serial.GetLastError(), _T("Unable to set COM-port read timeout"));
	}
    
	// Setup the serial port (9600,N81) using hardware handshaking
    lLastError = serial.Setup(CSerial::EBaud115200,
				 			  CSerial::EData8,
				 			  CSerial::EParNone,
				 			  CSerial::EStop1);
	if (lLastError != ERROR_SUCCESS){
		return ::ShowError(serial.GetLastError(), _T("Unable to set COM-port setting"));
	}
	
    serial.SetupHandshaking(CSerial::EHandshakeHardware);
    // start timer
    QueryPerformanceCounter(&t1);
    
    //_______________________________________________________________________________________________Serial Listen
	bool fContinue = true;	//Keep reading data until an EOF (Ctrl-Z) has been received
	serial.Purge();

	do{
    	//Wait for an Event
    	lLastError = serial.WaitEvent();	//Sits and waits for one of the events to happen
    	if(lLastError != ERROR_SUCCESS){
    		return ::ShowError(serial.GetLastError(), _T("Unable to wait for a COM-port event."));
		}
		
		//Save Event
		const CSerial::EEvent eEvent = serial.GetEventType();
	
		//________________________________________________________________________________Break Event
		/********************************************************************************\
				A Break Event occurs whwnever a break s detected on input.
		\********************************************************************************/
		if (eEvent & CSerial::EEventBreak){
			printf("\n### BREAK received ###\n");
		}

		//__________________________________________________________________________________CTS Event
		/********************************************************************************\
				Means that the CTS (Clear to sent) signal has changed.
		\********************************************************************************/
		if (eEvent & CSerial::EEventCTS){
			printf("\n### Clear to send %s ###\n", serial.GetCTS()?"on":"off");
		}

		//__________________________________________________________________________________DSR Event
		/********************************************************************************\
				Means that the DSR (Data Set Ready) signal has changed.
		\********************************************************************************/
		if (eEvent & CSerial::EEventDSR){
			printf("\n### Data set ready %s ###\n", serial.GetDSR()?"on":"off");
		}

		// Handle error event
		if (eEvent & CSerial::EEventError){
			printf("\n### ERROR: ");
			switch (serial.GetError())
			{
			case CSerial::EErrorBreak:		printf("Break condition");			break;
			case CSerial::EErrorFrame:		printf("Framing error");			break;
			case CSerial::EErrorIOE:		printf("IO device error");			break;
			case CSerial::EErrorMode:		printf("Unsupported mode");			break;
			case CSerial::EErrorOverrun:	printf("Buffer overrun");			break;
			case CSerial::EErrorRxOver:		printf("Input buffer overflow");	break;
			case CSerial::EErrorParity:		printf("Input parity error");		break;
			case CSerial::EErrorTxFull:		printf("Output buffer full");		break;
			default:						printf("Unknown");					break;
			}
			printf(" ###\n");
		}

		//_________________________________________________________________________________Ring Event
		/********************************************************************************\
				Indicates that the ring indicator was set high.  Only transitions from
			low to high will generate this event.
		\********************************************************************************/
		if (eEvent & CSerial::EEventRing){
			printf("\n### RING ###\n");
		}

		//______________________________________________________________________________RLSD/CD Event
		/******************************************************************************\
				Indicates that the RLSD (Receive Line Signal Detect) signal has changed.
			Note that this signal is often called the CD (Carrier Detect)
		\******************************************************************************/
		if (eEvent & CSerial::EEventRLSD){
			printf("\n### RLSD/CD %s ###\n", serial.GetRLSD()?"on":"off");
		}
		
		//_________________________________________________________________________Char Data Rcv Event
		/*************************************************************************\
				Called when the character specified in the character mask setup
			routine is called.  In the case of this application, the target 
			character is 0x7e as that is the start bit of an Xbee transmission
			packet.
		\*************************************************************************/
		if (eEvent & CSerial::EEventRcvEv){
			DWORD dwBytesRead = 0;
			char szBuffer[101];
			do{
				// Read data from the COM-port
				lLastError = serial.Read(szBuffer,27,&dwBytesRead);
				if (lLastError != ERROR_SUCCESS){
					return ::ShowError(serial.GetLastError(), _T("Unable to read from COM-port."));
				}

				if (dwBytesRead > 0){
					printf("Packet number %i    |    total bytes = %i    :-)  \n\r",count,dwBytesRead);
					// Finalize the data, so it is a valid string
					szBuffer[dwBytesRead] = '\0';
					i=0;
					for(int i=0 ; i < dwBytesRead; i++){
						printf("%hhx ",szBuffer[i]);
					}
					printf("\n\r");
					//if((count + 1) % 3 ==0){
					//	printf("\n\r\n\r");
					//}
					// Display the data
					//printf("%s", szBuffer);
					// Check if EOF (CTRL+'[') has been specified
					count++;
					if (strchr(szBuffer,EOF_Char)){
						fContinue = false;
					}
				}
			}
		    while (dwBytesRead == sizeof(szBuffer)-1);
		}
		
		//______________________________________________________________________________Data Rcv Event
		/********************************************************************************\
				serial.Read() :: The read function call is sent the address of the storage 
			buffer, the size of the buffer (minus 1 which may either be relevant to the 
			NULL character at the end or some other aspect.  And the address of the 
			counter which will be populated with the number of bytes read by the Read 
			function.
		\********************************************************************************/
		if (eEvent & CSerial::EEventRecv){
			// Read data, until there is nothing left !!!
			DWORD dwBytesRead = 0;
			unsigned char szBuffer[101];
			char startByte[1];
			char length[2];
			char pID[4];
			int pType;
			int pLength;
			int xAxis;
			int yAxis;
			int zAxis;
			int j = 0;
			unsigned char sID;
			
			char sum;
			
			lLastError = serial.Read(szBuffer,sizeof(szBuffer)-1,&dwBytesRead);
			
			if (lLastError != ERROR_SUCCESS){
				return ::ShowError(serial.GetLastError(), _T("Unable to read from COM-port."));
			}

			if (dwBytesRead > 0){
				//printf("\n\rNew packet number %i    |    total bytes = %i\n\r",count,dwBytesRead);
				// Finalize the data, so it is a valid string
				szBuffer[dwBytesRead] = '\0';
				//Start Parse
				j=0;
				while(j < dwBytesRead){
					//printf("2\n\r");
					if(szBuffer[j]==0x7E){
						//printf("3\n\r");
						count =count + 1;
						pLength = szBuffer[j+1]*256 + szBuffer[j+2];
						switch(szBuffer[j+3]){
							//_________________________________________Data Sample Packet
							//_________________________________________________AT Command
							case 0x08:
								printf("AT Command\n\r");
							break;
							//___________________________AT Command Queue Parameter Value
							case 0x09:
								printf("AT Command Queue Parameter Value\n\r");
							break;
							//_________________________________________________TX Request
							case 0x10:
								printf("TX Request\n\r");
							break;
							//________________________________________Explicit TX Request
							case 0x11:
								printf("Explicit TX Request\n\r");
							break;
							//__________________________________Remote AT Command Request
							case 0x17:
								printf("Remote AT Command Request\n\r");
							break;
							//________________________________________AT COmmand Response
							case 0x88:
								printf("AT Command Response\n\r");
							break;
							//_______________________________________________Modem Status
							case 0x8A:
								printf("Modem Status\n\r");
							break;
							//____________________________________________Transmit Status
							case 0x8B:
								printf("Transmit Status\n\r");
							break;
							//__________________________________________Route Info Packet
							case 0x8D:
								printf("Route Information Packet\n\r");
							break;
							//________________________________Aggregate Addressing Update
							case 0x8E:
								printf("Aggregate Addressing Update\n\r");
							break;
							//_______________________________________________RX Indicator
							case 0x90:
								printf("RX Indicator\n\r");
							break;
							//______________________________________Explicit RX Indicator
							case 0x91:
								printf("Explicit RX Indicator\n\r");
							break;
							
							//__________________________________Data Sample RX Inidicator
							case 0x92:
								QueryPerformanceCounter(&t2);
								elapsedTime = (t2.QuadPart - t1.QuadPart) * 1000.0 / frequency.QuadPart;
    							//cout << elapsedTime << " ms.\n";
								//printf("Data Packet RX");
								xAxis = szBuffer[j+20]*256 + szBuffer[21];
								yAxis = szBuffer[j+22]*256 + szBuffer[23];
								zAxis = szBuffer[j+24]*256 + szBuffer[25];
								if(xAxis <= 1023 && xAxis >= 00 && 
								   yAxis <= 1023 && yAxis >= 00 && 
								   zAxis <= 1023 && xAxis >= 00){
									switch(szBuffer[j+12]){
										case PUCK_A:
											statTrack[0] = xAxis;									
											statTrack[1] = yAxis;
											statTrack[2] = zAxis;
										break;
										case PUCK_B:
											statTrack[4] = xAxis;									
											statTrack[5] = yAxis;
											statTrack[6] = zAxis;
										break;
										default:
											printf(".");
										break;
									}
									//if(szBuffer[j+12] == APUCK || szBuffer[j+12] == BPUCK){
									fprintf(logFile,"%hhx,%f,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i\r",
																szBuffer[j+12],
																elapsedTime,
																statTrack[0],statTrack[1],statTrack[2],
																statTrack[3],statTrack[4],statTrack[5],
																statTrack[6],statTrack[7],statTrack[8],
																statTrack[9],statTrack[10],statTrack[11]);   	
								}
							break;
							//__________________________________________Node ID Indicator
							case 0x95:
								printf("Node ID Indicator\n\r");
							break;
							//____________________________________Remote Command Response
							case 0x97:
								printf("Remote Command Response\n\r");
							break;
							default:
								printf(",");
						}
						j+=26;	//Increments to end of packet
					}
					j++;
				}
			}

		}
	
	} 
	while(count < 100);

    // Close the Serial port
    serial.Close();
    fclose(logFile);
	cout<<"Test"<<endl;
    return 0;
}
//___________________________________________________________________________________________________________Functions
//__________________________________________________________________________________________________ShowError
/**************************************************************************************************\
		Generates error dialogues when prompted during system operation and displays source.
parameters
	LONG	lError
	LPCTSTR	lptszMessage
\**************************************************************************************************/
int ShowError (LONG lError, LPCTSTR lptszMessage){
	// Generate a message text
	TCHAR tszMessage[256];
	wsprintf(tszMessage,_T("%s\n(error code %d)"), lptszMessage, lError);

	// Display message-box and return with an error-code
	::MessageBox(0,tszMessage,_T("Listener"), MB_ICONSTOP|MB_OK);
	return 1;
}
