/*
	ofxCVC
	Reads and reassembled VideoTracker data stream
	Port of CVClient (Java and AS3)
 
    Adam Hinshaw 12/3/14
*/

#pragma once

#define CVC_MAJOR 0
#define CVC_MINOR 8
#define CVC_PATCH 2

#define CVC(_index) ofxCVC::instance(_index)

#include "ofMain.h"
#include "ofEvents.h"
#include "DebugRenderer.h"
#include "BlobManager.h"
#include "network/SocketReader.h"
#include "events/ChangeEvent.h"
#include "ui/ControlPanel.h"

#include "XmlSettingsEx.h"

class BlobManager;
class DebugRenderer;
class ControlPanel;
class ChangeEvent;

class ofxCVC {
    
	public:
    
       /* static ofxCVC& instance() {  // a reference instead of pointer in other classes. see http://stackoverflow.com/questions/86582/singleton-how-should-it-be-used
            static ofxCVC _instance;
            return _instance;
        }*/
    
        // Trying as pointer again (for now, trying to overcome SocketReader and linking error.)
       /* static ofxCVC* instance() {
            static ofxCVC *_instance;
            if(!_instance) _instance = new ofxCVC();
            return _instance;
        }*/
    
        static ofxCVC* instance(int index = 0) {
            static vector<ofxCVC*> _instances;
            if(_instances.size() < index+1){
                // TODO: Fix as this will fail if things are not accessed incrementally, e.g,, instance(0) instance(1)
                _instances.push_back(new ofxCVC(_instances.size()));
                ofLogNotice("ofxCVC:adding new instance") << _instances.size();
            }
            return _instances[index];
        }
    
        bool AUTO_CLEAR_DESTROYED_CACHE             = true;         // Flag for external control of when destroyed cache is cleared
        bool SWEEP_FOR_STAGNANTS                    = true;         // Look for blobs that have not moved for a while and kill them
        int BLOB_STAGNATION_KILL_AGE                = 1000 * 10;    // Age in milliseconds to kill a blob if it has not moved (see:SWEEP_FOR_STAGNANTS)
        bool OVERRIDE_SERVER_AUTO_CONNECT_CONFIG    = false;        // Dont allow connections to be auto connected or auto reconnected based on external Config settings.
		bool IGNORE_DELETE_COMMANDS                 = false;        // Ignore delete commands coming from sockets, (i.e, just let let SWEEP_FOR_STAGNANTS do it)
		bool USE_QUEUED_UPDATES                     = false;        // Use the overwrites and cull queue events.
		bool JOIN_INTERSECTED                       = false;        // join blobs that intersect each other.
		bool SMOOTH_UPDATES                         = false;        // Interpolate from existing position to the updated position, stopping jarring movements
		float UPDATE_INTERPOLATION                  = .15f;         // Amount to interpolate SMOOTH_UPDATES with
        bool DATA_LOCKABLE                          = true;         // use a mutex to lock when accessing the main blobs structure
    
        bool MUTLI_SOCKETS_ENABLED                  = false;        // Use multiple socket readers
        bool OVERLAP_ENABLED                        = false;        // Mutliple SocketReaders are to be layed out with overlaps
    
        bool THREAD_SOCKETS                         = false;        // Run the SocketReaders on threads.
    
        string& getVersion();

    	void setup();
		void update();
		
		void draw(float x=0.0, float y=0.0, float sx=1.0, float sy=1.0); // render the debug renderer
		
		void resize();
		
		void setDimensions(float x, float y, float w, float h);
		void addSocket(const string & ip, int port, bool _run_as_thread = false, bool auto_connect = true, bool auto_reconnect = true, float reconnect_interval_secs = 15.0f, const string & offset_rect_str = "");
		
		SocketReader * getSocket(int n);
		int getTotalSockets();
		//vector<int> getManualModeSocketIndexs();
		
		void setSocketScaleDims(float x, float y, float w, float h);
		
		ofRectangle & getDimensions();
		//void setDynamicDimensions(bool b);
		
		void toggleDebugRenderer();
		void showDebugRenderer(bool state);
		
		int getRowCount();
		int getColumnCount();
		
		void stop(ofEventArgs & e);
    
        bool loadConfigFromString(const string& xml_str);
        bool loadConfigFromFile(const string& xml_file_uri);
        bool loadConfigFromXML(const string& xml_str);
    
        bool isInitialised();
    
		string getSocketsMiniState();
		string socketsToString(bool full = false);
		string toString();
				
		vector<SocketReader*> sockets;
		
		ofRectangle dimensions; // space data is scaled into
		
		DebugRenderer* debug_renderer;
		
		//bool _dynamic_dimensions = true;

		ControlPanel* control_panel; // On screen UI (ofxGUI canvas?)

		BlobManager* blobs; // Blob states, making a pointer so i can use fwd referencing!
		
        int index;
    
        ofEvent<ChangeEvent> blobs_update_event;
        ofEvent<ChangeEvent> blobs_destroyed_event;
    
        // Events: TODO, a single register, unregister thang
        template<class ListenerClass>
        void registerForChangeEvents(ListenerClass * listener){
            cout << "registerForChangeEvents" << endl;
            ofAddListener(blobs_destroyed_event, listener, &ListenerClass::cvcBlobsDestroyed);
            ofAddListener(blobs_update_event,    listener, &ListenerClass::cvcBlobsUpdated);
        };
         
         template<class ListenerClass>
         void unregisterForChangeEvents(ListenerClass * listener){
             cout << "unregisterForChangeEvents" << endl;
             ofRemoveListener(blobs_destroyed_event,  listener, &ListenerClass::cvcBlobsDestroyed);
             ofRemoveListener(blobs_update_event,     listener, &ListenerClass::cvcBlobsUpdated);
         };
    
    private:
    
        ofxCVC(int _index = 0); // to avoid manual use
        
        // Stop the compiler generating methods of copy the object
        ofxCVC(ofxCVC const& copy);            // Not Implemented
        ofxCVC& operator=(ofxCVC const& copy); // Not Implemented
    
        ~ofxCVC() { cout << "ofxCVC Destroyed " << endl; }
    
        bool _initialised = false;
    
        bool parseConfig();
        bool parseSockets();
    
		// For multiple serving
		int row_width = 0;
		int col_height = 0;
    
        bbc::utils::XmlSettingsEx xml_settings;
};




