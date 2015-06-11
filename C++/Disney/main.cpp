/********************************************************************************************************************\
													Disney Puck 
\********************************************************************************************************************/
//__________________________________________________________________________________________________________Inclusions
#include <iostream>
#include <tchar.h>
#include <windows.h>
#include "Serial.h"
//_________________________________________________________________________________________________________Definitions

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
	CSerial serial;
	LONG    lLastError = ERROR_SUCCESS;	//Keeps track of last error as long
	DWORD dwBytesRead = 0;	//Used to track number of bytes read during read
	BYTE abBuffer[1];		//Temp Buffer to populate during read
	string myString;
    
    
	// Attempt to open the serial port (COM1)
    lLastError = serial.SetMask(CSerial::EEventBreak |
				   				CSerial::EEventError |
				   				CSerial::EEventRecv  |
				   				CSerial::EEventRcvEv);	/*Includes Event Gen on byte occurance*/

    serial.SetEventChar(0x7E);	/*Identifies the Xbee Start Byte as an event generator.*/

	lLastError = serial.SetupReadTimeouts(CSerial::EReadTimeoutNonblocking);	/*Use non-blocking reads because we 
		don't know how many bytes will be received.  This is normally the most convenient mode (and also the default 
		mode for reading data)*/
    if(lLastError != ERROR_SUCCESS){
    	return ::ShowError(serial.GetLastError(), _T("Unable to set COM-port read timeout"));
	}
    
    //Setup Serial Port to COM5
	lLastError = serial.Open(_T("COM5"));
	if (lLastError != ERROR_SUCCESS){
		return ::ShowError(serial.GetLastError(), _T("Unable to open COM-port"));
	}
	
	// Setup the serial port (9600,N81) using hardware handshaking
    lLastError = serial.Setup(CSerial::EBaud9600,
				 			  CSerial::EData8,
				 			  CSerial::EParNone,
				 			  CSerial::EStop1);
	if (lLastError != ERROR_SUCCESS){
		return ::ShowError(serial.GetLastError(), _T("Unable to set COM-port setting"));
	}
	
    serial.SetupHandshaking(CSerial::EHandshakeHardware);
    
	bool fContinue = true;	//Keep reading data until an EOF (Ctrl-Z) has been received
	do{
    	//Wait for an Event
    	lLastError = serial.WaitEvent();	//Sits and waits for one of the events to happen
    	if(lLastError != ERROR_SUCCESS){
    		return ::ShowError(serial.GetLastError(), _T("Unable to wait for a COM-port event."));
		}
		
		//Save Event
		const CSerial::EEvent eEvent = serial.GetEventType();
		
		// Handle break event
		if (eEvent & CSerial::EEventBreak){
			printf("\n### BREAK received ###\n");
		}

		// Handle CTS event
		if (eEvent & CSerial::EEventCTS){
			printf("\n### Clear to send %s ###\n", serial.GetCTS()?"on":"off");
		}

		// Handle DSR event
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

		// Handle ring event
		if (eEvent & CSerial::EEventRing){
			printf("\n### RING ###\n");
		}

		// Handle RLSD/CD event
		if (eEvent & CSerial::EEventRLSD){
			printf("\n### RLSD/CD %s ###\n", serial.GetRLSD()?"on":"off");
		}
		
		// Handle data receive event
		if (eEvent & CSerial::EEventRecv){
			// Read data, until there is nothing left
			DWORD dwBytesRead = 0;
			char szBuffer[101];
			do{
				// Read data from the COM-port
				lLastError = serial.Read(szBuffer,sizeof(szBuffer)-1,&dwBytesRead);
				if (lLastError != ERROR_SUCCESS){
					return ::ShowError(serial.GetLastError(), _T("Unable to read from COM-port."));
				}

				if (dwBytesRead > 0){
					// Finalize the data, so it is a valid string
					szBuffer[dwBytesRead] = '\0';

					// Display the data
					printf("%s", szBuffer);

					// Check if EOF (CTRL+'[') has been specified
					if (strchr(szBuffer,EOF_Char)){
						fContinue = false;
					}
				}
			}
		    while (dwBytesRead == sizeof(szBuffer)-1);
		}
		
		// Handle data receive event
		if (eEvent & CSerial::EEventRcvEv){
			//Receive a Packet.... This will be wherethe Xbee magic happens...
		}
	} 
	while(fContinue);
	do{
    	serial.Read(abBuffer,sizeof(abBuffer),&dwBytesRead);
    	if(dwBytesRead > 0){
    		
		}
	}while(dwBytesRead == sizeof(abBuffer));
	
	while(true){
		serial.Read(abBuffer,sizeof(abBuffer),&dwBytesRead);
        if (dwBytesRead > 0){
            cout<<abBuffer;
        }
	}
    // Close the port again
    serial.Close();
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
