/*
	BlobUpdateEvent
	Store of values to be sent to the update and create calls of BlobManager when it is updated 
	- Needed to implement the overwrite system in BlobManager
	- Will need to be ObjectPoolable maybe have an interface it needs to imlpement / object poolable that has clear and update methods...
*/
#pragma once


class BlobUpdateEvent {

	public:
		int event_id;
	
		int id;
		float x;
		float y;
		float rect_x;
		float rect_y;
		float rect_width;
		float rect_height;
		//float outlines[];
		float dx; // normailised x
		float dy; // normailised y
	
        BlobUpdateEvent();
	
		void init(int _event_id, int _id, float _x, float _y, float _rect_x, float _rect_y, float _rect_width, float _rect_height, float _dx, float _dy, float _outlines[] = NULL){
			update(_event_id, _id, _x, _y, _rect_x, _rect_y, _rect_width, _rect_height, _dx, _dy, _outlines);
		}
		
		void update(int _event_id, int _id, float _x, float _y, float _rect_x, float _rect_y, float _rect_width, float _rect_height, float _dx, float _dy, float  _outlines[] = NULL) {
			// Use to re-use the object (pool-able thing)
			event_id = _event_id;
			id = _id;
			x = _x;
			y = _y;
			rect_x = _rect_x;
			rect_y = _rect_y;
			rect_width = _rect_width;
			rect_height = _rect_height;		
			//outlines = _outlines;
			dx = _dx;
			dy = _dy;
		}
	
		void release() {
			clear();
		}
		
		void clear() {
			event_id = -1;
			id = -1;
			x = -1;
			y = -1;
			rect_x = -1;
			rect_y = -1;
			rect_width = -1;
			rect_height = -1;
			//outlines = NULL;
			dx = -1;
			dy = -1;
		}	
	
};

