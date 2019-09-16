/*
	BlobList
    Blob Holder

	Data structure that holds blobs and provides interface to get at them.

*/

#pragma once
#include "ofMain.h"

#define MAX_BLOB_COUNT 512 // buffer maximum number of blobs used at once
class Blob;

class BlobList {
    
	public:
    
        BlobList();
        void setup();
    
        Blob* get(int blob_id);
        Blob* at(int index);
    
        Blob* getFree();
    
        bool exists(int blob_id);
    
        void release(Blob* b);
        void release(int blob_id);
        void releaseAt(int index);
    
        bool lock();
        void unlock();
        
        Blob* first();
        Blob* next(int index = -1);
    
        int total();
    
	private:
    
        ofMutex mutex;
    
        int _total = 0; // counter of the active allocated blobs
    
        vector<Blob*> _data;
    
       // int _highest_used_index = 0;
		
};