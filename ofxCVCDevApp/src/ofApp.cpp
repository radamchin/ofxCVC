#include "ofApp.h"

#define CVC_DEV_USE_THREAD false

#define TEST_MUTLIPLES 1 // test another CVC instance

//--------------------------------------------------------------
void ofApp::setup(){
    
    font.loadFont(OF_TTF_MONO, 7, false);
    
    // CVC(0)->OVERRIDE_SERVER_AUTO_CONNECT_CONFIG = true; // needs xml config loading to work
    // CVC(0)->IGNORE_DELETE_COMMANDS = true;
    // CVC(0)->BLOB_STAGNATION_KILL_AGE = 1000 * 2;
    
    CVC(0)->SMOOTH_UPDATES = true;
    // CVC(0)->UPDATE_INTERPOLATION = 0.05;    
    //CVC(0)->DATA_LOCKABLE = false;
    
    CVC(0)->THREAD_SOCKETS = false;
    
    CVC(0)->setup();
    
    //ofLogNotice(ofxCVC::instance()->getVersion());
    
    // manual setup a socket
    //CVC(0)->->addSocket("127.0.0.1", 11001, CVC_DEV_USE_THREAD);
    //CVC(0)->)->addSocket("127.0.0.1", 11002, CVC_DEV_USE_THREAD);
    
    CVC(0)->loadConfigFromFile("cvc_config.xml");
    
    // TODO: init socket from xml node.
    
    //CVC(0)->ebug_renderer->toggleMouseBlobs(true);
    CVC(0)->debug_renderer->setMouseBlobDim(60, 40);
    //ofxCVC::instance()->debug_renderer->setMouseBlobOffset(-25,-80); // makes the blob relative to the where the debug renderer is drawn

    if(TEST_MUTLIPLES) {
    
        CVC(1)->SMOOTH_UPDATES = true;
        CVC(1)->UPDATE_INTERPOLATION = 0.05;

        CVC(1)->setup();
        CVC(1)->loadConfigFromFile("cvc_config_2.xml");
    
        CVC(1)->debug_renderer->setMouseBlobDim(32, 32);
        CVC(1)->control_panel->position(5, ofGetHeight()-(ofxCVC::instance()->control_panel->getHeight()*2)-10);
    }
    
    CVC(0)->registerForChangeEvents(this);    
    //ofAddListener(CVC(0)->blobs_destroyed_event, this, &ofApp::cvcBlobsDestroyed);
    //ofAddListener(CVC(0)->blobs_update_event, this, &ofApp::cvcBlobsUpdated);
    
    // override position it at the bottom of screen
    CVC(0)->control_panel->position(5, ofGetHeight()-(ofxCVC::instance()->control_panel->getHeight())-5);
    
    disrupt.setup(0);
}

void ofApp::cvcBlobsDestroyed(ChangeEvent &e) {
   cout << "ofApp::cvcBlobsDestroyed count=" << e.destroyed_blobs->size() << endl;
    
}

void ofApp::cvcBlobsUpdated(ChangeEvent &e) {    
//    cout << "ofApp::cvcBlobsUpdated count=" << e.bloblist->total() << endl;
    
}

//--------------------------------------------------------------
void ofApp::update(){
    
    CVC(0)->update();

    if(TEST_MUTLIPLES) CVC(1)->update();

}

//--------------------------------------------------------------
void ofApp::draw(){
    ofBackground(16);
    
    // BlobList dev
    ostringstream out;
    for(int i = 0;i<16;i++){
        out << CVC(0)->blobs->blob_list.at(i)->toMinString() << "\n";
    }
    ofDrawBitmapStringHighlight(out.str(),ofPoint(10, 150), ofColor::black, ofColor::aquamarine);
    
    //-------------
    // Draw some statistics to monitor this shit yo.
    // ofSetColor(255,255,255);
    ofDrawBitmapStringHighlight(ofToString(ofGetFrameRate(),2) + " " + getUpTimeStr(), 5, 15);
    
    ofDrawBitmapStringHighlight(CVC(0)->toString(),5,30, ofColor::black, ofColor::forestGreen);
    ofDrawBitmapStringHighlight(CVC(0)->socketsToString(true), 5, 45, ofColor::black, ofColor::forestGreen);
    ofDrawBitmapStringHighlight(CVC(0)->blobs->toString(), 5, 60, ofColor::black, ofColor::forestGreen);
    
    ofDrawBitmapStringHighlight(disrupt.toString(), 5, 80, ofColor::black, ofColor::blueSteel);
    
    
    if(TEST_MUTLIPLES) {
        ofDrawBitmapStringHighlight(CVC(1)->toString(),5,100, ofColor::black, ofColor::blanchedAlmond);
        ofDrawBitmapStringHighlight(CVC(1)->socketsToString(true), 5, 115, ofColor::black, ofColor::blanchedAlmond);
        ofDrawBitmapStringHighlight(CVC(1)->blobs->toString(), 5, 130, ofColor::black, ofColor::blanchedAlmond);
    }

    //-------------
    
    CVC(0)->draw();

    if(TEST_MUTLIPLES) CVC(1)->draw();
    
    disrupt.draw();
      
}

string ofApp::getUpTimeStr(bool show_secs) { // Candidate for BBC UTIL bbcGetUpTimeStr()
    
    unsigned long total_up_secs = ofGetElapsedTimeMillis() / 1000;
    
    int dys = (int) floor(total_up_secs / 86400); // 86400 == 60 * 60 * 24, seconds in a day.
    int hrs = (int) floor((total_up_secs % 86400) / 3600); // 3600 == 60 * 60. secopnds in an hour
    int mins = (int) floor(((total_up_secs % 86400) % 3600) / 60);
    int secs = (total_up_secs % 86400) % 3600 % 60;
    
    stringstream out;
    out << "up:" << dys << "d," << hrs << "h," << mins << "m," << secs << "s";
    return out.str();    
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}