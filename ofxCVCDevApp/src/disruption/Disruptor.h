/*
  Disruptor
 
    Blob type class of the forces disrupting the fluid
 	Created, updated and destroyed based on ofxCVC blob's
    
    These are the things whose positions effect the fluid!

 * */

#pragma once

#define DISRUPTOR_SMOOTH true
#define DISRUPTOR_INTERPOLATE 0.15

class Disruptor {
	
	private:

		long birth;
		long last_update;
		int updates = 0;
		bool _init = false;
    
	public:
		
        ofVec2f position;
		ofVec2f last_position;
		int id;
		
		Disruptor(int _id) {
			id = _id;
			birth = last_update = ofGetElapsedTimeMillis();
		}
				
		void move(float x, float y) {
		
			updates++;
		
			if(_init) {
				if(DISRUPTOR_SMOOTH) {
					x = ofLerp(last_position.x, x, DISRUPTOR_INTERPOLATE);
					y = ofLerp(last_position.y, y, DISRUPTOR_INTERPOLATE);
				}
				last_position.set(position);
			}else { 
				// make sure the lastpos is the first pos (not, 0,0)
				_init = true;
				last_position.set(x,y);
			}
		
			position.set(x,y);

			last_update = ofGetElapsedTimeMillis();
		}
	
		int getUpdates() {
			return updates;
		}
	
		long getAge() {
			return ofGetElapsedTimeMillis() - birth;
		}
	
		long getTimeSinceLastUpdate() {
			return ofGetElapsedTimeMillis() - last_update;
		}	
	
		void release() { // or reset, to use for pooling.
			_init = false;
			updates = 0;
		}
	
		string toString() {
            ostringstream out;
            out << "{disruptor id=" << id << ", pos=" << position << "lpos=" << last_position << "}";
			return out.str();
		}
	
};
