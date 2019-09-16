/*
 
 DataUtil
 
 Binary Buffer and other data parsing utilites
 

 */

#pragma once

#ifndef _CVC_DATA_UTIL
#define _CVC_DATA_UTIL

namespace cvc {
    namespace data {
            
        inline short readShort(const char *buf, int *pos) {
            *pos += 2;
            return (buf[0] << 8 | buf[1]);
        }

        inline int readInt(const char *buf, int *pos) {
            *pos += 4;
            return (buf[0] << 24) | (buf[1] & 0xFF) << 16 | (buf[2] & 0xFF)  << 8 | (buf[3] & 0xFF); // its little endian
        }

        inline float readFloat(const char *buf, int *pos) {
            // see: http://stackoverflow.com/questions/3991478/building-a-32bit-float-out-of-its-4-composite-bytes-c
            
            float output;
            
            *pos += 4;
            
            *((char*)(&output) + 3) = buf[0];
            *((char*)(&output) + 2) = buf[1];
            *((char*)(&output) + 1) = buf[2];
            *((char*)(&output) + 0) = buf[3];
            
            return output;
        }

        inline int findShortPos(short search_val, const char * read_buffer, int limit, char * write_buffer) {
            // modeled on Processing 1.5, Client.java::readBytesUntilShort method
            
            char b = read_buffer[0]; // prev byte read
            write_buffer[0] = read_buffer[0];
            
            int found = -1;
            
            for (int k = 1; k < limit; k++) {
                write_buffer[k] = read_buffer[k];
                short t = (b << 8 | read_buffer[k]);
                if(t == search_val) { // byte to short conversion (signed)
                    found = k;
                    break;
                }
                b = read_buffer[k];
            }
            if (found == -1) return 0;
            
            return found;    
        }
        
    }
    
}

#endif
