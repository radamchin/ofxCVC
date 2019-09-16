/*
	Blob
*/

#include "Blob.h"

Blob::Blob(int index) {
    _index = index;
}

Blob::~Blob() {

}

void Blob::init(int cvc_index, int id, float x, float y, float rect_x, float rect_y, float rect_width, float rect_height, float dx, float dy, float outlines[]){
    
    //cout << "Blob:init[" << _index << "] new id =" << id << endl;
    
    _cvc_index = cvc_index;
	_id = id;
	_updates = -1;
	_centroid.set(x,y);
	_decimal_centroid.set(dx,dy);
	_rect.set(rect_x,rect_y,rect_width,rect_height);
	
	_birth = ofGetElapsedTimeMillis();
	_last_update = ofGetElapsedTimeMillis();
	
	x_speed = 0.0f;
	y_speed = 0.0f;
	motion_speed = 0.0f;
	motion_accel = 0.0f;
	state = ADDED; 
	
    update(x, y, rect_x, rect_y, rect_width, rect_height, dx, dy, outlines);
	
	_inited = true;
}
	
void Blob::update(float x, float y, float rect_x, float rect_y, float rect_width, float rect_height, float dx, float dy, float outlines[]) {
	// could be used to re-use the object (would need to reset age too)

    if(isDead()) return; // should not be calling this
    
	//_outlines = outlines;
	_outlines_count = 0; //_outlines.length;
	
	// From tuio, NOT accurate if using smooth, maybe shoudl apply after smooth then?
	
	long diffTime = ofGetElapsedTimeMillis() - _last_update;
	float _dt = diffTime/1000.0f;
	float _dx = x - _centroid.x;
	float _dy = y - _centroid.y;
	float dist = (float)sqrt(_dx*_dx+_dy*_dy);
	float last_motion_speed = motion_speed;
	
	x_speed = _dx/_dt;
	y_speed = _dy/_dt;
	motion_speed = dist/_dt;
	motion_accel = (motion_speed - last_motion_speed)/_dt;
	updateState();

	if(CVC(_cvc_index)->SMOOTH_UPDATES && !isVirgin()) { // smooth
        // TODO: perhaps smoothing and amount could be pased, instead of looked up every frame?
		// lerp from existing position to the updated position, stopping jarring movements
		
        float t = CVC(_cvc_index)->UPDATE_INTERPOLATION;
        
		_centroid.x = ofLerp(_centroid.x, x, t);
		_centroid.y = ofLerp(_centroid.y, y, t);
		
		_rect.x = ofLerp(_rect.x, rect_x, t);
		_rect.y = ofLerp(_rect.y, rect_y, t);
			
		_rect.width = ofLerp(_rect.width, rect_width, t);
		_rect.height = ofLerp(_rect.height, rect_height, t);

		_decimal_centroid.x = ofLerp(_decimal_centroid.x, dx, t);
		_decimal_centroid.y = ofLerp(_decimal_centroid.y, dy, t);
		
		/*if(_outlines_count > 1) {
			// TODO: lerp these as well.
		}*/
		
	}else {
		_centroid.set(x, y);
		_rect.set(rect_x, rect_y, rect_width, rect_height);		
		_decimal_centroid.set(dx, dy);
	}

	_updates++;
	_last_update = ofGetElapsedTimeMillis();
}

void Blob::release(){
	state = REMOVED;
	
	_centroid.set(0,0);
	_decimal_centroid.set(0,0);
	_rect.set(0, 0, 0, 0);
    
    _id = -1;
    _outlines_count = -1;
	
    _birth = -1;
    _last_update = -1; // used for killing orphaned (static blobs)
	
    _updates = -1;
}	

void Blob::updateState() {
	
	if(motion_accel>0) {
		state = ACCELERATING;
	}else if(motion_accel<0) {
		state = DECELERATING;
	}else {
		state = STOPPED;
	}
	
}

void Blob::sum(Blob &b) {
	// merge with another blob, retaining my id and age properties.
	// stuff is added, and at end of add-ing session we would use div()
	
	// combine rects
	//  from java:bbc:RectangleF::unionSet
	float nx = min(_rect.x, b.getRect().x);
	float ny = min(_rect.y, b.getRect().y);
	float nw = max(_rect.x+_rect.width, b.getRect().x+b.getRect().width) - nx; // Furtherest right - the lowest x
	float nh = max(_rect.y+_rect.height, b.getRect().y+b.getRect().height) - ny; // furtherest left - the lowest y
	_rect.set(nx, ny, nw, nh);
	
	//_centroid.add(b.getPos());
	//_decimal_centroid.add(b.getDecPos());
	
	// TODO: outlines? find a combine algorithm? how to deal with non-intersecting things.
	
	x_speed += b.getXSpeed();
	y_speed += b.getYSpeed();
	motion_speed += b.getMotionSpeed();
	motion_accel += b.getMotionAccel();
}

void Blob::div(int i) {
	// divide the positional time things by a value
	
	if(i == 0) return;
	
	//_centroid.div(i);
	//_decimal_centroid.div(i);
	
	x_speed /= i;
	y_speed /= i;
	motion_speed /= i;
	motion_accel /= i;

	updateState();
}

/*string Blob::getSerializeRect(int fixed_places, bool int_to_char_encode){
	// TODO: use fixed_places &&&	import java.text.DecimalFormat;	 NumberFormat formatter = new DecimalFormat("0.00"); x = formatter.format(_bytesIncoming/(double)MB) + " mb";
	//return _id +"," + _rect.x.toFixed(fixed_places) + "," + _rect.y.toFixed(fixed_places) + "," + _rect.width.toFixed(fixed_places) + "," + _rect.height.toFixed(fixed_places);
	
	// currently returning rounded ints
	
	int x = round(_rect.x);
	int y = round(_rect.y);
	int w = round(_rect.width);
	int h = round(_rect.height);
	
	if(int_to_char_encode) {
		// NOTE: This technique has major shortcomings, mainly it is limited to sending values between 0 and 1114111 (Character.MAX_CODE_POINT)
		// 		 It may be all right for short running sessions but not for things that run for days as blob ids could get massive.
		 
		 // NOTE: This is not yet working, as it appears mpeClient or Server (or both) is fucking the encoding,
		 // 			may need to make sure all input/output streams in these are UTF-16.
	
		
		if(_id > Character.MAX_CODE_POINT) {
			// the id of the blob is too big for our encoding so lets modulate it back to something else
			 Logger.warn("Blob getSerializeRect _id " + _id  + " > Character.MAX_CODE_POint " + Character.MAX_CODE_POint + ". Encoding will fail.");
		}
					
		return (new String(Character.toChars(_id))) +"," + (new String(Character.toChars(x))) + "," + (new String(Character.toChars(y))) + "," + (new String(Character.toChars(w))) + "," + (new String(Character.toChars(h)));
		
	}else{	
		return _id +"," + x + "," + y + "," + w + "," + h;
	}
}*/


// TODO: could pass an ostringsteam & ref into these to append to

string Blob::getSerializedPosition(bool int_to_char_encode) {
    // return the id, x, y
	return getSerializedPoint(int_to_char_encode, _centroid.x, _centroid.y); 
}

string Blob::getSerializedRectPosition(bool int_to_char_encode) {
	// return the id, x, y
	return getSerializedPoint(int_to_char_encode, _rect.x, _rect.y); 
}

string Blob::getSerializedPoint(bool int_to_char_encode, float fx, float fy) {
	
    ostringstream out;
    
	if(int_to_char_encode) {
		// NOTE: This technique has major shortcomings, mainly it is limited to sending values between 0 and 1114111 (Character.MAX_CODE_POINT)
		 // 		 It may be all right for short running sessions but not for things that run for days as blob ids could get massive.
		 // NOTE: This is not yet working, as it appears mpeClient or Server (or both) is fucking the encoding,
		 //			may need to make sure all input/output streams in these are UTF-16.
		
        ofLogWarning("Blob::getSerializedPoint") << "int_to_char_encode not implemented!";
        
		/*if(_id > Character.MAX_CODE_POINT) {
			// the id of the blob is too big for our encoding so lets modulate it back to something else
			 Logger.warn("Blob getSerializedPoint _id " + _id  + " > Character.MAX_CODE_POint " + Character.MAX_CODE_POint + ". Encoding will fail.");
		}
					
		return (new String(Character.toChars(_id))) + "," + (new String(Character.toChars(x))) + "," + (new String(Character.toChars(y)));
	*/
        out << _id << "," << round(fx) << "," << round(fy);
        
	}else{
        out << _id << "," << round(fx) << "," << round(fy);
    }
        
    return out.str();
}

//----------------------------------------------------------------

string Blob::toMinString() {
    
    ostringstream out;
	
    out <<  "{index=" << _index<< ", id=" <<_id << ", dead=" << isDead() << ", virgin=" << isVirgin() << ", state=" << state << ":" + getStateString() << ", last_update=" << getLastUpdate() << ", age=" << getAge() << ", stagnation=" << getStagnation() << ", pos=" << _centroid << ", rect=" << _rect  << "}";
    
    return out.str();
}

string Blob::toString() {
	ostringstream out;
	
   out <<  "{Blob id=" <<_id << ", index=" << _index << ", pos=" << _centroid << ", rect=" << _rect << ", outlines=" << _outlines_count << ", age=" << getAge() << ", last_update=" << getLastUpdate() << ", stagnation=" << getStagnation() << ", x_speed=" << x_speed << ", y_speed=" << y_speed << ", motion_speed=" << motion_speed << ", motion_accel=" << motion_accel << ", state=" << state << ":" + getStateString() << "}";
	return out.str();
}

//----------------------------------------------------------------

bool Blob::isBrandNew() { 
	return _updates < 1; // just created as has not been update yet
} 
	
float Blob::getArea() { 
	return _rect.width * _rect.height; 
}

ofRectangle& Blob::getRect(){
	return _rect; 
}

ofVec2f & Blob::getPos(){
	return _centroid;	
}

ofVec2f & Blob::getDecPos(){
	return _decimal_centroid; 
}

int Blob::getID(){ 
	return _id; 
}

bool Blob::hasOutlines() { 
	return _outlines_count > 1;
}

/*float[] Blob::getOutlines() {
	return _outlines; 
}*/

int Blob::getOutlinesCount() { 
	return _outlines_count;
}

long Blob::getAge(){
	return ofGetElapsedTimeMillis() - _birth; // how old in milliseconds this blob is!
}
	
long Blob::getStagnation(){ 
	return ofGetElapsedTimeMillis() - _last_update; // how long it has been still
} 

long Blob::getLastUpdate(){ 
	return _last_update;
}

void Blob::setRect(const ofRectangle &r){ 
	_rect.set(r);
}	

/*void Blob::setOutlines(float *[] v) {
	_outlines = v;
}*/

void Blob::setIdHack(int v){
	_id = v;
} // dev hack to set the id (not to be used in real code)

//----------------------------------------------------------------	

float Blob::getXSpeed() { 
	return x_speed; // X velocity
} 
	
float Blob::getYSpeed() { 
	return y_speed; // Y velocity
} 

float Blob::getMotionSpeed() { 
	return motion_speed; 
}

float Blob::getMotionAccel() { 
	return motion_accel; 
}

int Blob::getState() { 
	return state; 
}

bool Blob::isMoving() {
	return ((state==ACCELERATING) || (state==DECELERATING)); 
}

string & Blob::getStateString() {
	// return small readable version of the state
	switch (state) {		
		case ADDED: case ACCELERATING: case DECELERATING: case STOPPED: case REMOVED: case PURGATORY:
			return getStateAbbrevNames()[state];
		default: return getStateAbbrevNames()[6]; // Unknown
	}
}

//----------------------------------------------------------------

/* TODO: this method
char *[] getAsVTMessageBytes(float w, float h, bool send_outline) { 
	// For use in re-broadcasting via VideoTracker
	// w & h are the dimensions we are scaling to
	// mimics videotracler.blobs.VideoBlob::getCorrectedMessageListBytes 
	
	// Hack workaround show stopper adamh 280513 for #capillary
	// update to see if it solves the ArrayIndexOutOFBoundsException issue
	_outlines_count = _outlines.length; 
	if(_outlines_count % 2 != 0) {
		_outlines_count--; // drop one down to make sure we get even values. This could be
		System.out.println("CVC: Blob.getAsVTMessageBytes just modded outlnes_count - 1 to " + _outlines_count + " to make even. Did it help?");
	}
	
	int length = 4 * 6; // x,y,rect.x,rect.y,rect.w,rect.h
	
	if(send_outline) length += _outlines_count * 2 * 4; // 2 values per outline point, 4 bytes per float
	
	byte[] out = new byte[length];
	
	int position = 0;
	position = ByteUtil.addFloatToBytes((_centroid.x/w), out, position); // x NOTE:could just use _decimal_centroid
	position = ByteUtil.addFloatToBytes((_centroid.y/h), out, position); // y NOTE:could just use _decimal_centroid
	position = ByteUtil.addFloatToBytes((_rect.x/w), out, position); // rect.x
	position = ByteUtil.addFloatToBytes((_rect.y/h), out, position); // rect.y
	position = ByteUtil.addFloatToBytes((_rect.width/w), out, position); // rect.width
	position = ByteUtil.addFloatToBytes((_rect.height/h), out, position); // rect.height
	
	if(send_outline && _outlines_count > 5){ // need 6 points to make a basic shape 
		// append the outlines corrected bytes.
		for(int i=0; i<_outlines_count; i+=2) {
			//Logger.print(i + " / " + outlines_length);
			position = ByteUtil.addFloatToBytes((_outlines[i]/w), out, position); // p.x
			position = ByteUtil.addFloatToBytes((_outlines[i+1]/h), out, position); // p.y
		}
	}
	
	return out;
}*/

//----------------------------------------------------------------	

vector<string>& Blob::getStateAbbrevNames(){
    static vector<string> state_names;
    if(state_names.size() == 0) {
        state_names.push_back("ADD");
        state_names.push_back("ACC");
        state_names.push_back("DEC");
	    state_names.push_back("STOP");
        state_names.push_back("REM");  // dead
        state_names.push_back("PURG"); // Unknown.
        state_names.push_back("UKWN");
    }
    return state_names;
}
