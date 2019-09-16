#include "ofxTCPClientThreaded.h"
#include "ofAppRunner.h"
#include "ofxNetworkUtils.h"

//--------------------------
ofxTCPClientThreaded::ofxTCPClientThreaded(){

	connected	= false;
	messageSize = 0;
	port		= 0;
	index		= -1;
	str			= "";
	tmpStr		= "";
	ipAddr		= "000.000.000.000";

	partialPrevMsg = "";
	messageDelimiter = "[/TCP]";
	memset(tmpBuff,  0, TCP_MAX_MSG_SIZE+1);
}

//--------------------------
ofxTCPClientThreaded::~ofxTCPClientThreaded(){
	close();
}

//--------------------------
void ofxTCPClientThreaded::setVerbose(bool _verbose){
    ofLogWarning("ofxTCPClientThreaded") << "setVerbose(): is deprecated, replaced by ofLogWarning and ofLogError";
}

//--------------------------
void ofxTCPClientThreaded::setup(string ip, int _port, bool _blocking){
    
    // Alot of this is moved into the threaded function so we dont get a lock for addresses not currently existing
    
    if(connecting) {
        
        TCPClient.Close();
        
        if(isThreadRunning()) stopThread();
        
        connecting	= false;

        // ofLogWarning("ofxTCPClientThreaded: can't setup as currently connecting");
        // return;
    }
    
	if( !TCPClient.Create() ){
		ofLogError("ofxTCPClientThreaded") << "setup(): couldn't create client";
		//return false;
    }
    
    /* // These time out value changes proved useless for the connecting issue, but they did kill the reading of the socket.
     
     TCPClient.SetTimeoutAccept(10); // seconds
    TCPClient.SetTimeoutReceive(10); // seconds
    TCPClient.SetTimeoutSend(10);
    
    ofLogNotice("\tTCPClient:timeouts") << "send=" << TCPClient.GetTimeoutSend() << ", recieve=" << TCPClient.GetTimeoutReceive() << ", accept=" << TCPClient.GetTimeoutAccept();
    */

    port		= _port;
	ipAddr		= ip;

    blocking    = _blocking;
    connecting	= true;
        
    if(!isThreadRunning()) startThread(); // blocking verbose

}

//don't use this one
//for server to use internally only!
//--------------------------
/*bool ofxTCPClientThreaded::setup(int _index, bool blocking){
	index = _index;

	//this fetches the port that the server
	//sets up the connection on
	//different to the server's listening port
	InetAddr addr;
	if( TCPClient.GetRemoteAddr(&addr) ){
		port   =  addr.GetPort();
		ipAddr =  addr.DottedDecimal();
	}

	TCPClient.SetNonBlocking(!blocking);
	connected = true;
	return true;
}*/
//--------------------------

void ofxTCPClientThreaded::threadedFunction() {
    
    // do the connecting on the thread
    
    ofLogNotice("ofxTCPClientThreaded:connect") << ipAddr << ":" << port << " @ " << ofGetElapsedTimeMillis();
    
	if( !TCPClient.Connect((char *)ipAddr.c_str(), port) ){ // << HERES WHERE IT BLOCKS FOR 75secs
		ofLogError("ofxTCPClientThreaded") << "Couldn't connect to " << ipAddr << " " << port << " @ " << ofGetElapsedTimeMillis();
        if(!TCPClient.Close()) { //we free the connection
            ofLogError("Unable to close TCPClient connection too.");
        }
        connecting = false;
       // stopThread();
		return;
	}
    
	TCPClient.SetNonBlocking(!blocking);
    
    connecting = false;
    connected = true;
    
   // stopThread();
    
    /*while(isThreadRunning()){
        
        ofSleepMillis(10);
    }*/
    
    
}

//--------------------------
bool ofxTCPClientThreaded::close(){
    
    if(isThreadRunning()) waitForThread();
    
    connecting = false;
    
	if( connected ){

		if( !TCPClient.Close() ){
			ofLogError("ofxTCPClientThreaded") << "close(): couldn't close client";
			return false;
		}else{
			connected = false;
			return true;
		}
	}else{
		return true;
	}
}

//--------------------------
void ofxTCPClientThreaded::setMessageDelimiter(string delim){
	if(delim != ""){
		messageDelimiter = delim; 
	}
}

//--------------------------
bool ofxTCPClientThreaded::send(string message){
	// tcp is a stream oriented protocol
	// so there's no way to be sure were
	// a message ends without using a terminator
	// note that you will receive a trailing [/TCP]\0
	// if sending from here and receiving from receiveRaw or
	// other applications
	if(!connected){
		ofLogWarning("ofxTCPClientThreaded") << "send(): not connected, call setup() first";
		return false;
	}
	message = partialPrevMsg + message + messageDelimiter;
	message += (char)0; //for flash
	int ret = TCPClient.SendAll( message.c_str(), message.length() );
    
    //ofLogNotice("TCPClient:send") << ret << "/" << message.length() << ":'" << message.c_str() << "'"  << endl;
    
	if( ret == 0 ){
		ofLogWarning("ofxTCPClientThreaded") << "send(): client disconnected";
		close();
		return false;
	}else if(ret<0){
		ofLogError("ofxTCPClientThreaded") << "send(): sending failed";
		return false;
	}else if(ret<(int)message.length()){
		// in case of partial send, store the
		// part that hasn't been sent and send
		// with the next message to not corrupt
		// next messages
		partialPrevMsg=message.substr(ret);
		return true;
	}else{
		partialPrevMsg = "";
		return true;
	}
}

bool ofxTCPClientThreaded::sendRawMsg(const char * msg, int size){

	// tcp is a stream oriented protocol
	// so there's no way to be sure were
	// a message ends without using a terminator
	// note that you will receive a trailing [/TCP]\0
	// if sending from here and receiving from receiveRaw or
	// other applications
	if(!connected){
		ofLogWarning("ofxTCPClientThreaded") << "sendRawMsg(): not connected, call setup() first";
		return false;
	}
	tmpBuffSend.append(msg,size);
	tmpBuffSend.append(messageDelimiter.c_str(),messageDelimiter.size());

	int ret = TCPClient.SendAll( tmpBuffSend.getBinaryBuffer(), tmpBuffSend.size() );
	if( ret == 0 ){
		ofLogWarning("ofxTCPClientThreaded") << "sendRawMsg(): client disconnected";
		close();
		return false;
	}else if(ret<0){
		ofLogError("ofxTCPClientThreaded") << "sendRawMsg(): sending failed";
		return false;
	}else if(ret<size){
		// in case of partial send, store the
		// part that hasn't been sent and send
		// with the next message to not corrupt
		// next messages
		tmpBuffSend.set(&tmpBuffSend.getBinaryBuffer()[ret],tmpBuffSend.size()-ret);
		return true;
	}else{
		tmpBuffSend.clear();
		return true;
	}
}

//--------------------------
bool ofxTCPClientThreaded::sendRaw(string message){
	if( message.length() == 0) return false;

	if( !TCPClient.SendAll(message.c_str(), message.length()) ){
		ofLogError("ofxTCPClientThreaded") << "sendRawBytes(): sending failed";
		close();
		return false;
	}else{
		return true;
	}
}

//--------------------------
bool ofxTCPClientThreaded::sendRawBytes(const char* rawBytes, const int numBytes){
	if( numBytes <= 0) return false;

	if( !TCPClient.SendAll(rawBytes, numBytes) ){
		ofLogError("ofxTCPClientThreaded") << "sendRawBytes(): sending failed";
		close();
		return false;
	}else{
		return true;
	}
}


//this only works after you have called receive
//--------------------------
int ofxTCPClientThreaded::getNumReceivedBytes(){
	return messageSize;
}

//--------------------------
static void removeZeros(char * buffer, int size){
	for(int i=0;i<size-1;i++){
		if(buffer[i]==(char)0){
			for(int j=i;j<size-1;j++){
				buffer[j]=buffer[j+1];
			}
			buffer[size-1]=(char)0;
		}
	}
}

//--------------------------
string ofxTCPClientThreaded::receive(){

	str    = "";
	int length=-2;
	//only get data from the buffer if we don't have already some complete message
	if(tmpStr.find(messageDelimiter)==string::npos){
		memset(tmpBuff,  0, TCP_MAX_MSG_SIZE+1); //one more so there's always a \0 at the end for string concat
		length = TCPClient.Receive(tmpBuff, TCP_MAX_MSG_SIZE);
		if(length>0){ // don't copy the data if there was an error or disconnection
			removeZeros(tmpBuff,length);
			tmpStr += tmpBuff;
		}
	}

	// check for connection reset or disconnection
	if((length==-1 && (ofxNetworkCheckError() == ECONNRESET) ) || length == 0){
		close();
		if(tmpStr.length()==0) // return if there's no more data left in the buffer
			return "";
	}

	// process any available data
	if(tmpStr.find(messageDelimiter)!=string::npos){
		str=tmpStr.substr(0,tmpStr.find(messageDelimiter));
		tmpStr=tmpStr.substr(tmpStr.find(messageDelimiter)+messageDelimiter.size());
	}
	return str;
}


static int findDelimiter(char * data, int size, string delimiter){
	unsigned int posInDelimiter=0;
	for(int i=0;i<size;i++){
		if(data[i]==delimiter[posInDelimiter]){
			posInDelimiter++;
			if(posInDelimiter==delimiter.size()) return i-delimiter.size()+1;
		}else{
			posInDelimiter=0;
		}
	}
	return -1;
}

int ofxTCPClientThreaded::receiveRawMsg(char * receiveBuffer, int numBytes){
	int length=-2;
	//only get data from the buffer if we don't have already some complete message
	if(findDelimiter(tmpBuffReceive.getBinaryBuffer(),tmpBuffReceive.size(),messageDelimiter)==-1){
		memset(tmpBuff,  0, TCP_MAX_MSG_SIZE);
		length = receiveRawBytes(tmpBuff, TCP_MAX_MSG_SIZE);
		if(length>0){ // don't copy the data if there was an error or disconnection
			tmpBuffReceive.append(tmpBuff,length);
		}
	}

	// process any available data
	int posDelimiter = findDelimiter(tmpBuffReceive.getBinaryBuffer(),tmpBuffReceive.size(),messageDelimiter);
	if(posDelimiter>0){
		memcpy(receiveBuffer,tmpBuffReceive.getBinaryBuffer(),posDelimiter);
		if(tmpBuffReceive.size() > posDelimiter + (int)messageDelimiter.size()){
			memcpy(tmpBuff,tmpBuffReceive.getBinaryBuffer()+posDelimiter+messageDelimiter.size(),tmpBuffReceive.size()-(posDelimiter+messageDelimiter.size()));
			tmpBuffReceive.set(tmpBuff,tmpBuffReceive.size()-(posDelimiter+messageDelimiter.size()));
		}else{
			tmpBuffReceive.clear();
		}
	}

	if(posDelimiter>0){
		return posDelimiter;
	}else{
		return 0;
	}
}

//--------------------------
int ofxTCPClientThreaded::receiveRawBytes(char * receiveBuffer, int numBytes){
	messageSize = TCPClient.Receive(receiveBuffer, numBytes);
	if(messageSize==0){
		close();
	}
	return messageSize;
}

//--------------------------
string ofxTCPClientThreaded::receiveRaw(){
	messageSize = TCPClient.Receive(tmpBuff, TCP_MAX_MSG_SIZE);
	if(messageSize==0){
		close();
	}else if(messageSize<TCP_MAX_MSG_SIZE) {
        // null terminate!!
        tmpBuff[messageSize] = 0;
    }

	return tmpBuff;
}

//--------------------------
bool ofxTCPClientThreaded::isConnected(){
	return connected;
}

//--------------------------
bool ofxTCPClientThreaded::isConnecting(){
	return connecting;
}

//--------------------------
int ofxTCPClientThreaded::getPort(){
	return port;
}

//--------------------------
string ofxTCPClientThreaded::getIP(){
	return ipAddr;
}






