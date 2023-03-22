/*  Some of the methods in this libarary work ... others do not ... many have not even been tested !!!

    The library was released to help oters explore the functionality of the DFPlayer (FN-M16P) MP3 player.

    FOR TESTING, USERS SHOULD MAKE A LOCAL COPY OF THIS LIBRARY, AND BE PREPARED TO MAKE CORRECTIONS THE LIBRARY CODE. 

    MFR DOCUMENTATION: http://www.trainelectronics.com/Arduino/MP3Sound/TalkingTemperature/FN-M16P%20Embedded%20MP3%20Audio%20Module%20Datasheet.pdf
*/

#ifndef __DFPLAYER_H_
#define __DFPLAYER_H_
#include "application.h"
#define LOGGING true;       // comment out this line to remove logging code from production compiles
                         

//  DFPlayer module return codes
    const int OK = 0;                           // Source: (CMD: 41)
    const int MODULE_BUSY = -1;                 // Source: (CMD: 40, LSB 00)
    const int REQUEST_INCOMPLETE = -2;          // Source: (CMD: 40, LSB 01)
    const int REQUEST_VALIDATION_ERROR = -3;    // Source: (CMD: 40, LSB 02)
    const int ERROR_40_OTHER = -4;              // Source: (CMD: 40, LSB: ??)  -- have seen LSB: 05 and 06 ... meaning???

//  Library return codes
    /*  This library returns
            -   DFPlayer return codes listed above
            -   Positive integer values that represent data ... like volume level, track number, etc.
            -   Additional error codes listed below
    */
    const int RESPONSE_TIMEOUT = -11;
    const int RESPONSE_VALIDATION_ERROR = -12;
    const int RESPONSE_CHECKSUM_ERROR = -13;
    

// Equalizer setting constants ... settings per mfr doc sec 3.2, no sound difference detected in testing
	const uint8_t EQ_NORMAL = 0;
	const uint8_t EQ_POP = 1;
	const uint8_t EQ_ROCK = 2;
	const uint8_t EQ_JAZZ = 3;
	const uint8_t EQ_CLASSIC = 4;
	const uint8_t EQ_BASS = 5;

// Playback Mode constants  ... settings per mfr doc sec 3.2, command did not perform as expected in testing, regardless of the setting
    const uint8_t PB_TRACK_REPEAT = 0;
    const uint8_t PB_FOLDER_REPEAT = 1;
    const uint8_t PB_SINGLE_PLAY = 2;
    const uint8_t PB_RANDOM_PLAY = 3;

// Playback Source constants ... mfr doc sec 3.2 showed 0-5 for U/TF/AUX/Sleep/flash ... example sec 3.4.3 showed that U was 1, TF was 2, and SLEEP was 5
    //  the values listed here may also correspond to the return value of the getStatus() method
    const uint8_t PS_UDISK = 1;     // per mfr doc sec 3.3.1 and 3.4.3
    const uint8_t PS_TFCARD = 2;    // testing shows this is true
    const uint8_t PS_AUX = 3;       // uDISK and TFCARD ? ref mfr doc sec 3.3.1
    const uint8_t PS_PC = 4;        // ? per mfr doc sec 3.3.1
    const uint8_t PS_SLEEP = 5;     // per mfr doc 3.4.3
    const uint8_t PS_FLASH = 8;     //  ? per mfr doc sec 3.3.1
	
//  This union provides an easy way to convert a calculated two-byte checksum to two one-byte uint8_t characters ... and vice versa
	union UIConverter{
        uint16_t dblByte;
        uint8_t byte[2];
	};

/* --------------------------------------------------------

DFPlayer class definition

-----------------------------------------------------------*/

class DFPlayer {
	private: // objects

        #ifdef LOGGING
	        bool logging;   /*  when the "#ifdef LOGGING true" statement is NOT commented out (at the top of this file 
	                        testers can turn logging on and off within their code via the setLogging() method.
                        */
        #endif

    private: // methods
		int sendCommand(uint8_t);
		int sendCommand(uint8_t, uint8_t, uint8_t);
		int sendCommand(uint8_t, uint16_t);
		int getResponse(void);
		int assembleResponseFrame(uint8_t frame[], int& fx, bool& rc);
		int validateResponseFrame(uint8_t frame[], int& fx);
		int processResponseFrame(uint8_t frame[], bool& rc);

	public: // methods
	    DFPlayer();
		int play(void);                 
		int playTrack(uint16_t trk);
		int playNext(void);                 
		int playPrevious(void);
		int getVolume(void);            
		int setVolume(uint8_t vol);         
		int increaseVolume(void);           
		int decreaseVolume(void);
		int getEqualizer(void);         
		int setEqualizer(uint8_t eq);
		int getStatus(void);
		int playback(void);
        int getPlaybackMode(void);
        int setPlaybackMode(uint8_t);
        int getFileCount_TFCard(void);
        int getCurrentTrack_TFCard(void);
        int getFileCount_UDisk(void);
        int getCurrentTrack_UDisk(void);
        int getFileCount_Flash(void);
        int getCurrentTrack_Flash(void);
        int setPlaybackSource(uint8_t);
        int playFolderTrack(uint8_t, uint8_t);
        int setModeStandby(void);
        int setModeNormal(void);
        int getSoftwareVersion(void);
        int reset(void);
        int pause(void);
        int adjustVolume(uint8_t, uint8_t);
        int repeatPlay(uint8_t);
        bool isPlaying(void);
        void setLogging(bool);
};	

#endif