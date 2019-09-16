
#include "ofxCVC.h"

//--------------------------------------------------------------

ofxCVC::ofxCVC(int _index){
    index = _index;
    ofLogNotice("ofxCVC initialized v") << getVersion() << "[" << index << "]";
    ofAddListener(ofEvents().exit, this, &ofxCVC::stop);
}

string& ofxCVC::getVersion() {
    static string version_str;
    if(version_str.size() == 0) version_str = ofToString(CVC_MAJOR) + "." + ofToString(CVC_MINOR) + "." + ofToString(CVC_PATCH);
    return version_str;
}

void ofxCVC::setup() {
	
    blobs = new BlobManager(index);
    
    debug_renderer = new DebugRenderer(index);    
	debug_renderer->setup();
    
    control_panel = new ControlPanel(index);
    control_panel->setup();
		
	setDimensions(0, 0, ofGetWidth(), ofGetHeight());
    
    _initialised = true;
}

bool ofxCVC::isInitialised() {
    return _initialised;
}

void ofxCVC::update() {
	    
    // if(JOIN_INTERSECTED) blobs.joinIntersected();
    
    if(SWEEP_FOR_STAGNANTS) blobs->sweepForStagnants();

    if(blobs->hasDestroyedBlobIds()) {
        // dispatch an event for remove (instead of the hard coded visualisation class)
        static ChangeEvent dest_event;
        blobs->lock();
        dest_event.destroyed_blobs = blobs->getDestroyedBlobIds();
        ofNotifyEvent(blobs_destroyed_event, dest_event, this); 
        //ofNotifyEvent(ChangeEvent::blobs_destroyed, dest_event);
        blobs->unlock();
    }

    // if(USE_QUEUED_UPDATES) blobs->applyQueuedUpdates();

    if(blobs->hasBlobs()) {
        // dispatch event here. sending a pointer to the blobs
        static ChangeEvent upd_event;
        blobs->lock();
        //upd_event.blob_state = blobs->getBlobs();
        upd_event.bloblist = &blobs->blob_list;
        ofNotifyEvent(blobs_update_event, upd_event, this); 
        //ofNotifyEvent(ChangeEvent::blobs_update, upd_event);
        blobs->unlock();
    }

    if(AUTO_CLEAR_DESTROYED_CACHE) blobs->flushDestroyedCache();
        
}

void ofxCVC::draw(float x, float y, float sx, float sy) {
	debug_renderer->draw(x, y, sx, sy);
}

void ofxCVC::resize() {
	// force a resize on all socket readers
	setDimensions(dimensions.x, dimensions.y, dimensions.width, dimensions.height);
}

void ofxCVC::setDimensions(float x, float y, float w, float h) {
    ofLogNotice("ofxCVC::setDimensions") << x << "," << y << "," << w << "," << h;
    
	dimensions.set(x,y,w,h);
	    
    // if(MUTLI_SOCKETS_ENABLED) walk resize the sockets proportionaltely to the screen space, lay them out you.  See resize() from Java:SocketServer.
    
    for(int i=0; i<sockets.size(); i++) {
        sockets[i]->resize(x, y, w, h);
    }
	
}

void ofxCVC::addSocket(const string & ip, int port, bool _run_as_thread, bool auto_connect, bool auto_reconnect, float reconnect_interval_secs, const string & offset_rect_str) {
	ofLogNotice("addSocket") << ip << "," << port << ","  << _run_as_thread << "," << auto_connect << "," << auto_reconnect << "," << reconnect_interval_secs  <<  ",'"  <<  offset_rect_str  << "'";
	
	SocketReader *reader = new SocketReader;
	reader->setup(index, sockets.size(), ip, port, _run_as_thread, auto_connect, auto_reconnect, reconnect_interval_secs);
	sockets.push_back(reader);
	
	control_panel->addSocket(reader);
    
    resize();
}

//-------------------------------------------------------

SocketReader * ofxCVC::getSocket(int n){
	return sockets[n];
}
	
int ofxCVC::getTotalSockets(){
	return sockets.size();
}

/*vector<int> ofxCVC::getManualModeSocketIndexs() {
	// Look for a server that has manual mode set and return its index.
	vector<int> out;
	for(int i=0; i<getTotalSockets(); i++) {
		if(getSocket(i)->isInManualMode()) out.push_back(i);
	}
	
	return out;
}*/

void ofxCVC::setSocketScaleDims(float x, float y, float w, float h) {
	ofLogNotice("ofxCVC:setSocketScaleDims") << "[" << x <<", " << y << ", " << w << ", " << h <<"]";
	
	for(int i=0; i<sockets.size(); i++) sockets[i]->resize(x, y, w, h);
	
}

ofRectangle & ofxCVC::getDimensions() {
	return dimensions;
}

/*void ofxCVC::setDynamicDimensions(bool b){
	_dynamic_dimensions = b;
}*/

//------------------------------------------------------------

bool ofxCVC::loadConfigFromString(const string& xml_str) {    
    bool loaded = xml_settings.loadFromBuffer(xml_str);
    if(!loaded) return false;
    
    return parseConfig();
}

bool ofxCVC::loadConfigFromFile(const string& xml_file_uri) {
    bool loaded = xml_settings.loadFile(xml_file_uri);
    if(!loaded) return false;
    
    return parseConfig();
}

/*bool ofxCVC::loadConfigFromXML(const ofXML& xml) {
    xml_settings.
    return true;
}*/

bool ofxCVC::parseConfig() {
    
    // walk xml_settings sending into appropriate vars and calls.
        
    JOIN_INTERSECTED = xml_settings.getAttribute("vision:joining", "enabled", JOIN_INTERSECTED);
   
    SMOOTH_UPDATES = xml_settings.getAttribute("vision:smoothing", "enabled", SMOOTH_UPDATES);
    UPDATE_INTERPOLATION = xml_settings.getAttribute("vision:smoothing", "amount", UPDATE_INTERPOLATION);
    
    // TODO: USE_QUEUED_UPDATES = "vision:queue_updates", "enabled", 
    
    SWEEP_FOR_STAGNANTS = xml_settings.getAttribute("vision:sweep", "enabled", SWEEP_FOR_STAGNANTS);
    BLOB_STAGNATION_KILL_AGE = xml_settings.getAttribute("vision:sweep", "age", BLOB_STAGNATION_KILL_AGE);
    
    bool sockets_parsed = parseSockets();
    
    control_panel->setState(xml_settings.getAttribute("vision:ui", "visible",   true),
                            xml_settings.getAttribute("vision:ui", "open",      true),
                            xml_settings.getAttribute("vision:ui", "blobs",     true),
                            xml_settings.getAttribute("vision:ui", "points",    true),
                            xml_settings.getAttribute("vision:ui", "outlines",  false),
                            xml_settings.getAttribute("vision:ui", "ids",       true),
                            xml_settings.getAttribute("vision:ui", "regions",   true),
                            xml_settings.getAttribute("vision:ui", "x",         0),
                            xml_settings.getAttribute("vision:ui", "y",         0)
                         );
        
    return sockets_parsed;
}

bool ofxCVC::parseSockets() {
    
    TiXmlElement * xml = xml_settings.getNode("vision:sockets");
    
    if(!xml) {
        ofLogError("ofxCVC") << " config xml is missing a <sockets /> node. I can't help you with out it champ.";
        return false;
    }
    
    MUTLI_SOCKETS_ENABLED = xml_settings.getAttribute("vision:sockets", "use_multiple", MUTLI_SOCKETS_ENABLED);
    
    if(MUTLI_SOCKETS_ENABLED) {
        OVERLAP_ENABLED = xml_settings.getAttribute("vision:sockets", "use_overlap", OVERLAP_ENABLED);
        if(OVERLAP_ENABLED) {
            string overlap_dists = xml_settings.getAttribute("vision:sockets", "overlap_dist", "0,0");
            // TODO: parse this into accessable array
            // DataUtil.vectorFromString(servers_node.getString("overlap_dist", "0,0"), SocketServer.overlap_frac);
            // ofLogNotice("multiple_servers.overlap = true, distance=" + SocketServer.overlap_frac);

        }
    }
    
    vector<ofxXmlSettings*> socket_nodes;
    int socket_count = 0;
     
     if(xml) {
         for(const TiXmlNode* row_node=xml->FirstChild(); row_node; row_node=row_node->NextSibling()){
             int cur_row_width = 0;
             
             if(xml_settings.validNode(row_node, "row")) {
                 col_height++;
                 for(const TiXmlNode* socket_node=row_node->FirstChild(); socket_node; socket_node=socket_node->NextSibling()){
                     if(xml_settings.validNode(socket_node, "socket")){ // It's a socket!
                         socket_count++;
                         cur_row_width++;
                         // Convert this socket node into its own doc, for access below
                         ofxXmlSettings * soc_xml = new ofxXmlSettings;
                         soc_xml->loadFromBuffer( xml_settings.makeString(socket_node) );
                         socket_nodes.push_back(soc_xml);
                     }
                 }
             }
             
             if(cur_row_width > row_width) row_width = cur_row_width;
             
         }
     }
    
     ofLogNotice("ofxCVC") << " sockets=" << socket_count << ", rows=" << row_width << " cols=" << col_height;
    
    if(socket_count == 0) {
        ofLogError("ofxCVC") << "Could find no <socket /> nodes in config.xml";
        return false;
    }
    
    // Create the sockets
    
    if(!MUTLI_SOCKETS_ENABLED) {
        // limit to 1 only
        if(socket_count > 1) {
            ofLogWarning("ofxCVC") << "config limiting to 1 socket as use_multiple='0'";
            socket_count = row_width = col_height = 1;
        }
    }
    
    for(int k = 0; k<socket_count;k++) {
        //cout << "socket:" << k << " = " << xml_settings.makeString(socket_nodes[k]) << endl;

        addSocket(socket_nodes[k]->getAttribute("socket", "ip", "127.0.0.1"),
                  socket_nodes[k]->getAttribute("socket", "port", 11000),
                  THREAD_SOCKETS,
                  socket_nodes[k]->getAttribute("socket", "connect", false) && !OVERRIDE_SERVER_AUTO_CONNECT_CONFIG,
                  socket_nodes[k]->getAttribute("socket", "reconnect", false) && !OVERRIDE_SERVER_AUTO_CONNECT_CONFIG,
                  socket_nodes[k]->getAttribute("socket", "reconnect_interval", SocketReader::DEFAULT_RECONNECT_INTERVAL_SECS),
                  socket_nodes[k]->getAttribute("socket", "offset", "0,0,0,0")
                  );
    }
    
    resize(); // call resize again so if multi-serving then any servers will reflow correctly due to the servers array being filled and its length effecting distribution
    
    control_panel->refresh();
    
    return true;
}

//----------------------------------------------------

void ofxCVC::toggleDebugRenderer(){
	showDebugRenderer(!debug_renderer->getEnabled());
}

void ofxCVC::showDebugRenderer(bool state) {
	debug_renderer->showBlobs(state);
	debug_renderer->showPoints(state);
	debug_renderer->showOutlines(state);		
}

//----------------------------------------------------
	
int ofxCVC::getRowCount() {
	return row_width;
}

int ofxCVC::getColumnCount() {
	return col_height;
}

//----------------------------------------------------

void ofxCVC::stop(ofEventArgs & e) {
	// cleanup safely "kill"
    ofRemoveListener(ofEvents().exit, this, &ofxCVC::stop);
    ofLogNotice("ofxCVC") << "stop";
	for(int i=0; i<sockets.size(); i++) sockets[i]->close();
}
	
string ofxCVC::socketsToString(bool full) {
	ostringstream out;
	out <<  "{Sockets ";
    if(full) {
        for(int i=0; i<sockets.size(); i++) out << (i==0 ? "" : ", " ) << "[" << i  << "]:" << sockets[i]->toString();
    }else{
        for(int i=0; i<sockets.size(); i++) out << (i==0 ? "" : ", " ) << "[" << i  << "]:" << sockets[i]->toMinString();
    }
	out << "}";
	return out.str();
}
	
string ofxCVC::getSocketsMiniState() {
	// "n/c"
	int t = sockets.size();
	int c = 0;
	for(int n=0;n<t;n++) c += (getSocket(n)->isConnected() ? 1 : 0);
	
    ostringstream out;
	out << c << "/" << t;
	return out.str();
}

/*string ofxCVC::toFullString() {	
		String out = toString();		
		out += ",\n\t" + serversToString();		
		out +=  ",\nblobs=" + blobs.toString();
		return out;
	}*/

string ofxCVC::toString() {
	ostringstream out;
	out << "{ofxCVC[" << index << "] v" << getVersion() << ", queued=" << USE_QUEUED_UPDATES << ", sweep=" << SWEEP_FOR_STAGNANTS  << ", join=" << JOIN_INTERSECTED << ", smooth=" << SMOOTH_UPDATES << ", lockable=" << DATA_LOCKABLE << ", dims=[" << dimensions.x << "," << dimensions.y << "," << dimensions.width << "," << dimensions.height << "]" << "}";
    // << ", multi=" << SocketReader::isMultiServing << ", overlap=" << SocketReader::overlapEnabled
	return out.str();
}
