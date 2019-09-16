/*
	BlobList
    Blob Holder

	Data structure that holds blobs and provides interface to get at them.

*/

#include "BlobList.h"

#include "Blob.h"

BlobList::BlobList () {
    
}

void BlobList::setup() {
    
    ofLogNotice("BlobList:setup") << MAX_BLOB_COUNT;
    
    //_data = new vector<Blob*>();
    
    _data.reserve(MAX_BLOB_COUNT); //
    
    for(int i = 0; i<MAX_BLOB_COUNT; i++) {
        _data.push_back(new Blob(i));   // we store its index, in it
    }
   
}

Blob* BlobList::get(int blob_id) {
    // up to user to call exists first
    for(int i = 0; i<MAX_BLOB_COUNT; i++){
        if(_data[i]->getID() == blob_id) {
            return _data[i];
        }else if(_data[i]->isVirgin()) { // we've hit ceiling of used blobs so outta here now
            break;
        }
    }
    
    return NULL;
}

Blob* BlobList::at(int index) {
    if(index >= MAX_BLOB_COUNT || index < 0) {
        ofLogError("ofxCVC::BlobList:: index out or range") << index << " Mapping to last, but will have side effects";
        index = MAX_BLOB_COUNT-1;
    }
    
    return _data[index]; // Not sure if this is useful.
}

Blob* BlobList::getFree() {
    // Find first dead (free) blob
    
    for(int i = 0; i<MAX_BLOB_COUNT; i++){
        if(_data[i]->isDead()) {
            _data[i]->setInPurgatory(); // mark this undead, callee must do something with it or it will for ever be in limbo!
            _total++;
            
//            /cout << "BlobList::getFree:found @ " << i << ", id=" << _data[i]->getID() << ", _total now " << _total << endl;
            return _data[i];
        }
    }
    
    ofLogError("BlobList::getFree Exhausted MAX_BLOB_COUNT") << MAX_BLOB_COUNT << " you gunna crash. Sorry we should have never got here";
    
    return NULL;
}

bool BlobList::exists(int blob_id) {
    
    for(int i = 0; i<MAX_BLOB_COUNT; i++){
        if(!_data[i]->isDead()) {
            if(_data[i]->getID() == blob_id) {
                return true;
            }
        }else if(_data[i]->isVirgin()) {
            // never initialised so don't need to go beyond in the loop
           return false;
        }
    }
    
    return false;
}

void BlobList::release(Blob* b) {
    releaseAt(b->getIndex());
}

void BlobList::release(int blob_id) {    
    releaseAt(get(blob_id)->getIndex());    
}

void BlobList::releaseAt(int index) {
    
    if(index >= MAX_BLOB_COUNT || index < 0) {
        ofLogError("ofxCVC::BlobList:: index out or range") << index << " Mapping to last, but will have side effects";
        index = MAX_BLOB_COUNT-1;
    }
    
    //cout << "BlobList::releaseAt:" << index << " id=" << _data[index]->getID();
    
    _data[index]->release();
    _total--;
    
    //cout << " total now=" << _total << endl;
    
}
Blob* BlobList::first() {
    return next(-1);
}
Blob* BlobList::next(int index) {
    
    // used for walking, iterating. Linked list stylie, but not a linked list
    
    /*
     Usage:
     Blob *b; = blob_list.first();
     
     while(b) {
        // do stuff
        b = blob_list.next(b->getIndex());
     }
          
     */
    
    index++;
    
    if(index >= MAX_BLOB_COUNT) return NULL;
    
    if(_data[index]->isVirgin()) return NULL;
    
    if(_data[index]->isDead()) { // walk list until we find one thats not dead
        while(_data[++index]->isDead()) {
            if(_data[index]->isVirgin()) { //  first that has never been assigned
                break;
            }else if(index == MAX_BLOB_COUNT-1) { // end of line
                break;
            }
       }
    }
    return _data[index]->isVirgin() ? NULL : _data[index];
}

int BlobList::total() {
    return _total;
}

bool BlobList::lock() {
    
    /*if(!mutex.tryLock()){
     // ofLogWarning("BlobList") << "mutex is busy - already locked";
     return false;
     }*/
    
    mutex.lock();
    
    return true;
}

void BlobList::unlock(){
    mutex.unlock();
}
