/*
	Blob
*/

#pragma once

#include "ofMain.h"
#include "ofxCVC.h"

class Blob {
    
	public:
		
		typedef enum {ADDED, ACCELERATING, DECELERATING, STOPPED, REMOVED, PURGATORY} BlobState;
		
		static vector<string>& getStateAbbrevNames();
		
        Blob(int index = -1);
		~Blob();

		void init(int _cvc_index, int id, float x, float y, float rect_x, float rect_y, float rect_width, float rect_height, float dx, float dy, float outlines[] = NULL);

		void update(float x, float y, float rect_x, float rect_y, float rect_width, float rect_height, float dx, float dy, float outlines[] = NULL);
		void release();		

		void sum(Blob &b); // merge with another blob, retaining my id and age properties.
		void div(int i); 	// divide the positional time things by a value

		///string getSerializeRect(int fixed_places, bool int_to_char_encode);
		string getSerializedPosition(bool int_to_char_encode);
		string getSerializedRectPosition(bool int_to_char_encode = false);
		string getSerializedPoint(bool int_to_char_encode, float fx, float fy);

		//----------------------------------------------------------------

		bool isBrandNew();// just created as has not been update yet			
		float getArea();
		
		ofRectangle&    getRect();
		ofVec2f&        getPos();
		ofVec2f&        getDecPos();
		
		int getID();
		
		bool hasOutlines();
		
		//&float[] getOutlines();
		
		int getOutlinesCount();
		
		long getAge();			
		
		long getStagnation();

		long getLastUpdate();
		//----------------------------------------------------------------

		//char *[] getAsVTMessageBytes(float w, float h, bool send_outline);

		//----------------------------------------------------------------

		void setRect(const ofRectangle &r);	

		//void setOutlines(float *[] v) { _outlines = v;

		void setIdHack(int v); // dev hack to set the id (not to be used in real code)

		//----------------------------------------------------------------	

		float getXSpeed(); // X velocity
			
		float getYSpeed(); // the Y velocity
		
		float getMotionSpeed();

		float getMotionAccel();

		int getState();

		bool isMoving();
    
		string & getStateString();
    
        bool isDead() { return state == REMOVED; }
    
        bool isVirgin() { return !_inited; }
    
        int getIndex() { return _index; } // position in holding data structure
		
        void setInPurgatory() { state = PURGATORY; } // inbetween alive and dead, used to signify the thing is released from pool, but not yet initied so it can't be used by some one else
    
        //----------------------------------------------------------------

		string toString();
        string toMinString();

	private:
    
        int         _index; // index it exists at in its data source
    
        int         _cvc_index; // index of the ofxCVC instance im associated with
    
	 	int 		_id = -1;
		ofVec2f 	_centroid; // centroid position (not necessarily the centre of the blob)
		ofRectangle _rect; // outside rectangle
		ofVec2f		_decimal_centroid; // 0..1
	
		//float	 	_outlines[0];
		int 		_outlines_count;
	
	 	long 		_birth = 0;
		long 		_last_update = 0; // used for killing orphaned (static blobs)
	
		float		x_speed; // The X-axis velocity value.
		float 		y_speed; // The Y-axis velocity value.
		float 		motion_speed; // The motion speed value.
		float 		motion_accel; // The motion acceleration value.	
	
		int         state = REMOVED; //  Reflects the current state
	
	 	int         _updates = -1;
	 	bool        _inited = false; // switched when init() called, but never set false again
		
		void updateState();
    
};
