/*
	Disrupt

	Takes the blobs (virtual mouses) and applies their forces to fluid
*/

#pragma once


#include "ofMain.h"
#include "ofEvents.h"
#include "Disruptor.h"

class Disrupt {
	
	private:
		
		map<int,Disruptor*> _table;
    
        int cvc_index;
    
	public:
		
		typedef map<int, Disruptor*>::iterator DisruptorIterator;
		
        bool smooth = true; // Disruptor updates to be set to inbetween where the update is and was to help smooth our the VideoTracker updates
		float interpolation = 0.15f; // amount to lerp between update and current position
		
		ofTrueTypeFont font;
	
		//---------------------------------------------------------------------------
		
        Disrupt() {
        
        }
    
        void setup(int _cvc_index) {
            cvc_index = _cvc_index;

            
            font.loadFont(OF_TTF_MONO, 8, false);
            
            // Need to change so ChangeEvent is specific to instance of CVC..
            
           // ofAddListener(ChangeEvent::blobs_destroyed, this, &Disrupt::cvcBlobsDestroyed);
          //  ofAddListener(ChangeEvent::blobs_update, this, &Disrupt::cvcBlobsUpdated);
            
            ofAddListener(CVC(cvc_index)->blobs_destroyed_event, this, &Disrupt::cvcBlobsDestroyed);
            ofAddListener(CVC(cvc_index)->blobs_update_event, this, &Disrupt::cvcBlobsUpdated);
            
            
            ofAddListener(ofEvents().update, this, &Disrupt::update);
            
            // TODO remove listeners, eg ofRemoveListener(ofEvents().update, this, &Disrupt::update);
        }
	
		//---------------------------------------------------------------------------
	
		void draw(int x = 0, int y = 0) {
			// Debug draw all the disruptors
		
			ofPushMatrix();
			ofTranslate(x, y);
            
			for (DisruptorIterator iterator=_table.begin(); iterator!=_table.end(); ++iterator){
				Disruptor d = *(iterator->second);
                
                ofSetRectMode(OF_RECTMODE_CENTER);
                
                ofFill();
				ofSetColor(255, 255, 255, 96);
				ofEllipse(d.position.x, d.position.y, 30, 30);
                
                ofNoFill();
				ofSetLineWidth(1);
                ofSetColor(0, 255, 0, 255);
				ofEllipse(d.position.x, d.position.y, 30, 30);
                
				// centre cross
				ofLine(d.position.x-10, d.position.y, d.position.x + 10, d.position.y);
				ofLine(d.position.x, d.position.y-10, d.position.x, d.position.y + 10);
                
                // label
                ofSetRectMode(OF_RECTMODE_CORNER);
				string label = ofToString(d.id);
			
				ofFill();
				ofSetColor(255);
				ofRect(d.position.x+2,d.position.y+2,font.stringWidth(label)+6,font.getLineHeight()+2);
				ofSetColor(0);
				font.drawString(label,d.position.x+4, d.position.y+font.getLineHeight()+1);
	    	}
		
			ofPopMatrix();
		}
    
    
        //---------------------------------------------------------------------------
    
        void cvcBlobsDestroyed(ChangeEvent &e) {
            //cout << "Disrupt::cvcBlobsDestroyed count=" << e.destroyed_blobs->size() << endl;
            for(int i = 0; i<e.destroyed_blobs->size();i++) removeDisruptor(e.destroyed_blobs->at(i));
            
        }
        
        void cvcBlobsUpdated(ChangeEvent &e) {
            //cout << "Disrupt::cvcBlobsUpdated count=" << e.blob_state->size() << endl;
            
            /*for (BlobManager::BlobIterator iterator=e.blob_state->begin(); iterator!=e.blob_state->end(); ++iterator){
                // iterator->first = (int)key iterator->second = (Blob)value
                Blob * b = iterator->second;
                updateDisruptor(b->getID(), b->getPos().x, b->getPos().y);
            }*/
            if(e.bloblist->total() > 0) {
                Blob* b = e.bloblist->first();
            
                while(b) {
                    updateDisruptor(b->getID(), b->getPos().x, b->getPos().y);
                    b = e.bloblist->next(b->getIndex());
                }
            }
            
        }
    
		//---------------------------------------------------------------------------
		
		void update(ofEventArgs& args) {
	
			// affect the Fluid with available disruptors
			if(count() == 0) return;
		
			for (DisruptorIterator iterator=_table.begin(); iterator!=_table.end(); ++iterator){
				Disruptor d = *(iterator->second);
			
				if(d.getTimeSinceLastUpdate() > 5000) { // TODO: turn it back onto Prefs.disruptor_stagnation_time_ms
					// its over 5 seconds since it was updated so kill it. (An extra safety measure)
					ofLogNotice("FluidDisruption") << " found stagnant disruptor, id= " << d.id << ". About to remove";
					removeDisruptor(d.id);
				}else {
					if(d.getUpdates() > 1) { // write to fluid
					
						// Fluid::instance()->addScreenForce(d.position.x, d.position.y, d.last_position.x, d.last_position.y, true);
					
						/*// From memo tuio code
						 // Note: TUIO cursor coords are in 0..1
						  for (int i=0;i<tuioCursorList.size();i++) {
						        TuioCursor tcur = (TuioCursor)tuioCursorList.elementAt(i);
						        float vx = tcur.getXSpeed() * tuioCursorSpeedMult;
						        float vy = tcur.getYSpeed() * tuioCursorSpeedMult;
						        if(vx == 0 && vy == 0) {
						            vx = random(-tuioStationaryForce, tuioStationaryForce);
						            vy = random(-tuioStationaryForce, tuioStationaryForce);
						        }
						        addForce(tcur.getX(), tcur.getY(), vx, vy);
						    }
						  */
					}
				
				}
		
			}
		
		}	

		//---------------------------------------------------------------------------
	
		map<int, Disruptor*> * getDisruptors() {
			return &_table;
		}
		
		bool _createDisruptor(int id) {

			if(_table.count(id)) {
				ofLogWarning("FluidDisruption") << "create - Disruptor of '" << id << "' already exists.";
				return false;
			}

			_table[id] = new Disruptor(id);
			return true;
		}
		
		void updateDisruptor(int id, float x, float y) {
		
			if(!_table.count(id)) {
				bool success = _createDisruptor(id);
			}
		
			_table[id]->move(x, y);		
		}
	
		void removeDisruptor(int id) {
		
			if(_table.count(id)) {
				_table[id]->release();
				delete _table[id];
				_table.erase(id); // remove from map
			}
		
		}
	
		void reset() {
			// Try clearing manually (to dealloc pointers)
            ofLogNotice("Disrupt") << "reset";
            for (DisruptorIterator iterator=_table.begin(); iterator!=_table.end(); ++iterator){
				int d_id = (iterator->first);
                removeDisruptor(d_id);
            }
            
			_table.clear();
		}
	
		int count() {
			return _table.size();
		}
	
		string toString() {
			return "{Disrupt count=" + ofToString(count()) + ", smooth=" + ofToString(DISRUPTOR_SMOOTH) + ", lerp=" + ofToString(DISRUPTOR_INTERPOLATE) + "}";
		}
	
};
