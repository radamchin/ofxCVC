/*
	ControlPanel
	Draws all blobs outlines points etc.	
*/

#include "ControlPanel.h"

#define CVC_GET_WIDG(_type, _name) ((_type *)canvas->getWidget(_name))
#define CVC_WIDG_SET(_type, _name, _var) CVC_GET_WIDG(_type, _name)->setValue(_var)

#define CVC_UI_VAL(_type) ((_type *)e.widget)->getValue()
#define CVC_UI_BOOL_VAL CVC_UI_VAL(ofxUIToggle)

#define CP_TITLE "TITLE"

#define CP_SHOW_BLOBS "BLOBS"
#define CP_SHOW_POINTS "POINTS"
#define CP_SHOW_IDS "IDS"
#define CP_SHOW_OUTLINES "OUTLINES"
#define CP_SHOW_MOUSE "MOUSE"
#define CP_SHOW_EX_INFO "EX INFO"

#define CP_SHOW_REGIONS "REGIONS"

#define CP_TOGGLE_JOIN "JOIN"
#define CP_TOGGLE_QUEUE "QUEUE"
#define CP_TOGGLE_SWEEP "SWEEP"
#define CP_TOGGLE_SMOOTH "SMOOTH"

#define CP_INFO_LABEL "INFO_LABEL"


ControlPanel::ControlPanel(int _cvc_index) {
    cvc_index = _cvc_index;
    ofLogNotice("new ofxCVC::ControlPanel") << cvc_index;
}

void ControlPanel::setup() {
    
    int W = 260;
	canvas = new ofxUICanvas(); //"", OFX_UI_FONT_SMALL);//"CVC:", OFX_UI_FONT_SMALL);
    
    canvas->setDrawBack(false);
    canvas->setDrawPadding(true);
    canvas->setColorPadded(ofColor(128,128,128,160));
    canvas->setColorBack(ofColor(0, 51, 75,255));
    canvas->setColorOutlineHighlight(ofColor(0,255,255,255));
    canvas->setColorFill(ofColor(0, 181*.9, 229*.9));
    canvas->setColorFillHighlight(ofColor(0, 181*1.1, 229*1.1));
    
    canvas->setWidgetSpacing(5);
    
    canvas->setGlobalSpacerHeight(1);
    
  	//canvas->setFont(OF_TTF_MONO, false);
    canvas->setFont("fonts/pf_ronda_seven.ttf", false);
    canvas->setFontSize(OFX_UI_FONT_LARGE, 4); //5
    canvas->setFontSize(OFX_UI_FONT_MEDIUM, 4); //5
    canvas->setFontSize(OFX_UI_FONT_SMALL, 4); //5
    
    //----------------------------------------------------
    
    // add widget
    
    //canvas->addWidgetToHeader(new ofxUILabel(CP_TITLE, "CVC TITLE", OFX_UI_FONT_SMALL));
    canvas->addWidgetDown(new ofxUILabel(CP_TITLE, "CVC TITLE", OFX_UI_FONT_SMALL));
    
    addToggle(CP_SHOW_BLOBS);
    addToggle(CP_SHOW_POINTS, false, OFX_UI_WIDGET_POSITION_RIGHT, 0);
    addToggle(CP_SHOW_IDS, false, OFX_UI_WIDGET_POSITION_RIGHT, 0);
    addToggle(CP_SHOW_OUTLINES, false, OFX_UI_WIDGET_POSITION_RIGHT, 0);
    
    addToggle(CP_SHOW_MOUSE, false, OFX_UI_WIDGET_POSITION_DOWN, 0);
    addToggle(CP_SHOW_EX_INFO, false, OFX_UI_WIDGET_POSITION_RIGHT, 0);
    addToggle(CP_SHOW_REGIONS, false, OFX_UI_WIDGET_POSITION_RIGHT, 0);
    
    canvas->addWidgetDown(new ofxUILabel(CP_INFO_LABEL, "info label yo", OFX_UI_FONT_SMALL));
    
    // Join and Queue not yet implementd so removing for now..
   // addToggle(CP_TOGGLE_JOIN);
    //addToggle(CP_TOGGLE_QUEUE, false, OFX_UI_WIDGET_POSITION_RIGHT);
    addToggle(CP_TOGGLE_SWEEP);//, false, OFX_UI_WIDGET_POSITION_RIGHT);
    addToggle(CP_TOGGLE_SMOOTH, false, OFX_UI_WIDGET_POSITION_RIGHT);
    
    canvas->addSpacer();
    
    //----------------------------------------------------
    // loop all widgets and if they have labels, set them white!
    
    vector<ofxUIWidget*> all_widgets = canvas->getWidgets();
    for(int i=0;i<all_widgets.size();i++) {
        if(all_widgets[i]->hasLabel()) {
            ((ofxUIWidgetWithLabel *)all_widgets[i])->getLabelWidget()->setColorFill(ofColor::white);
            ((ofxUIWidgetWithLabel *)all_widgets[i])->getLabelWidget()->setColorFillHighlight(ofColor::white);
        }else if(dynamic_cast<ofxUILabel*>(all_widgets[i])){ // its a label
            ((ofxUILabel *)all_widgets[i])->setColorFill(ofColor::white);            
        }
    }
    
    ofAddListener(canvas->newGUIEvent, this, &ControlPanel::guiEvent);
    
    canvas->setWidth(W);
    canvas->autoSizeToFitWidgets();
    
    _initialised = true;
    
    show();
}

//------------------------------------------------------------

void ControlPanel::addSocket(SocketReader* socket, int col) {

	int n = socket_connectors.size();
	int line_height = 12;

	SocketConnector* socket_conn = new SocketConnector(socket, col);
	canvas->addWidget((ofxUIWidget*)socket_conn);
    float ty = canvas->getRect()->getHeight() + 5;
    socket_conn->init(5, ty, 80, line_height); // (ty + (n * line_height))
	
	ofLogNotice("ControlPanel") << "addSocket:" << n << " already added";

	socket_connectors.push_back(socket_conn);
    
    canvas->autoSizeToFitWidgets();
}

ofxUIToggle* ControlPanel::addToggle(const string &id, bool state, int direction, int offset_x) {

    ofxUIToggle* t = new ofxUIToggle(id, state, 10, 10);
    
    if(direction == OFX_UI_WIDGET_POSITION_RIGHT) {
        canvas->addWidgetRight(t);
    }else{ // default to adding down.
        canvas->addWidgetDown(t);
    }
    
    offsetWidget(t, offset_x, 0);    
    return t;
}

ofxUIWidget* ControlPanel::offsetWidget(ofxUIWidget* w, int off_x, int off_y) {
    ofxUIRectangle* r = w->getRect();
    r->setX(r->getX()+off_x);
    r->setY(r->getY()+off_y);    
    // getPaddingRect
    return w;
}

void ControlPanel::setState(bool visible, bool open, bool blobs, bool points, bool outlines, bool ids, bool regions, int x, int y){
    // toggle all check boxes from an external call (set up the defaults)
    ofLogNotice("ControlPanel") << "setState: visible=" << visible << ", open=" << open << ",blobs=" << blobs << ",points="<<points<<",outlines="<<outlines <<",ids=" << ids << ",regions=" << regions <<",x=" << x << ",y=" << y;
    
    // Ensure the debug renderer is matched
    CVC(cvc_index)->debug_renderer->draw_blobs      = blobs;
    CVC(cvc_index)->debug_renderer->draw_points     = points;
    CVC(cvc_index)->debug_renderer->draw_ids        = ids;
    CVC(cvc_index)->debug_renderer->draw_outlines   = outlines;
    CVC(cvc_index)->debug_renderer->draw_regions    = regions;
        
    position(x,y);
    
    if(visible){
        show();
    }else{
        hide();
    }
    
    if(open) { // this is the minimise/maximise thing
        // TODO
    }else{
        // TODO:
    }
    
}

//------------------------------------------------------------

void ControlPanel::update(ofEventArgs& args) {
    
   CVC_GET_WIDG(ofxUILabel, CP_TITLE)->setLabel( "CVC["+ ofToString(cvc_index) + "]: SOCKETS=" + CVC(cvc_index)->getSocketsMiniState() + ",  Blobs=" + ofToString(CVC(cvc_index)->blobs->getCount()));
        
}

void ControlPanel::position(int x, int y) {
    
    canvas->setPosition(x, y);
}

//------------------------------------------------------------

int ControlPanel::getX() {
    return canvas->getRect()->getX();
}

int ControlPanel::getY(){
    return canvas->getRect()->getY();
}

int ControlPanel::getWidth(){
    return canvas->getRect()->getWidth();
}

int ControlPanel::getHeight(){
    return canvas->getRect()->getHeight();
}

//------------------------------------------------------------

void ControlPanel::setVisible(bool state) {
    canvas->setVisible(state);
}

bool ControlPanel::isVisible() {
    return canvas->isEnabled();
}

void ControlPanel::refresh() {
    
    if(!isVisible()) return;
    
    CVC_WIDG_SET(ofxUIToggle,   CP_SHOW_BLOBS,      CVC(cvc_index)->debug_renderer->draw_blobs);
    CVC_WIDG_SET(ofxUIToggle,   CP_SHOW_POINTS,     CVC(cvc_index)->debug_renderer->draw_points);
    CVC_WIDG_SET(ofxUIToggle,   CP_SHOW_IDS,        CVC(cvc_index)->debug_renderer->draw_ids);
    CVC_WIDG_SET(ofxUIToggle,   CP_SHOW_OUTLINES,   CVC(cvc_index)->debug_renderer->draw_outlines);
    CVC_WIDG_SET(ofxUIToggle,   CP_SHOW_MOUSE,      CVC(cvc_index)->debug_renderer->draw_mouse_blob);
    CVC_WIDG_SET(ofxUIToggle,   CP_SHOW_EX_INFO,    CVC(cvc_index)->debug_renderer->extended_blob_info);
    CVC_WIDG_SET(ofxUIToggle,   CP_SHOW_REGIONS,    CVC(cvc_index)->debug_renderer->draw_regions);
    //CVC_WIDG_SET(ofxUIToggle,   CP_TOGGLE_JOIN,     CVC(cvc_index)->JOIN_INTERSECTED);
    //CVC_WIDG_SET(ofxUIToggle,   CP_TOGGLE_QUEUE,    CVC(cvc_index)->USE_QUEUED_UPDATES);
    CVC_WIDG_SET(ofxUIToggle,   CP_TOGGLE_SWEEP,    CVC(cvc_index)->SWEEP_FOR_STAGNANTS);
    CVC_WIDG_SET(ofxUIToggle,   CP_TOGGLE_SMOOTH,   CVC(cvc_index)->SMOOTH_UPDATES);
 
    CVC_GET_WIDG(ofxUIToggle, CP_TOGGLE_SMOOTH)->getLabel()->setLabel("SMOOTH(" + ofToString(CVC(cvc_index)->UPDATE_INTERPOLATION) + ")");
    
    ostringstream out;
    out << "overlap=" << CVC(cvc_index)->OVERLAP_ENABLED <<  ", v" << CVC(cvc_index)->getVersion() << ", lockable=" << CVC(cvc_index)->DATA_LOCKABLE;
    // TODO: custom util to dump a ofRectangle to x,y,w,h ", dims=" << CVC(cvc_index)->getDimensions() <<
    
    CVC_GET_WIDG(ofxUILabel, CP_INFO_LABEL)->setLabel(out.str()); // "yo yo"); // "
    
}

// TODO: minimise and maximise from ofUISuperCanvas
/*
 void setMinified(bool _bIsMinified);
 bool isMinified();
 void toggleMinified();
 */

void ControlPanel::toggle() {
    
    ofLogNotice("ofxCVC:ControlPanel") << "UI toggle -> " << (!isVisible());
    
    if(isVisible()) {
        hide();
    }else{
        show();
    }
    
}

void ControlPanel::hide() {
    canvas->setVisible(false);
    ofRemoveListener(ofEvents().update, this, &ControlPanel::update);
    
}

void ControlPanel::show() {    
    refresh(); // bout to be come visible again so update it
    ofAddListener(ofEvents().update, this, &ControlPanel::update);
    canvas->setVisible(true);
}

void ControlPanel::guiEvent(ofxUIEventArgs &e) {
    
    string name = e.widget->getName();
    int kind = e.widget->getKind();
   // ofLogNotice("ofxCVC::ControlPanel::guiEvent") << name << "," << kind;
    
    if(e.widget->getParent()) {
        if(e.widget->getParent()->getKind() == OFX_UI_WIDGET_SOCKET_CONNECTOR) {
          //  ofLogNotice("It's a socket connector");
            if(kind == OFX_UI_WIDGET_LABELBUTTON) {
                if(e.getButton()->getValue() == 1){ // 0 == MOUSE_UP 1 == MOUSE_DOWN
                  ((SocketConnector*)e.widget->getParent())->buttonClicked();
                }
            }
            return;
        }
    }
    
    if(name ==       CP_SHOW_BLOBS){
        CVC(cvc_index)->debug_renderer->showBlobs(CVC_UI_BOOL_VAL);
        
    }else if(name == CP_SHOW_POINTS){
        CVC(cvc_index)->debug_renderer->showPoints(CVC_UI_BOOL_VAL);
    
    }else if(name == CP_SHOW_IDS){
        CVC(cvc_index)->debug_renderer->showIDs(CVC_UI_BOOL_VAL);
    
    }else if(name == CP_SHOW_OUTLINES){
        CVC(cvc_index)->debug_renderer->showOutlines(CVC_UI_BOOL_VAL);
    
    }else if(name == CP_SHOW_MOUSE){
        CVC(cvc_index)->debug_renderer->showMouseBlobs(CVC_UI_BOOL_VAL);
    
    }else if(name == CP_SHOW_EX_INFO){
        CVC(cvc_index)->debug_renderer->showExtendedBlobInfo(CVC_UI_BOOL_VAL);
        
    }else if(name == CP_SHOW_REGIONS){
        CVC(cvc_index)->debug_renderer->showRegions(CVC_UI_BOOL_VAL);
        
    }else if(name == CP_TOGGLE_JOIN){
        CVC(cvc_index)->JOIN_INTERSECTED = CVC_UI_BOOL_VAL;
        
    }else if(name == CP_TOGGLE_QUEUE){
        CVC(cvc_index)->USE_QUEUED_UPDATES = CVC_UI_BOOL_VAL;
        
    }else if(name == CP_TOGGLE_SWEEP){
        CVC(cvc_index)->SWEEP_FOR_STAGNANTS = CVC_UI_BOOL_VAL;
        
    }else if(name == CP_TOGGLE_SMOOTH){
        CVC(cvc_index)->SMOOTH_UPDATES = CVC_UI_BOOL_VAL;
        
    }
      
}