/*
	SocketConnector
	UI to show ip, port, connection status & button
*/

#pragma once

#include "ofMain.h"
#include "ofEvents.h"
#include "ofxCVC.h"
#include "ofxUI.h"
#include "SocketReader.h"

#define CVC_UI_SC_INDEX_LABEL "SC_INDEX"
#define CVC_UI_SC_HOST_LABEL "SC_HOST"
#define CVC_UI_SC_COLON_LABEL "SC_COLON"
#define CVC_UI_SC_PORT_LABEL "SC_PORT"
#define CVC_UI_SC_BUTTON "SC_BUTTON"
#define CVC_UI_SC_RECONNECT_TOGGLE "SC_RECONNECT_TOGGLE"

class SocketConnector: ofxUIWidget {

    typedef enum {DISCONNECTED, CONNECTING, CONNECTED} ConnectionState;
    
private:
	
	ofxUILabel*index_label;
	ofxUITextInput* host_label;
	ofxUILabel*colon_label;
	ofxUITextInput* port_label;
	
	ofxUILabelButton*  connect_btn;
    ofxUIToggle* reconnect_toggle;
	
	ConnectionState state = DISCONNECTED;

	SocketReader* socket;
	
	int color;
	
	int status_dot_color = 0xCCCCCC;
		
    bool draw_status_dot = true;
    
public:

	SocketConnector(SocketReader* _socket, int _color) {// , int x = 0, int y = 0, int w=0, int h=0) {
        socket = _socket;
		color = _color;
        
        paddedRect = new ofxUIRectangle();
        rect = new ofxUIRectangle();
    }
    
	void init(int x, int y, int w, int h) {
		
	    string indx_str = ofToString(socket->index);
        
        rect->set(x,y,w,h);
        name = string("SOCKET_CONNECTOR_"+indx_str);
        kind = OFX_UI_WIDGET_SOCKET_CONNECTOR;
        paddedRect->set(-padding, -padding, w+padding*2.0, h+padding);
        paddedRect->setParent(rect);
        
       // x = y = 0;
        
		index_label = (ofxUILabel*) pushWidget(new ofxUILabel(x+0, y+2, 10, CVC_UI_SC_INDEX_LABEL+indx_str, indx_str, OFX_UI_FONT_SMALL));
		index_label->setColorFill(ofColor::white);
        //  ofxUILabel(float x, float y, float w, string _name, string _label, int _size);
        
		host_label = (ofxUITextInput*) pushWidget(new ofxUITextInput(x+21, y+0, 80, CVC_UI_SC_HOST_LABEL+indx_str, socket->host, OFX_UI_FONT_SMALL));
                                                //  ofxUITextInput(float x, float y, float w, string _name, string _textstring, int _size, float h = 0);
        host_label->getLabelWidget()->setColorFill(ofColor::white);
        host_label->getLabelWidget()->setColorFillHighlight(ofColor::white);
        
		colon_label = (ofxUILabel*) pushWidget(new ofxUILabel(x+100, y+1, 10, CVC_UI_SC_COLON_LABEL+indx_str, ":", OFX_UI_FONT_SMALL));
		colon_label->setColorFill(ofColor::white);
        
        port_label = (ofxUITextInput*) pushWidget(new ofxUITextInput(x+105, y+0, 40, CVC_UI_SC_PORT_LABEL+indx_str, ofToString(socket->port), OFX_UI_FONT_SMALL));
        port_label->getLabelWidget()->setColorFill(ofColor::white);
        port_label->getLabelWidget()->setColorFillHighlight(ofColor::white);
        
        // TODO: if(!socket->.isInManualMode()) {
            connect_btn = (ofxUILabelButton*) pushWidget(new ofxUILabelButton(CVC_UI_SC_BUTTON+indx_str, false, 65, h, x+150, y+0, OFX_UI_FONT_SMALL));
            connect_btn->setLabelText("CONNECT");
            connect_btn->getLabelWidget()->setColorFill(ofColor::white);
            connect_btn->getLabelWidget()->setColorFillHighlight(ofColor::white);
        // }
        
        
        reconnect_toggle = (ofxUIToggle*) pushWidget(new ofxUIToggle(CVC_UI_SC_RECONNECT_TOGGLE+indx_str, &socket->auto_reconnect, 8, 8, x+220,y+2, OFX_UI_FONT_SMALL) );
        reconnect_toggle->setLabelVisible(false);
/*        reconnect_toggle->getLabelWidget()->setLabel("AUTO");
        reconnect_toggle->getLabelWidget()->setColorFill(ofColor::white);
        reconnect_toggle->getLabelWidget()->setColorFillHighlight(ofColor::white);
        */
        
        
        connectionChange(getStateFromSocket(socket));
        
		// TODO Could add listeners for when the server connects and disconnects? (instead am polling it in draw() for now)
        
	}
    
    ConnectionState getStateFromSocket(SocketReader* s) {
        if(socket->isConnected()) {
            return CONNECTED;
        }else if(socket->isConnecting()) {
            return CONNECTING;
        }else{ //assume disconnect
            return DISCONNECTED;
        }
    }
    
	void connectionChange(ConnectionState _state) {
		// draw a circle red or green based on if we are connected or not
		ofLogNotice("SocketConnector") << "connectionChange: " << _state;
        
        state = _state;
        
        if(state == CONNECTED) {
            status_dot_color = 0x00FF00;
            if(connect_btn) {
                connect_btn->setLabelText("DISCONNECT");
            }
            
        }else if(state == CONNECTING){
            status_dot_color = 0xFF8800;
            if(connect_btn) {
                connect_btn->setLabelText("CONNECT");
            }
            
        }else { // assume disconnected
            status_dot_color = 0xFF0000;
            if(connect_btn) {
                connect_btn->setLabelText("CONNECT");
            }
        }
        
        if(state != CONNECTING) draw_status_dot = true;
		   
	}
	    
    void buttonClicked() {
        
        if(state == DISCONNECTED || state == CONNECTING) {
            socket->connect(host_label->getTextString(), ofToInt(port_label->getTextString()));
        }else{
            socket->disconnect();
        }
        
    }
	
	void draw(){
		
        ConnectionState cur_state = getStateFromSocket(socket);
        if(cur_state != state) connectionChange(cur_state);
        
        // if(socket->isConnected() != showing_connected) connectionChange(socket->isConnected()); // hack to update state.
        
		//ofLogNotice("SocketConnector") << "draw";
		
	    ofxUIPushStyle();
    
	    ofEnableBlendMode(OF_BLENDMODE_ALPHA);
        
        if(state == CONNECTING) {
            if(ofGetFrameNum() % 15 == 0) draw_status_dot = !draw_status_dot;
        }
        
        if(draw_status_dot) {
            ofSetHexColor(status_dot_color);
            ofFill();
            ofxUISetRectMode(OF_RECTMODE_CENTER);
            ofxUICircle(rect->getX()+14, rect->getY()+5, 4);
        }
        
	    ofxUIPopStyle();
	}

	ofxUIWidget* pushWidget(ofxUIWidget* w) {
		//w->getRect()->setParent(this->getRect());

		//w->getRect()->y += rect->y;
		//w->getRect()->x += rect->x;
		//    w->setVisible(*value);
		//   w->setLabelVisible(*value);
		// toggles.push_back(ltoggle);

        parent->addWidget(w);
        w->setParent(this);
		//W->setModal(modal);
		return w;
	}	
	

};
