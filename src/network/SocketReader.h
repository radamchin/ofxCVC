/*
	SocketReader
	Connets to VideoTracker HTTP Socket, and reads and transforms data
	Note: Design is merge of 2 cvc java classes, SocketServer & BinSocket
*/

#pragma once

#ifndef _CVC_SOCKET_READER
#define _CVC_SOCKET_READER

#include "ofMain.h"
#include "ofEvents.h"
#include "ofxTCPClientThreaded.h"
#include "DataUtil.h"

#define MAX_BUFFER_SIZE  32768 // 1024*32
#define MIN_CVC_MSG_SIZE  7 // 2+1+4 a destroy blob message. entity,command,blob_id

class SocketReader : public ofThread {
    
	public:
    	
		bool run_as_thread;
    
        static const int DEFAULT_RECONNECT_INTERVAL_SECS = 15;
        int reconnect_interval; // milliseconds
    
        bool auto_connect;
        bool auto_reconnect;
        
        const int MESSAGE_HEADER_LENGTH = 2;
        const int BASE_MESSAGE_LENGTH = (MESSAGE_HEADER_LENGTH + 31);  //given there is 33 bytes in a base message, and our base header length is 2
        // 2 + 1 + 4 + (4 * 6); // (short)start_entity, (char)comand, (int)blob_id, (float),x,y,rx,ry,rw,rh
        
        const short MESSAGE_START = -1000; // The message start delimiter, Same as videotracker.blobs.BlobMessager.START_ENTITY
        const short MESSAGE_END = -2001; // The message end delimiter, same as videotracker.blobs.BlobMessager.END_ENTITY
        
        // Message ids
        static const char CREATE_BLOB = 0;
        static const char UPDATE_BLOB = 1;
        static const char DESTROY_BLOB = 2;
        
        static const int BLOB_ID_OVERLAP_BUFFER = 100000;   // Amount to shift blob ids, based on index
                                                            // **May need to increase this if its running for a long time (and blob ids are getting above it.
           
        ofRectangle scaler; // Dimensions blob normalised positions are scaled to
        
        int index;
        string host;
        int port;

        //------------------------------
    
        SocketReader();
    
		void setup(int _cvc_index, int _index = 0, string _host = "127.0.0.1", int _port = 1000,
                   bool _run_as_thread = false,
                   bool _auto_connect = true, bool _auto_reconnect = true, float _reconnect_interval_secs = DEFAULT_RECONNECT_INTERVAL_SECS);
  
        void connect();
		void connect(string _host, int _port);
        void connected(); // when finally connected
        bool disconnect();
    
        void threadedFunction();
        void _update(ofEventArgs& args); // ofListener method
    
		void update();
        void close();
		
		void resize(float x, float y, float w, float h);
    
        bool isConnected();
        bool isConnecting();
    
		string toString();
        string toMinString();
    
    private:
    
        ofxTCPClientThreaded tcp_client;
    
        int cvc_index; // index of ofxCVC that i'm paired with
    
        bool _connected = false;
        bool _connecting = false;
    
        int last_connect_attempted_time;
        
        char read_buffer[MAX_BUFFER_SIZE]; // client reads to this buffer
        char write_buffer[MAX_BUFFER_SIZE]; // valid message gets written to this buffer.
    
        int sizeRead;
        int position = 0; // current position in buffer
    
        bool parseMessage(const char * buf, int len);
        void mapOutlines(float *outlines, int *len);
    
        void checkConnection();
    
        inline int mapBlobID(int id) { // Incases of multiserving, shift the ids of each soecket into their own range.
            return (id + (index * SocketReader::BLOB_ID_OVERLAP_BUFFER));
        }
        
};

#endif
