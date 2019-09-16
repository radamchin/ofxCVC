/*
	DebugRenderer
	Draws all blobs outlines points etc,
	
*/

#pragma once
#include "ofMain.h"
#include "ofEvents.h"
#include "ofxCVC.h"
#define B_COLORS_LEN 8

class DebugRenderer { 

	public:
    
        bool draw_blobs                 = true;
        bool draw_points                = true;
        bool draw_ids                   = true;
        bool draw_outlines              = true;
        
        bool draw_mouse_blob            = false;
        int mouse_blob_id               = 0;
        long mouse_blob_created_time    = 0;
        
        ofRectangle mouse_blob_dim      = ofRectangle(0,0,80,80);
        bool extended_blob_info         = false; // show more info on the label.
        
        bool draw_regions               = true;
        		
		DebugRenderer(int _cvc_index);
		~DebugRenderer();
		
		void setup();
		bool getEnabled();
		void draw(int x = 0, int y = 0, float sx = 1.0, float sy = 1.0);
    
        int getSocketColor(int index);
        void setHexColor(int hex, int alpha);
    
        
        //------------------------------------------------------------
        
        void toggleBlobs()                      { showBlobs(!draw_blobs); }
        void showBlobs(bool state)              { draw_blobs = state; }
        
        void toggleIDs()                        { showIDs(!draw_ids); }
        void showIDs(bool state)                { draw_ids = state; }
        
        void toggleOutlines()                   { showOutlines(!draw_outlines); }
        void showOutlines(bool state)           { draw_outlines = state; }
        
        void togglePoints()                     { showPoints(!draw_points); }
        void showPoints(bool state)             { draw_points = state; }
        
        void toggleExtendedBlobInfo()           { showExtendedBlobInfo(!extended_blob_info); }
        void showExtendedBlobInfo(bool state)   { extended_blob_info = state; }
        
        void toggleRegions()                    { showRegions(!draw_regions); }
        void showRegions(bool state)            { draw_regions = state; }

		//------------------------------------------------------------
	
		void toggleMouseBlobs();
		void showMouseBlobs(bool state);
    
		void setMouseBlobDim(int w, int h);
		void setMouseBlobOffset(int x, int y);
		void makeMouseBlob();
        void setMouseBlobScaler(float x, float y);
	
		void mouseBlobMove(int m_x, int m_y);
		void mouseBlobPress();
		void destroyMouseBlob();
		
		// For ofMouse event listening
	    void mouseMoved(ofMouseEventArgs & args);
	    void mouseDragged(ofMouseEventArgs & args);
        void mousePressed(ofMouseEventArgs & args);
        void mouseReleased(ofMouseEventArgs & args);
	
	private:
        ofVec2f mouse_blob_scaler; // used to scale up or down mouse values on input
		ofVec2f mouse_blob_offset; // used to shift the blob based on a translated view
		int blob_colors[B_COLORS_LEN] = {0xFFFF0000, 0xFF00FFFF, 0xFF0000FF,0xFFFFFF00, 0xFFFF00FF, 0xFFFFAA00,0xFF9900FF, 0xFFCCCCCC}; // was 0x44 on all for alpha component
    
		int blobColor(int id = 0);
    
        int cvc_index;
    
        ofTrueTypeFont font;
    
};