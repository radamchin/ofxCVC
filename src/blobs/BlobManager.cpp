/*
	BlobManager
	On the job for all the Blobs
*/

#include "BlobManager.h"

BlobManager::BlobManager(int _cvc_index){
    cvc_index = _cvc_index;
    blob_list.setup();
}

bool BlobManager::createBlob(int id, float x, float y, float rx, float ry, float rw, float rh, float dx, float dy, float outlines[]) {
	// Queuing call
	
    if(blob_list.exists(id)){
		ofLogWarning("BlobManager::createBlob") << "- Blob of '" << id << "' already exists.";
		return false;
	}
	
	if(CVC(cvc_index)->USE_QUEUED_UPDATES) {
		// add here for a deferred render / mass update.
//		queueBlobEvent(BinSocket.CREATE_BLOB, id, x, y, rx, ry, rw, rh, outlines, dx, dy);
	}else{
		return _createBlob(id, x, y, rx, ry, rw, rh, dx, dy, outlines, false);
	}
	
	return true;
}

bool BlobManager::_createBlob(int id, float x, float y, float rx, float ry, float rw, float rh, float dx, float dy, float outlines[], bool check_exists) {
	// Deferred but direct call
	
    // TODO: could pass an arg here to no recheck it exists, as if called from createBlob, that has already occured once.
    
	//ofLogNotice("_createBlob") << '" << id << "'";
	
    lock();
    
    if(check_exists) {
        if(blob_list.exists(id)){
            ofLogWarning("BlobManager::createBlob")  << "- Blob of '" << id << "' already exists.";
            unlock();
            return false;
        }
    }
		
    blob_list.getFree()->init(cvc_index, id, x, y, rx, ry, rw, rh, dx, dy, outlines);
	
    _modifications_count++;
    
    unlock();
    
	return true;
}

bool BlobManager::updateBlob(int id, float x, float y, float rx, float ry, float rw, float rh, float dx, float dy, float outlines[]) {
	// Queuing call
	   
    if(!blob_list.exists(id)){       
        //ofLogWarning("BlobManager::updateBlob")  << "- Blob of '" << id << "' does not exists.";
        // Create it instead as we may have started the flash of set of the Java - ?
        createBlob(id, x, y, rx, ry, rw, rh, dx, dy, outlines);
        return false;
    }
	
	//ofLogNotice("updateBlob") << id << ", x=" << x << ", y=" << y << ", outlines.length=" << outlines.length;
	if(CVC(cvc_index)->USE_QUEUED_UPDATES) {
//		queueBlobEvent(BinSocket.MOVE_BLOB, id, x, y, rx, ry, rw, rh, outlines, dx, dy);
	}else{
		_updateBlob(id, x, y, rx, ry, rw, rh, dx, dy, outlines, false);
	}
	
	return true;
}

bool BlobManager::_updateBlob(int id, float x, float y, float rx, float ry, float rw, float rh, float dx, float dy, float outlines[], bool check_exists) {
	// Deferred but direct call
	
    lock();
    
    if(check_exists) { 
        if(!blob_list.exists(id)){   
            ofLogWarning("BlobManager::_updateBlob") << " - Blob of '" << id << "' does not exists. Might have been deleted";
            unlock();
            return false;
        }
    }
    
	_modifications_count++;
	
	blob_list.get(id)->update(x, y, rx, ry, rw, rh, dx, dy, outlines);
    
    unlock();
    
	return true;
}

bool BlobManager::destroyBlob(int id, DestroyOrigin origin) {
	
	// Destroys happen instantly as there can be no overwrites on them, ie blob_id should never come back
	
    //ofLogNotice("") << id << ":" << origin;
    
    lock();
    
	if(!blob_list.exists(id)){ 
		ofLogWarning("BlobManager::destroyBlob") <<  "Blob of '" << id << "' does not exists. origin=" << origin;
        unlock();
		return false;
	}
	
	/*if(ofxCVC::USE_QUEUED_UPDATES) {
		queueBlobEvent(BinSocket.DESTROY_BLOB, id);
	}else{*/
		_destroyBlob(id, origin, false);
	//}
	
    unlock();
    
	return true;
}

bool BlobManager::_destroyBlob(int id, DestroyOrigin origin, bool check_exists) {
	// Destroys happen instantly as there can be no overwrites on them, ie blob_id should never come back
        
    if(check_exists) {
        if(!blob_list.exists(id)){        
            ofLogWarning("BlobManager::_destroyBlob") << "- Blob of '" << id << "' does not exists. origin=" << origin;
            return false;
        }
    }
	
	_modifications_count++;
	
	_destroyed_blobs.push_back(id);
    
    blob_list.release(id);
    	
    //ofLogNotice("_destroyBlob") << id << ":" << origin;
    
	return true;
}

//-------------------------------------------------------------------------------------
bool BlobManager::hasBlobs() {
    return blob_list.total() > 0;
}
int BlobManager::getCount() {
    return blob_list.total();
}

//-----------------------------------------------------------------------------

void BlobManager::sweepForStagnants() {
	// Look for blobs older than BLOB_STAGNATION_KILL_AGE and destroy them
    
    int stagnation_kill_age = CVC(cvc_index)->BLOB_STAGNATION_KILL_AGE;
   
    lock();
    
    if(hasBlobs()) {
        
        Blob* b = blob_list.first();
     
        //ostringstream out;
        
        while(b) {
            
            // out << b->getIndex() << ",";
            
            if(b->getStagnation() >= stagnation_kill_age){
                //ofLogNotice("REM STAGNAT BLOB") << b.getID() << ", " << b.getStagnation() << ":" << stagnation_kill_age;
                _destroyBlob(b->getID(), STAGNANT, false);
                // TODO: optimisation would be to send in blob so blob_list did not have to look it up
            }
            
            b = blob_list.next(b->getIndex());
        }
        
       // cout << out.str() << endl;
    }
    
    unlock();
    
}

//-------------------------------------------------------------------------------------

bool BlobManager::lock(){
    
    if(!CVC(cvc_index)->DATA_LOCKABLE) return false;
    
  //  cout << "BlobManager::lock" << endl;
    
    blob_list.lock();
    
    /*if(!mutex.tryLock()){
       // ofLogWarning("BlobManager") << "mutex is busy - already locked";
        return false;
    }*/
	
    return true;
}

void BlobManager::unlock(){
    
    if(!CVC(cvc_index)->DATA_LOCKABLE) return;
    
	//mutex.unlock();
    blob_list.unlock();
}

//-------------------------------------------------------------------------------------
// Update / Overwrite methods
//-------------------------------------------------------------------------------------
/*
void BlobManager::applyQueuedUpdates() {
	// walk _update_list and pass on any updates needed - will need to store the updates in some fasion. TO pass on
	//System.out.println("renderUpdates:"+getTimer());
	BlobUpdateEvent e;
	
	// was getting thread based java.util.ConcurrentModificationException
	
	for (Map.Entry<Integer,BlobUpdateEvent> entry : _update_list.entrySet()) {
   		e = entry.getValue();
   	
		if(e.event_id == BinSocket.MOVE_BLOB) {
			_updateBlob(e.id, e.x, e.y, e.rx, e.ry, e.rw, e.rh, e.outlines, e.dx, e.dy);
		}else if(e.event_id == BinSocket.CREATE_BLOB) { // assume BinSocket.CREATE_BLOB 
			_createBlob(e.id, e.x, e.y, e.rx, e.ry, e.rw, e.rh, e.outlines, e.dx, e.dy);
		}else { // assume BinSocket.DESTROY_BLOB
			_destroyBlob(e.id, EVENT_QUEUE);
		}	
		
		_blob_pool.recycleUpdateEvent(e);
	}

	_update_list.clear();
}
*/

/*
bool _queueBlobEvent(int event_id, int id, float x, float y, float rx, float ry, float rw, float rh, float dx, float dy, float outlines[]) {
	
	// Store in _update_list for later firing
	
	//_update_list.containsValue(id);
	
	//synchronized(_update_list) {
	
		if(_update_list.count(id)) { // Already exists so update it instead
			_overwrites++;
			//System.out.println("overwrite occured for " + id);
			
			BlobUpdateEvent e = _update_list.get(id);
			
			if(e == null) {
				Logger.warn("BlobManager:queueBlobEvent Already existing id of '" + id + "' returned a null event");
				return false;
			}
			
			if(e.event_id == BinSocket.DESTROY_BLOB) {
				System.out.println("WARNING: Trying to overwrite blob event queue of event_id:DESTROY_BLOB with event_id#" + event_id + " on blob " + id);
				return false;
			}else{
				// Reuse the Existing event.
				e.update(event_id, id, x, y, rx, ry, rw, rh, dx, dy, outlines);
			}
			
		}else{ 
			BlobUpdateEvent e = null;
			
			while(e == null) e = _blob_pool.getUpdateEvent();
			
			//if(e == null) System.err.println("NULL update event returned from pool, with length of " + _blob_pool.getEventAvailable()); 
				
			e.init(event_id, id, x, y, rx, ry, rw, rh, dx, dy, outlines);
			_update_list.put(id, e);
		}
		
	//}
	
	return true;
}
*/
//-------------------------------------------------------------------------------------

void BlobManager::updateBlobsFromCentroidRectStates(const int *buf, int len) {
	// blob_values = [id,centroid_x,centroid_y,rectw,recth,...]
	
    
    
	// auto making the rect x,y from the projection from centroid -(width/2 & (-height/2)
	//cout << endl << "ids from buffer:";
    for(int i = 0; i<len; i+=5) {
      //  cout << buf[i] << ",";
		updateBlob(	buf[i], // id
					buf[i+1], buf[i+2], // x, y
					buf[i+1]-(buf[i+3]*.5f), buf[i+2]-(buf[i+4]*.5f),  // r.x, r.y
					buf[i+3], buf[i+4], // r.w, r.h
					0.0f, 0.0f, // dx, dy, TODO: calcluate from x,y against the display width and height
                    NULL //outlines
                   );  
	}
    
}

/*
void BlobManager::updateBlobsFromRectStates(int blob_values[]) {
	// blob_values = [id,rectx,recty,rectw,recth,...]
	
	int c = blob_values.length;
	
	// System.out.println("updateBlobsFromRectStates:length = " + c  + ", mod%5=" + (c%5) + ", blobs=" + (c/5));
	
	// auto making the centroid x,y from the centre of the rect 	
	
	for(int i = 0; i<c; i+=5) {
		updateBlob(	blob_values[i], 
					blob_values[i+1]+(blob_values[i+3]*.5f), blob_values[i+2]+(blob_values[i+4]*.5f), 
					blob_values[i+1], blob_values[i+2], 
					blob_values[i+3], blob_values[i+4], 
					_empty_outlines,
					0f, 0f
					);
	}
	
	// Deletes will be dealt with the stagnant sweeper turned up high.		
}
*/
//-------------------------------------------------------------------------------------
bool BlobManager::hasDestroyedBlobIds() {
    return _destroyed_blobs.size() > 0;
}

void BlobManager::flushDestroyedCache() {
	if(_destroyed_blobs.size() > 0) _destroyed_blobs.clear(); // zero it.;
}		

vector<int> * BlobManager::getDestroyedBlobIds(){
	return &_destroyed_blobs;
}

/*
string BlobManager::getBlobRectsStringSerialised(int fixed_places, bool int_to_char_encode) {
	//return the blobs as strings. id,rx,rect,rw,rh
	String out = "";		
	int i = 0;
	
	for (Map.Entry<Integer,Blob> entry : _blob_table.entrySet()) {
   		Blob blob = entry.getValue();
   		out += (i == 0 ? "" : ",") + blob.getSerializeRect(fixed_places, int_to_char_encode);
		i++;
	}
	
	return out;
}
*/

string BlobManager::getSerialisedBlobPositions(bool int_to_char_encode) {
    //return the blobs as strings. id,centroid_x,centroid_y
	ostringstream out;
	int i = 0;
	
    lock();
      
    if(hasBlobs()) { 
        Blob* b = blob_list.first();
        
        while(b) {
            out << (i == 0 ? "" : ",") << b->getSerializedPosition(int_to_char_encode);
            i++;
            b = blob_list.next(b->getIndex());
        }
            
    }
    
    unlock();
    
	return out.str();
}

/* // NOTE this would be more useful as the centre,x and y of the rect, 
string BlobManager::getSerialisedBlobRectPositions(bool int_to_char_encode) {
	//return the blobs as strings. id,rect.x,rect.y
	ostringstream out;
	int i = 0;
	
    lock();
    for (BlobIterator iterator=_blob_table.begin(); iterator!=_blob_table.end(); ++iterator){
		// iterator->first = (int)key iterator->second = (Blob)value
		Blob b = *(iterator->second);
        out << (i == 0 ? "" : ",") << b.getSerializedRectPosition(int_to_char_encode);
        i++;
	}
    unlock();
    
	return out.str();
}
*/

void BlobManager::clear() {		
	// Remove all blobs
    lock();
    
    if(hasBlobs()){
        Blob* b = blob_list.first();
        while(b) {
            _destroyBlob(b->getID(), CLEAR);
            b = blob_list.next(b->getIndex());
        }
    }
    
	unlock();
}

string BlobManager::toString() {
	ostringstream out;
	out << "{BlobManager blobs="<< getCount() << ", overwrites=" << _overwrites << ", updates_awaiting=" << _update_list.size() <<
			", destroyed=" << _destroyed_blobs.size()  << ", mods=" << _modifications_count << "}";
	return out.str();
}
