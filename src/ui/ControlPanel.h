/*
	ControlPanel
	
*/

#pragma once

#include "ofMain.h"
#include "ofEvents.h"
#include "ofxCVC.h"
#include "ofxUI.h"
#include "SocketReader.h"
#include "SocketConnector.h"

class ControlPanel {

	public:
        
		ControlPanel(int _cvc_index);
	
		void setup();
    
        void update(ofEventArgs& args);
    
        void show();
        void hide();
        void toggle();
    
        void refresh(); // update widget values
    
        bool isVisible();
        void setVisible(bool state = true);
        void position(int x, int y);

		void addSocket(SocketReader *socket, int col = 0xCCFFCC);
    
        void setState(bool visible = true, bool open = true, bool blobs = true, bool points = true, bool outlines = false, bool ids = true, bool regions = false, int x = 0, int y = 0);
    
        int getX();
        int getY();
        int getWidth();
        int getHeight();
    
        bool isInitialised() { return _initialised == true; }
    
        ofxUICanvas *canvas;
    
		//------------------------------------------------------------
	
	private:
        int cvc_index;
    
        bool _initialised = false;
    
		vector<SocketConnector*> socket_connectors;
    
        void guiEvent(ofxUIEventArgs &e);
    
        ofxUIWidget* offsetWidget(ofxUIWidget * w, int off_x=0, int off_y=0);
    
        ofxUIToggle* addToggle(const string &id, bool state = false, int direction = OFX_UI_WIDGET_POSITION_DOWN, int offset_x = 0);
    
};