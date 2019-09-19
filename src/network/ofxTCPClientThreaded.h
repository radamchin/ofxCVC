#pragma once

/*
 ofxTCPClientThreaded
 
 Adam Hinshaw 17/5/14
 
 A mod. of ofxTCPClient so Connect is threaded, and does not block entire  app for 75secs if machine / ip is not existant.
 
 Notes: *Just connection is threaded, rest still runs on main host thread.
 
 TODO: could just have this extend ofxTCPClient & ofThread to remove all common code.
 
 */

#include "ofConstants.h"
#include "ofxTCPManager.h"
#include "ofFileUtils.h"
#include "ofTypes.h"
#include "ofMain.h"
#include "ofThread.h"

#define TCP_MAX_MSG_SIZE 512
//#define STR_END_MSG "[/TCP]"
//#define STR_END_MSG_LEN 6

#ifndef TARGET_WIN32
	#define TCP_CONNRESET ECONNRESET
#else
	#define TCP_CONNRESET WSAECONNRESET
#endif


class ofxTCPClientThreaded : public ofThread {

    public:

		ofxTCPClientThreaded();
		~ofxTCPClientThreaded();

		void threadedFunction();

		void setVerbose(bool _verbose);
		void setup(string ip, int _port, bool blocking = false);
		void setMessageDelimiter(string delim);
		bool close();

	
		//send data as a string - a short message
		//is added to the end of the string which is
		//used to indicate the end of the message to
		//the receiver see: STR_END_MSG (ofxTCPClientThreaded.h)
		bool send(string message);

		//send data as a string without the end message
		bool sendRaw(string message);

		//same as send for binary messages
		bool sendRawMsg(const char * msg, int size);

		//the received message length in bytes
		int getNumReceivedBytes();

		//send and receive raw bytes lets you send and receive
		//byte (char) arrays without modifiying or appending the data.
		//Strings terminate on null bytes so this is the better option
		//if you are trying to send something other than just ascii strings
		bool sendRawBytes(const char * rawBytes, const int numBytes);


		//get the message as a string
		//this will only work with messages coming via
		//send() and sendToAll()
		//or from messages terminating with the STR_END_MSG
		//which by default is  [/TCP]
		//eg: if you want to send "Hello World" from other
		//software and want to receive it as a string
		//sender should send "Hello World[/TCP]"
		string receive();

		//no terminating string you will need to be sure
		//you are receiving all the data by using a loop
		string receiveRaw();

		//pass in buffer to be filled - make sure the buffer
		//is at least as big as numBytes
		int receiveRawBytes(char * receiveBytes, int numBytes);

		//same as receive for binary data
		//pass in buffer to be filled - make sure the buffer
		//is at least as big as numBytes
		int receiveRawMsg(char * receiveBuffer, int numBytes);


		bool isConnected();
        bool isConnecting();
		int getPort();
		string getIP();

		//don't use this one
		//for server to use internally only!
		//--------------------------
		bool setup(int _index, bool blocking);



private:
		// private copy so this can't be copied to avoid problems with destruction
		ofxTCPClientThreaded(const ofxTCPManager & mom){};
		ofxTCPClientThreaded & operator=(const ofxTCPClientThreaded & mom){return *this;}

		friend class ofxTCPServer;

		ofxTCPManager	TCPClient;

		char			tmpBuff[TCP_MAX_MSG_SIZE+1];
		ofBuffer 		tmpBuffReceive;
		ofBuffer 		tmpBuffSend;
		string			str, tmpStr, ipAddr;
		int				index, messageSize, port;
		bool			connected;
        bool            connecting;
		string 			partialPrevMsg;
		string			messageDelimiter;
        bool            blocking;
};
