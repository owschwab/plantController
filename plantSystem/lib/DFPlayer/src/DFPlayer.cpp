#include "DFPlayer.h"

// global constants related to the processing of response frames ...
    const int FST=0, VER=1, LEN=2, CMD=3, FBK=4, MSB=5, LSB=6, CMSB=7, CLSB=8, LST=9;   // define reference names for response frame bytes
    const int RESPONSE_FRAME_SIZE = 10;

/*  =======================================================================

                                PUBLIC METHODS
===========================================================================
Most modifications to DFPlayer command processing will involve making
chanes to the following methods and/or to the processResponseFrame() method. 

 *** This section controls how the DFPlayer requests are sent ...
     sendCommand methods are overloaded three command formats are supported:
        sendCommand(CMD);
        sendCommand(CMD, uint8_t MSB, uint8_t LSB);
        sendCommand(CMD, uint16_t var); 
        
 
 *** The processResponseFrame() method controls how DFPlayer responses are
     processed.

===========================================================================*/
DFPlayer::DFPlayer(void) { // DFPlayer constructor
//    playing = false;
    logging = false;
}
int DFPlayer::play(void)                                { return(sendCommand(0x03)); }              // limited testing, have questions about usage... Sometimes works, sometimes doesn't.
int DFPlayer::playTrack(uint16_t trk)                   { return(sendCommand(0x03, trk)); }         // limited testing, played trackes in root ...   played, trk: 0 to 2999
int DFPlayer::playNext(void)                            { return(sendCommand(0x01)); }              // tested ok, plays current track +1 in root and folders
int DFPlayer::playPrevious(void)                        { return(sendCommand(0x02)); }              // tested ok, plays current track -1 in root and folders
int DFPlayer::playFolderTrack(uint8_t fol, uint8_t trk) { return(sendCommand(0x0F, fol, trk)); }    // tested ok, but did not test limits ... fol: 1 to 100, trk 1-255
int DFPlayer::getStatus(void)                           { return(sendCommand(0x42)); }              // untested, what does "Status" mean? Guessing it should return MSB.
int DFPlayer::getVolume(void)                           { return(sendCommand(0x43)); }              // tested ok ...
int DFPlayer::setVolume(uint8_t vol)                    { return(sendCommand(0x06, 0, vol)); }      // tested ok ... vol: low to high, 0 to 30
int DFPlayer::increaseVolume(void)                      { return(sendCommand(0x04)); }              // tested ok ...
int DFPlayer::decreaseVolume(void)                      { return(sendCommand(0x05)); }              // tested ok ...
int DFPlayer::getEqualizer(void)                        { return(sendCommand(0x44)); }              // untested ...
int DFPlayer::setEqualizer(uint8_t equ)                 { return(sendCommand(0x07, 0, equ)); }      // limited testing, all settings sounded the same ... equ: EQ_NORMAL, EQ_POP, EQ_ROCK, EQ_JAZZ, EQ_CLASSIC, or EQ_BASS
int DFPlayer::playback(void)                            { return(sendCommand(0x0D)); }              // untested ...
int DFPlayer::getPlaybackMode(void)                     { return(sendCommand(0x45)); }              // untested ...
int DFPlayer::setPlaybackMode(uint8_t pbm)              { return(sendCommand(0x08, 0, pbm)); }      // limited testing, unexpected results ... pbm: PB_TRACK_REPEAT, PB_FOLDER_REPEAT, PB_SINGLE_PLAY, PB_RANDOM_PLAY
int DFPlayer::getFileCount_UDisk(void)                  { return(sendCommand(0x47)); }              // untested ...
int DFPlayer::getCurrentTrack_UDisk(void)               { return(sendCommand(0x4B)); }              // untested ...
int DFPlayer::getFileCount_TFCard(void)                 { return(sendCommand(0x48)); }              // tested ok ... (mfr doc said this was UDisk, but that was wrong)
int DFPlayer::getCurrentTrack_TFCard(void)              { return(sendCommand(0x4C)); }              // tested ok, but not useful (*Note 1) ... (mfr doc said this was UDisk, but that was wrong)
int DFPlayer::getFileCount_Flash(void)                  { return(sendCommand(0x49)); }              // untested ...
int DFPlayer::getCurrentTrack_Flash(void)               { return(sendCommand(0x4D)); }              // untested ...
int DFPlayer::setPlaybackSource(uint8_t pbs)            { return(sendCommand(0x09, 0, pbs)); }      // untested ...pbs: PS_UDISK, PS_TFCARD, PS_AUX, PS_SLEEP, PS_FLASH
int DFPlayer::setModeStandby(void)                      { return(sendCommand(0x0A)); }              // untested ...             // (low-power mode)
int DFPlayer::setModeNormal(void)                       { return(sendCommand(0x0B)); }              // untested ...
int DFPlayer::getSoftwareVersion(void)                  { return(sendCommand(0x46)); }              // untested ...
int DFPlayer::reset(void)                               { return(sendCommand(0x0C)); }              // tested ok ...
int DFPlayer::pause(void)                               { return(sendCommand(0x0E)); }              // tested ok ... stops track playing, but there is no resume command ...
int DFPlayer::repeatPlay(uint8_t rpt)                   { return(sendCommand(0x11, 0, rpt)); }      // untested ... rpt: 1 (start repeat), 0 (stop play)
int DFPlayer::adjustVolume(uint8_t adj, uint8_t gain)   { return(sendCommand(0x10, adj, gain)); }   // untested ... adj: 1, gain: 0-3

#ifdef LOGGING
    void DFPlayer::setLogging(bool log) { logging = log; }                                              // tested ok ... allows users to turn logging on and off when testing DFPlayer methods
#endif
                                    // *Note 1: command returns track number which corresponds to the order that the file was added to the TFCard, whether it is stored in the root or a folder     

/*  ============================================================================================================================

                                PRIVATE METHODS

================================================================================================================================

    sendCommand() is overloaded: 
    --  sendCommand(CMD} sends commands without parameters
    --  sendCommand(CMD, MSB, LSB) sends commands with two (uint8_t) parameters
    --  sendCommand(CMD, <doubleByte>) sends commands with one (uint16_t) parameter
    
    getResponse() processes response frames from the DFPlayer, and calls the following three sub-functions:
        --  assembleResponseFrame() reads Serial1 to build response frames
        --  validateResponseFrame() validates response frame format and checksums
        --  processResponseFrame() processes frames and determines the value that is returned when responseComplete is true.

================================================================================================================================*/


// -------------------------------------------------------------------- sendCommand(CMD} --> 4-byte payload

int DFPlayer::sendCommand(uint8_t cmd) {
    // declare an instance of the UIConverter union & compute the two-byte checksum
    UIConverter checksum;
	checksum.dblByte = 0xFFFF - (0xFF + 0x04 + cmd + 0x01) +1;
	// declare, populate, and send the command buffer to the DFPlayer
    uint8_t buf[] = {0x7E, 0xFF, 0x04, cmd, 0x01, checksum.byte[1], checksum.byte[0], 0xEF};
	Serial1.write(buf,sizeof(buf));
    #ifdef LOGGING
        if (logging) {
            Serial.println();
    	    for(uint n = 0; n < sizeof(buf); n++) {
    	        Serial.printf( "%02x ", buf[n]);
    	    }
	    Serial.println(" --> request");
        }
	#endif
    return getResponse();
}


// -------------------------------------------------------------------- sendCommand() - 6-byte Payload (2x uint8_t)

int DFPlayer::sendCommand(uint8_t cmd, uint8_t msb, uint8_t lsb) {
    // declare an instance of the UIConverter union & compute the two-byte checksum
    UIConverter checksum;
	checksum.dblByte = 0xFFFF - (0xFF + 0x06 + cmd + 0x01 + msb + lsb) +1;
	// declare, populate, and send the command buffer to the DFPlayer
    uint8_t buf[] = {0x7E, 0xFF, 0x06, cmd, 0x01, msb, lsb, checksum.byte[1], checksum.byte[0], 0xEF};
	Serial1.write(buf,sizeof(buf));
    #ifdef LOGGING
        if (logging) {
            Serial.println();
	        for(uint n = 0; n < sizeof(buf); n++) {
	            Serial.printf( "%02x ", buf[n]);
	        }
	     Serial.println(" --> request");
        }
    #endif
	return getResponse();
}


// -------------------------------------------------------------------- sendCommand() - 6-byte Payload (uint16_t)

int DFPlayer::sendCommand(uint8_t cmd, uint16_t dbyte) {
    // declare an instance of the UIConverter union to set the msb and lsb variables
    UIConverter uiconverter;
    uiconverter.dblByte = dbyte;   uint8_t msb = uiconverter.byte[1];    uint8_t lsb = uiconverter.byte[0];
    // compute the two-byte checksum and store it in the UIConverter
	uiconverter.dblByte = 0xFFFF - (0xFF + 0x06 + cmd + 0x01 + msb + lsb) +1;
	// declare, populate, and send the command buffer to the DFPlayer
    uint8_t buf[] = {0x7E, 0xFF, 0x06, cmd, 0x01, msb, lsb, uiconverter.byte[1], uiconverter.byte[0], 0xEF};
	Serial1.write(buf,sizeof(buf));
    #ifdef LOGGING
        if (logging) {
            Serial.println();
    	    for(uint n = 0; n < sizeof(buf); n++) {
    	        Serial.printf( "%02x ", buf[n]);
    	    }
    	    Serial.println(" --> request");
        }
    #endif
	return getResponse();
}


// -------------------------------------------------------------------- getResponse()

int DFPlayer::getResponse(void) {

//    returnValue = OK;
    uint8_t buffer[RESPONSE_FRAME_SIZE] = { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
  	int bx = 0; // buffer index

    int ret = OK;    
    bool responseComplete = false;
    while ( !responseComplete) {
        ret = assembleResponseFrame(buffer, bx, responseComplete);
        if (ret == OK) ret = validateResponseFrame(buffer, bx);
        if (ret == OK) ret = processResponseFrame(buffer, responseComplete);
        
    }
    for (uint i = 0; i < sizeof(buffer); i++) { buffer[i] = 0x00; }
    bx = 0;
    return ret;
}


// -------------------------------------------------------------------- assembleResponseFrame()


int DFPlayer::assembleResponseFrame(uint8_t frame[], int& fx, bool& responseComplete) {

    bool frameComplete = false;
    while (!frameComplete) {
        uint32_t timeStart = millis();

        // deal with Serial1 timeout issues ...
        while (!Serial1.available()) {
            if (millis() > (timeStart + 500)) {
                #ifdef LOGGING
                    if (logging) {
                        Serial.print("ERROR - Response Timeout, Returning: ");  Serial.println(RESPONSE_TIMEOUT);
                    }
                #endif
                responseComplete = true;
                return RESPONSE_TIMEOUT;
            } 
        }
    
        // read Serial1 and blindly fill the frame bytes
        while ((Serial1.available()) && (fx < RESPONSE_FRAME_SIZE)) {
            frame[fx]  =  Serial1.read();
            fx++;
        }
        
        // when the frame is full ... 
        if (fx == RESPONSE_FRAME_SIZE) {
            fx = 0;
            frameComplete = true;
            #ifdef LOGGING
                if (logging) {
                    for (int i = 0; i < RESPONSE_FRAME_SIZE; i++) {
                        Serial.printf("%02x ", frame[i]);
                    }
                }
            #endif
        }
    }
    return OK;
}


// -------------------------------------------------------------------- validateResponseFrame()


int DFPlayer::validateResponseFrame(uint8_t frame[], int& fx) {

    // calculate frame checksum
    UIConverter checksum;
    checksum.dblByte = 0xFFFF - (frame[VER] + frame[LEN] + frame[CMD] + frame[FBK] + frame[MSB] + frame[LSB]) +1;

    bool frameError = false, checksumError = false;
    for (int i = 0; i < RESPONSE_FRAME_SIZE; i++) {
        switch (i) {
            case  FST: if (!(frame[FST] == 0x7E)) { frameError = true; } break; 
            case  VER: if (!(frame[VER] == 0xFF)) { frameError = true; } break;
            case  LEN: if (!(frame[LEN] == 0x06)) { frameError = true; } break;
            case  CMSB: if (!(frame[CMSB] == checksum.byte[1])) { checksumError = true; } break;
            case  CLSB: if (!(frame[CLSB] == checksum.byte[0])) { checksumError = true; } break;
            case  LST: if (!(frame[LST] == 0xEF)) { frameError = true; } break;
        }   
    } 
    
    if (frameError) { 
        #ifdef LOGGING
            if (logging) {
                Serial.println("<-- invalid or incomplete frame detected, resyncing ...");
            }
        #endif
        //make a copy of the frame, and clear the frame
        uint8_t bufcpy[RESPONSE_FRAME_SIZE];
        for (uint i = 0; i < RESPONSE_FRAME_SIZE; i++) { bufcpy[i] = frame[i]; frame[i] = 0x00; }
        //scan the copied frame for a '7e' ... if found, copy it and the following bytes back to the frame
        bool cpy = false;
        for (uint i = 1; i < RESPONSE_FRAME_SIZE; i++) { // start with 1 to skip the '7e' we processed before
            if (!cpy) { 
                if (bufcpy[i] == 0x7e) { cpy = true; }// triggers copy process
            }
            if (cpy) { 
                frame[fx] = bufcpy[i]; fx++; } // performs copy process
        } return RESPONSE_VALIDATION_ERROR;
    }
    
    // frame is valid ...
    #ifdef LOGGING
        if (logging) {
            Serial.print("<-- frame ok ");
        }
    #endif

    // process checksum errors --------------------------------------------------   
    if (checksumError) {
        #ifdef LOGGING
            if (logging) {
                Serial.println("<-- checksum invalid, frame discarded");
            }
        #endif
        for (uint i = 0; i < RESPONSE_FRAME_SIZE; i++) { frame[i] = 0x00; }
        int fx = 0;
        return RESPONSE_CHECKSUM_ERROR;
    }
    // checksum is valid ...
    #ifdef LOGGING
        if (logging) {
            Serial.print("<-- checksum ok ");
        }
    #endif
    return OK;
}


/* -------------------------------------------------------------------- processResponseFrame()

    DFPlayer commands respond with one or more frames. We interpret each frame based on the frame[CMD] byte.

    All responses terminate with a frame where frame[CMD] denotes success or failure
        frame[CMD] == 0x41 denotes success
        frame[CMD] -- 0x40 denotes failure ... with frame[LSB] defining the type of failure
        
    Various types of data frames preceed the termination frames. This method includes a code block to process each type of frame.
        Many data frames return a value in the frame[LSB] byte.
        One data frame appears to return a value in the frame[MSB] byte.
        It seems likely that some data frames might need to return two-byte value, but no case has been identified, so that code block is untested. 
        Regardless of where the data comes from, the data is stored in the "returnValue" field, so the termination frame can return the value to the method caller. 
        
    Some frames are essentially ignored, but code is included to identify them when logging is enabled.
*/



int DFPlayer::processResponseFrame(uint8_t frame[], bool& responseComplete) {
    static int returnValue = OK; // <-- stores the value to be returned when "responseComplete" is true 
    switch (frame[CMD]) {
        
        // handle cases where DFPlayer returns "Response Complete" ... with an error
        case 0x40: 
            #ifdef LOGGING
                if (logging) {
                    switch(frame[LSB]) {
                        case 0x00: Serial.print("<-- DFPlayer ERROR: Module Busy, returning: "); Serial.println(MODULE_BUSY); break;
                        case 0x01: Serial.print("<-- DFPlayer ERROR: Request Incomplete, returning: "); Serial.println(REQUEST_INCOMPLETE); break;
                        case 0x02: Serial.print("<-- DFPlayer ERROR: Request Validation Error, returning: "); Serial.println(REQUEST_VALIDATION_ERROR); break;
                        case 0x05: Serial.print("<-- DFPlayer ERROR: Unknown ERROR 0x05, returning: "); Serial.println(ERROR_40_OTHER); break; // seen in testing, no mfr doc
                        case 0x06: Serial.print("<-- DFPlayer ERROR: Unknown ERROR 0x06, returning: "); Serial.println(ERROR_40_OTHER); break; // seen in testing, no mfr doc
                          default: Serial.print("<-- DFPlayer ERROR: Other, returning: "); Serial.println(ERROR_40_OTHER);
                    }
                }
            #endif
            switch(frame[LSB]) {
                case 0x00: returnValue = MODULE_BUSY; break;
                case 0x01: returnValue = REQUEST_INCOMPLETE; break;
                case 0x02: returnValue = REQUEST_VALIDATION_ERROR; break;
                  default: returnValue = ERROR_40_OTHER;
            }
            responseComplete = true;
            break;

        // Handle cases where DFPlayer reports "Response Complete"
        case 0x41: 
            #ifdef LOGGING
                if (logging) {
                    Serial.print("<-- process complete, returning: ");  Serial.println(returnValue);
                }
            #endif
            responseComplete = true;
            break;
            
        // Handle cases where DFPlayer returns a value in the  MSB
        case 0x42:      
            #ifdef LOGGING
                if (logging) {
                    Serial.println("<-- data");
                }
            #endif
            returnValue = frame[MSB];
            break;
        
        // Handle cases where DFPlayer returns a value in the LSB
        case 0x43:      case 0x44:      case 0x45:      case 0x46:      case 0x47:
        case 0x48:      case 0x49:      case 0x4B:      case 0x4C:      case 0x4D:
            #ifdef LOGGING
                if (logging) {
                    Serial.println("<-- data");
                }
            #endif
            returnValue = frame[LSB];
            break;
        
        // Handle cases where DFPlayer returns a two-byte value (no current cases, so this code is untested)
        // case 0xNN: 
            UIConverter uiconverter;
            uiconverter.byte[1] = MSB;  uiconverter.byte[0] = LSB;
            #ifdef LOGGING
                if (logging) {
                    Serial.println("<-- data");
                }
            #endif
            returnValue = uiconverter.dblByte;
            break;

        // Handle cases where DFPlayer is merely reporting that the player is playing ... not currently handled ...
        case 0x3C:      case 0x3D:      case 0x3E: 
            #ifdef LOGGING
                if (logging) {
                    Serial.println("<-- playing ");
                }
            #endif
            break;
            
        // Handle cases where DFPlayer is reporting initialization parameters ... not currently handled ...
            case 0x3F:
            #ifdef LOGGING
                if (logging) {
                    Serial.println("<-- DFPlayer initialization frame ... ignored");
                }
            #endif
            break;

        default: 
            #ifdef LOGGING
                if (logging) {
                Serial.println("<-- Unknown ???");
                }
            #endif
            break;
    }
    if (responseComplete) {
        int rv = returnValue;
        returnValue = OK;
        return rv;
    } else { 
        return OK;
    }
}

