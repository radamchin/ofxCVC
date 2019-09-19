/*
	DebugRenderer
	Draws all blobs outlines points etc,
	
*/

#include "DebugRenderer.h"


DebugRenderer::DebugRenderer(int _cvc_index) {
    cvc_index = _cvc_index;
}

DebugRenderer::~DebugRenderer() {

}

void DebugRenderer::setup() {
    
    font.loadFont(OF_TTF_MONO, 9, false);
    
    mouse_blob_scaler = ofVec2f(1,1);
    mouse_blob_offset = ofVec2f(0,0);
    
    if(draw_mouse_blob) showMouseBlobs(true);

}

bool DebugRenderer::getEnabled() {		
	return draw_blobs || draw_points || draw_outlines || draw_ids || draw_regions;
}

void DebugRenderer::draw(int x, int y, float sx, float sy){
	
    if(getEnabled()) {
        
        ofEnableAlphaBlending();
        ofPushStyle();
        
        ofPushMatrix();
        ofTranslate(x,y);
        ofScale(sx, sy);
        
        int c = CVC(cvc_index)->getTotalSockets();
		SocketReader * s;
		ofRectangle r;
		
		// draw the outside super rect in great
        ofNoFill();
        ofSetLineWidth(1);
        ofSetColor(128,128,128,255);
        ofRect(CVC(cvc_index)->getDimensions());

		if(draw_regions) {
            // Render the outline of each socket
			
			for(int i = 0; i<c; i++){
				s = CVC(cvc_index)->getSocket(i);
				r = s->scaler;
				setHexColor(getSocketColor(i), 192);
				ofNoFill();
				ofRect(r);
				
				// Draw a diagonal line through it too so we can spot corners
				setHexColor(getSocketColor(i), 92);
				ofLine(r.x, r.y, r.getRight(), r.getBottom());
				ofLine(r.x, r.getBottom(), r.getRight(), r.y);
				
				string txt = "[" + ofToString(i) + "] " + s->toMinString();
				float tw = font.stringWidth(txt);                
				ofSetHexColor(0xFFFFFFFF);
                font.drawString(txt, round(r.x + (r.width*.5f) - (tw * .5f)), round(r.y+12));
			}
			
		}
		
		if(CVC(cvc_index)->blobs->hasBlobs()) { // render / rect to the blobs in blobs
            
            CVC(cvc_index)->blobs->lock();
                        
			bool outlines_drawn;
            
            ofVec2f pos;
            
            Blob* b = CVC(cvc_index)->blobs->blob_list.first();
            
            while(b) {
	    		pos = b->getPos();
               
				ofSetLineWidth(1);
                
				if(draw_blobs) {
                    ofNoFill();
                    ofSetColor(255);
					ofRect(b->getRect());
                    ofFill();
					setHexColor(blobColor(b->getID()), 48);
					ofRect(b->getRect());
				}

				// if(draw_outlines) outlines_drawn = BlobDrawing.drawPath(g, b.getOutlines(), 0xFFFFCC00, .5f);
			
				if(draw_points) { // Draw centre cross
                    ofSetColor(255);
                    ofNoFill();
					ofLine(pos.x,       pos.y-15,   pos.x,      pos.y+15);
					ofLine(pos.x-15,    pos.y,      pos.x+15,   pos.y);
				}
			
				// Draw id label
				if(draw_ids || extended_blob_info) {
                    ostringstream out;
					out << b->getID();
				
					if(extended_blob_info) out << ", pos=" << pos <<
															"\nrect=" << b->getRect() <<
                                                            "\nd=" << b->getDecPos() << ", outlines=" << b->getOutlinesCount() << ", area=" << b->getArea() << 
															"\n" << "age=" << b->getAge() << ", stag=" << b->getStagnation()  <<  ", xs=" << b->getXSpeed() << ", ys=" << b->getYSpeed() << endl << "state=" << b->getStateString() << ", ma=" << b->getMotionAccel() << ",ms=" << b->getMotionSpeed();
					
                    float tw = font.stringWidth(out.str());
					// Draw a white box behind it.
					int line_count = ofStringTimesInString(out.str(), "\n");
					int th = 12*(line_count + (line_count > 1 ? 1.5 : 1)); // height is approx. font height by number of lines in string
                    
                    ofFill();
					ofSetColor(0, 192);
					ofRect(pos.x+8, pos.y+5, tw+8, th+3);
					
					ofSetColor(255);
					font.drawString(out.str(), round(pos.x+10), round(pos.y+17));
				}
                
                b = CVC(cvc_index)->blobs->blob_list.next(b->getIndex());
				
			}
			
            CVC(cvc_index)->blobs->unlock();
            
			/*if(ofxCVC::JOIN_INTERSECTED){
				int ic = CVC(cvc_index)->blobs->intersected_rects.size();
				ofRectangle * ir;
				
				if(ic > 0) { // draw the intersected rects
					ofSetHexColor(0xFF00FF00);
					ofSetLineWidth(1f);
					ofNoFill();
					for(int i = 0; i<ic; i++) {
						ir = CVC(cvc_index)->blobs->intersected_rects.get(i);
						//ofRect(ir);
						ofLine(ir.x, ir.y, ir.x+ir.width, ir.y+ir.height);
						ofLine( ir.x+ir.width, ir.y, ir.x, ir.y+ir.height);
					}
				}
				
				ic = CVC(cvc_index)->blobs->intersected_rects2.size();
				if(ic > 0) { // draw the intersected rects
					ofSetHexColor(0xFF00FFFF);
					ofSetLineWidth(1f);
					ofNoFill();
					for(int i = 0; i<ic; i++) {
						ir = CVC(cvc_index)->blobs->intersected_rects2.get(i);
						ofRect(ir);
						//ofLine(ir.x, ir.y, ir.x+ir.width, ir.y+ir.height);
						//ofLine( ir.x+ir.width, ir.y, ir.x, ir.y+ir.height);
					}
				}
			}*/
			
		}
        
        ofPopStyle();
        ofDisableBlendMode();
		ofPopMatrix();
		
	}
	
}

//------------------------------------------------------------
	
int DebugRenderer::getSocketColor(int index) {
	if(index >= B_COLORS_LEN) index = index % B_COLORS_LEN;
	
	return blob_colors[index];
}

int DebugRenderer::blobColor(int id) {
	// based on its id return a unique color
	return getSocketColor((int)floor(id / SocketReader::BLOB_ID_OVERLAP_BUFFER));
}
void DebugRenderer::setHexColor(int hex, int alpha) {
    // break down hex color into rgb and send on
    int r = (hex >> 16) & 0xff;
	int g = (hex >> 8) & 0xff;
	int b = (hex >> 0) & 0xff;
    ofSetColor(r,g,b,alpha);
}

//------------------------------------------------------------

void DebugRenderer::toggleMouseBlobs() {
    showMouseBlobs(!draw_mouse_blob);
}

void DebugRenderer::showMouseBlobs(bool state) {
	
	ofLogNotice("cvc:showMouseBlobs") << state;
	
	if(state) {
		mouse_blob_id = INT_MAX; // Using Max int so it will play nice with other things that parse it (namely mpe -> mmr protocol)
		makeMouseBlob();
		ofRegisterMouseEvents(this);
	}else{
		destroyMouseBlob();
		ofUnregisterMouseEvents(this);		
	}

	draw_mouse_blob = state;
}

void DebugRenderer::setMouseBlobDim(int w, int h) {
	mouse_blob_dim.width = w;
	mouse_blob_dim.height = h;
}

void DebugRenderer::setMouseBlobOffset(int x, int y) {
	mouse_blob_offset.set(x,y);
	ofLogNotice("setMouseBlobOffset") << x << "," << y << " -> " << mouse_blob_offset;
}
void DebugRenderer::setMouseBlobScaler(float x, float y){
    mouse_blob_scaler.set(x,y);
    ofLogNotice("setMouseBlobOffset") << mouse_blob_scaler;
}

void DebugRenderer::makeMouseBlob() {
	mouse_blob_id--; // decrement so we dont blow the max integer (it will get reset when toggled!), collision with real blobs is more plausible but highly unlikely, if it does then real blobs will suffer same mpe, parse fate too.
	
	mouse_blob_created_time = ofGetElapsedTimeMillis();
	
	float m_x = ofGetMouseX() + mouse_blob_offset.x;
	float m_y = ofGetMouseY() + mouse_blob_offset.y;
	
	ofLogNotice("makeMouseBlob") << mouse_blob_id << " [" << m_x << "," << m_y << "]";
	
	ofVec2f mousePos = ofVec2f(m_x, m_y);
	ofVec2f topLeft = ofVec2f(m_x-(mouse_blob_dim.width*.5f), m_y-(mouse_blob_dim.height*.5f));
	
	//float outlines[] = BlobDrawing.makeDiamond(mouse_blob_dim);
	//BlobDrawing.offset(outlines, topLeft);
	CVC(cvc_index)->blobs->createBlob(mouse_blob_id, mousePos.x, mousePos.y, topLeft.x, topLeft.y, mouse_blob_dim.width, mouse_blob_dim.height, ofNormalize(mousePos.x, 0, ofGetWidth()), ofNormalize(mousePos.y, 0, ofGetHeight()));//, outlines);
}

void DebugRenderer::destroyMouseBlob() {
	CVC(cvc_index)->blobs->destroyBlob(mouse_blob_id, BlobManager::OTHER);
}

void DebugRenderer::mouseMoved(ofMouseEventArgs & args){
	mouseBlobMove(args.x, args.y);
}
void DebugRenderer::mouseDragged(ofMouseEventArgs & args){
	mouseBlobMove(args.x, args.y);
}
void DebugRenderer::mousePressed(ofMouseEventArgs & args){
	mouseBlobPress();
}

void DebugRenderer::mouseReleased(ofMouseEventArgs & args){
}

void DebugRenderer::mouseScrolled(ofMouseEventArgs & args){
}

void DebugRenderer::mouseEntered(ofMouseEventArgs & args){
}

void DebugRenderer::mouseExited(ofMouseEventArgs & args){
}

//-------------------------------------------
void DebugRenderer::mouseBlobPress() {
	
	if(ofGetElapsedTimeMillis() < mouse_blob_created_time + 100) return; // limit quick recreation issue due to event listening
	
	// add a new blob where the mouse is (and kill the old one)
	
	destroyMouseBlob();
	makeMouseBlob();
}

void DebugRenderer::mouseBlobMove(int m_x, int m_y) {
	//Logger.print("mouseBlobMove:" + m_x +"," + m_y);
	
	m_x += mouse_blob_offset.x;
	m_y += mouse_blob_offset.y;
    
    m_x *= mouse_blob_scaler.x;
    m_y *= mouse_blob_scaler.y;
	
	ofVec2f mousePos = ofVec2f(m_x, m_y);			
	ofVec2f topLeft = ofVec2f(m_x-(mouse_blob_dim.width*.5f), m_y-(mouse_blob_dim.height*.5f));
	
	//float outines[] = BlobDrawing.makeDiamond(mouse_blob_dim);
	//BlobDrawing.offset(outines, topLeft);
	
	CVC(cvc_index)->blobs->updateBlob(mouse_blob_id, mousePos.x, mousePos.y, topLeft.x, topLeft.y, mouse_blob_dim.width, mouse_blob_dim.height, ofNormalize(mousePos.x, 0, ofGetWidth()), ofNormalize(mousePos.y, 0, ofGetHeight()));//, outlines);
}
		
