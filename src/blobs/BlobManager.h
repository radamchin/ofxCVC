/*
	BlobManager
	Manage Blob's
*/

#pragma once
#include "ofMain.h"
#include "ofxCVC.h"
#include "Blob.h"
#include "BlobUpdateEvent.h"

#include "BlobList.h"

class Blob;

class BlobManager {
    
	public:
		
        typedef enum {TRACKING, EVENT_QUEUE, STAGNANT, INTESECTION, CLEAR, OTHER} DestroyOrigin; // ids used to identify the event that caused a destroy call
	
		BlobManager(int _cvc_index);
		~BlobManager();
	
		bool createBlob(int id, float x, float y, float rx, float ry, float rw, float rh, float dx, float dy, float outlines[] = NULL);
		bool updateBlob(int id, float x, float y, float rx, float ry, float rw, float rh, float dx, float dy, float outlines[] = NULL);
		bool destroyBlob(int id, DestroyOrigin origin = OTHER);
	
		void sweepForStagnants();
	//	void applyQueuedUpdates();
	//	void joinIntersected();
	
		void updateBlobsFromCentroidRectStates(const int *buf, int len);
	//	void updateBlobsFromRectStates(int blob_values[] );
	
		void flushDestroyedCache();
		vector<int> * getDestroyedBlobIds();
        bool hasDestroyedBlobIds();
	
        bool hasBlobs();
        int getCount();
	
		void resetModCount() {_modifications_count = 0;};
		int getModCount() {return _modifications_count;};	
	
	//	string getBlobRectPositionsStringSerialised(bool int_to_char_encode = false);
		string getSerialisedBlobPositions(bool int_to_char_encode = false); // centroid x,y
    //    string getSerialisedBlobRectPositions(int fixed_places, bool int_to_char_encode = false); // rect.x, rect.y
	
		long getOverwrites() {return _overwrites;}; //Number of overwrites that have occurred

		void clear();  // remove all blobs
		string toString();
        
        bool lock();
        void unlock();
    
        BlobList blob_list;
    
	private:
    
        int cvc_index;
    
        mutable ofMutex mutex;
        int lock_count;
    
		int _modifications_count = 0;
		long _overwrites = 0; // counter of when an update overwrites something in the update_list;
		
        vector<int> _destroyed_blobs; // store the id's of the blobs destroyed last tick. it will be purged externally every tick after being read
	 	map<int, BlobUpdateEvent*> _update_list; // id lut of values to send in at updates. is overwritten if id already exists and then flushed during updateRender phase
		
		// float[] _empty_outlines = new float[0];
		
		// TODO: intersection vars
		//ArrayList<ofRectangle> intersected_rects = new ArrayList<ofRectangle>(); // super rects of intersections (class scope for debug rendering during dev)
		//RectGroup rgroup = new RectGroup();
		//ArrayList<ofRectangle> intersected_rects2 = new ArrayList<ofRectangle>();
		
        bool _createBlob(int id, float x, float y, float rx, float ry, float rw, float rh, float dx, float dy, float outlines[] = NULL, bool check_exists = true);
		bool _updateBlob(int id, float x, float y, float rx, float ry, float rw, float rh, float dx, float dy, float outlines[] = NULL, bool check_exists = true);
		bool _destroyBlob(int id, DestroyOrigin origin = OTHER, bool check_exists = true);
	
		// bool _queueBlobEvent(int event_id, int id, float x, float y, float rx, float ry, float rw, float rh, float dx, float dy, float outlines[]);
		
};