#include "SocketReader.h"

#include "ofxCVC.h" // here to avoid circular references.
#include "BlobManager.h"

using namespace cvc;

//--------------------------------------------------------------

SocketReader::SocketReader() {

}

void SocketReader::setup(int _cvc_index, int _index, string _host, int _port, bool _run_as_thread, bool _auto_connect, bool _auto_reconnect, float _reconnect_interval_secs){
    
    cvc_index = _cvc_index,
	
    index = _index;
    host = _host;
	port = _port;
    
    scaler.set(0,0,640,480); // default size.
    
    auto_connect = _auto_connect;
    auto_reconnect = _auto_reconnect;
    
	if(_reconnect_interval_secs > FLT_MIN || _reconnect_interval_secs > FLT_MAX) { // povo NaN
		reconnect_interval = (int)round(_reconnect_interval_secs * 1000.f);
	}
	
    run_as_thread = _run_as_thread;
    
    if(!run_as_thread) {  // use a listener to update
        ofAddListener(ofEvents().update, this, &SocketReader::_update); // May need to pass, OF_EVENT_ORDER_BEFORE_APP as last arg to solve possible execution issues?
    }
    
    if(auto_connect) connect();
    
}

void SocketReader::connect() {
    return connect(host, port);
}

void SocketReader::connect(string _host, int _port) {
    
    if(isConnected()) return false; // || tcp_client.isConnecting())
    
    if(host != _host) host = _host;
    if(port != _port) port = _port;
    
	// _connected = tcp_client.setup(host, port, false); // not blocking
    
	last_connect_attempted_time = 0;
    
    _connecting = true;
    tcp_client.setup(host, port, false); // not blocking
    
   /* if(_connected) {
        ofLogNotice("Connected") << toString();
    }*/
 	
	if(run_as_thread){
    	if(!isThreadRunning()){
        	ofLogNotice("SocketReader") << "Starting Thread";
        	startThread(true);   // blocking
			ofLogNotice("SocketReader Thread name") << getThreadName();
    	}
	}
	   
	// return _connected;
}

void SocketReader::connected() {
     ofLogNotice("SocketReader::Connected") << toString();
    _connected = true;
    _connecting = false;
}

bool SocketReader::disconnect() {
    
    // if(!tcp_client.isConnected()) return false;
    
    tcp_client.close();
    
    _connected = false;
    _connecting = false;
    
    return true;
}

void SocketReader::resize(float x, float y, float w, float h) {
	ofLogNotice("resize") << "SocketReader[" << index <<"] " << x << "," << y << "," << w << "," << h;
	// TODO: isMutliserving smart resize logic.. Might be better if this was done up at the ofxCVC level to avoid reference passing etc
    scaler.set(x, y, w, h);
}

void SocketReader::checkConnection() {
    
    if(auto_reconnect) {
        if((ofGetElapsedTimeMillis() - last_connect_attempted_time) > reconnect_interval ){
            // _connected =
            if(!_connecting) {
                connect(); //  tcp_client.setup(host, port);
            }
            last_connect_attempted_time = ofGetElapsedTimeMillis();
        }
    }
    
}

//--------------------------------------------------------------

void SocketReader::threadedFunction() {
    
    while(isThreadRunning()){
        
       // lock();
        update();
      //  unlock();
        
        sleep(10); // sleep the thread to give up some cpu was 5 in CVC.java
    }

}

void SocketReader::_update(ofEventArgs& args) {
    update();
}

void SocketReader::update(){ // Will become the threaded method.
	
    if(_connecting) {
    
        if(tcp_client.isConnected()) {
            connected();
        }else if(!tcp_client.isConnecting()){
            // must have timed out
            _connecting = false;
        }
        
    }else if(_connected){
                
        int recievedBytes = 0;
        int totalReceivedBytes = 0;
        
        position = 0;
        
        while(true) {
            recievedBytes = tcp_client.receiveRawBytes(&read_buffer[position], MAX_BUFFER_SIZE);
            
            if(recievedBytes < 1)  break; // nothing left in that buffer.
            
            totalReceivedBytes += recievedBytes;
            position += recievedBytes;
        }
        
        // read_buffer is filled with  data up to totalReceivedBytes
        // now walk it looking for msgEndEntityAt
                
        int msg_end_entity_pos = -1;
        int p = 0;
        int ln;
        
        while(true) {
            msg_end_entity_pos = data::findShortPos(MESSAGE_END, &read_buffer[p], totalReceivedBytes-p, &write_buffer[0]);
                        
            if(msg_end_entity_pos < 1) break;
            
            ln = msg_end_entity_pos; // p should be greater than msgEnd
            
            bool valid_msg = parseMessage(&read_buffer[p], ln);
            
            if(!valid_msg) {
                cout << "invalid msg ln=" << ln << ", ent=" << msg_end_entity_pos << ", p=" <<  p << endl;
                break;
            }
            
            p += ln + 1;
        }
        
        // cout << "msg_end_entity_pos = " << msg_end_entity_pos << endl;
                            
        if(!tcp_client.isConnected()){
			_connected = false;
		}
        
	}else if(auto_reconnect){

        checkConnection();
        
	}
}

bool SocketReader::parseMessage(const char *buf, int len) {
    
    if(len < MIN_CVC_MSG_SIZE) return false;
    
    int p = 0;
    
    short start_entity = data::readShort(&buf[p], &p); // 2 bytes
    //cout << "start_entity=" << start_entity << " expected " <<  MESSAGE_START;
    char cmd = buf[p++]; // 1 byte
    
    int blob_id = mapBlobID(data::readInt(&buf[p], &p)); // 4 chars
   // cout << "blob_id=" << blob_id << endl;
    
    if(cmd == DESTROY_BLOB) {
        // Shorter message - it has not other properties besides command and id.
        if(!CVC(cvc_index)->IGNORE_DELETE_COMMANDS) CVC(cvc_index)->blobs->destroyBlob(blob_id, BlobManager::TRACKING);
    }else{
        
        bool has_outlines = len > BASE_MESSAGE_LENGTH;
        //cout << "has_outlines=" << has_outlines << endl;
        
        float b_x = data::readFloat(&buf[p], &p); // 4 bytes for a float.
        float b_y = data::readFloat(&buf[p], &p);
        float r_x = data::readFloat(&buf[p], &p);
        float r_y = data::readFloat(&buf[p], &p);
        float r_w = data::readFloat(&buf[p], &p);
        float r_h = data::readFloat(&buf[p], &p);
        
        int x = scaler.x;
        int y = scaler.y;
        int w = scaler.width;
        int h = scaler.height;

        int current_position = BASE_MESSAGE_LENGTH;
        
        if(has_outlines) {
            // Parse the float outlines to screen coords            
            // mapOutlines(&buf[p], len-p); ?? need to read from the buffer and convert to an float array            
            // TODO:
            // Plan: Send pointer to buffer for the outlines the blobs(buf[p] & len)
            // let it take care of the converting to screen vals. (, applying w&h to outlines.)
            // This way the blob will make the final copy in its self
            // Blob would need more of a buffer structure too (long array with a len pos), so it can expand and contract based on size of values coming in.
        }
        
        if(cmd == UPDATE_BLOB) {
           
            CVC(cvc_index)->blobs->updateBlob(blob_id, x+(b_x*w), y+(b_y*h), x+(r_x*w), y+(r_y*h), x+(r_w*w), y+(r_h*h), b_x, b_y); //, outline_coords); // send pointer to outlines[0] & length

        }else if(cmd == CREATE_BLOB){
            
            CVC(cvc_index)->blobs->createBlob(blob_id, x+(b_x*w), y+(b_y*h), x+(r_x*w), y+(r_y*h), x+(r_w*w), y+(r_h*h), b_x, b_y); //, outline_coords);

        }else{
			ofLogError("SocketReader::parseMessage: Uknown cmd of ") << cmd;
			return false;
		}
        
        // createBlob updateBlob deleteBlob can all be called direct to blobmanager, saving having methods here, just need a method to translate the outlines to screen space.
        
    }
    
    return true;
}

void SocketReader::mapOutlines(float *outlines, int *len) {
    //TODO:have the outlines as a buffer, and map to screen space.
        
}

void SocketReader::close() {
    
    ofLogNotice("SocketReader") << "[" << index << "]" << "close:" << isThreadRunning() << "," << tcp_client.isConnected();
    
    if(run_as_thread){
		if(isThreadRunning()) waitForThread(); // stopThread(); use wait to stop instead
    }
        
    if(tcp_client.isConnected()) tcp_client.close();
    
    _connected = _connecting = false;
}

bool SocketReader::isConnected(){
    return _connected; // tcp_client.isConnected();
}

bool SocketReader::isConnecting(){
    return _connecting; // tcp_client.isConnecting();
}

string SocketReader::toString() {
	ostringstream out;
	out << "{SocketReader[" << index << "]" << host << ":" << port << ", conn=" << _connected << ", connecting=" << _connecting << "}";
	return out.str();
}

string SocketReader::toMinString() {
    string conn_stats = _connected ? "(+)" : "(-)";
    if(_connecting) conn_stats = "(...)";
    return conn_stats + host + ":" + ofToString(port) + (run_as_thread ? ".t" : ".m");
}

