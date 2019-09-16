/*
	ChangeEvent.h
	
	Use ofEvent infrastructure to broadcast to changes to the ofxCVC state (as apposed to direct update calls on Visualisation classes)
*/

#pragma once

#include "ofMain.h"
#include "BlobManager.h"
#include "Blob.h"
#include "BlobList.h"

class ChangeEvent : public ofEventArgs {
    
    public:

        /*enum Type{
            destroy,
            update
        } type;*/
        
        vector<int> * destroyed_blobs;
        //map<int, Blob*> * blob_state;
    
        BlobList* bloblist;
    
        ChangeEvent() {
            destroyed_blobs = NULL;
            //blob_state      = NULL;
            bloblist = NULL;
        }
        
       // static ofEvent <ChangeEvent> blobs_destroyed;
       // static ofEvent <ChangeEvent> blobs_update;
    
    
};

