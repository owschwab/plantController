
#include "DFPlayer.h"

    // global variables
    DFPlayer dfPlayer;




int folder = 0;
int volume = 0;
bool powerOn = false;

void setup() {
    Particle.variable("folder", folder);
    Particle.variable("volume", volume);
    Particle.function("setVolume", setVolume);
    Particle.function("playFolder", playFolder);
    Particle.function("setPower", setPower);
    
    Serial.begin(115200);
    Serial1.begin(9600);
    delay(5000);


    // the following command tells the program to display data on the serial console.
    // logging can be turned on and off at will throughout the user's code
    dfPlayer.setLogging(true);
    volume = dfPlayer.getVolume();
    
}

void loop() {

}
// playFolder is a Particle Function ... that can be run from the Particle Console ..

int playFolder(String buffer) {
    folder = atoi(buffer);
    
    // the following commands play 6 tracks that were stored in the root folder of the TFCard
    // the dfPlayer.playTrack method uses DFPlayer command 0x03
    // The tracks were named 001 - 006.
    // The tracks were added to the TFCard in the following sequence 001, 002, 003, 005, 004, and 006
    // This code plays the tracks in order 001 - 006
    delay(3000);
    dfPlayer.playTrack(1);
    delay(3000);
    dfPlayer.playTrack(2);
    delay(3000);
    dfPlayer.playTrack(3);
    delay(3000);
    dfPlayer.playTrack(5);
    delay(3000);
    dfPlayer.playTrack(4);
    delay(3000);
    dfPlayer.playTrack(6);
    
    
    // the following commands play four tracks located in a folder on the TFCard
    // the dfPlayer.playFolderTrack method uses DFPlayer command 0x0F
    // the tracks were added to the TFCard in an arbitrary order, but play in sequence from 001 to 004
    delay(3000);
    dfPlayer.playFolderTrack(folder,1);
    delay(3000);
    dfPlayer.playFolderTrack(folder,2);
    delay(3000);
    dfPlayer.playFolderTrack(folder,3);
    delay(3000);
    dfPlayer.playFolderTrack(folder,4);
    return 0;
}

// setVolume is a Particle method that can be used to adjust volume via the Particle Console
int setVolume(String buffer) {
    volume = atoi(buffer);
    dfPlayer.setVolume(volume);
    return 0;
}

// setPower is a Particle method that that plays a gong when power is toggled false, and chirping birds when power is toggned true 
bool setPower(String buffer) {
    if ((buffer[0] == 'f') && (powerOn == true))  {
        powerOn = false;
        dfPlayer.playFolderTrack(1,2);
    }
    if ((buffer[0] == 't') && (powerOn == false)) {
        powerOn = true;
        dfPlayer.playFolderTrack(1,1);
    }
    return powerOn;
}   



/* ===============================================================================================================================
==================================================================================================================================

                                                  SAMPLE OUTPUT FROM THE ABOVE CODE
                                                  
    The first request gets the volume setting
    The other requests come from executing the Particle function "playFolder"
                                                
==================================================================================================================================
7e ff 04 43 01 fe b9 ef  --> request
7e ff 06 43 00 00 0f fe a9 ef <-- frame ok <-- checksum ok <-- data
7e ff 06 41 00 00 00 fe ba ef <-- frame ok <-- checksum ok <-- process complete, returning: 15

7e ff 06 03 01 00 01 fe f6 ef  --> request
7e ff 06 41 00 00 00 fe ba ef <-- frame ok <-- checksum ok <-- process complete, returning: 0

7e ff 06 03 01 00 02 fe f5 ef  --> request
7e ff 06 3d 00 00 01 fe bd ef <-- frame ok <-- checksum ok <-- playing
7e ff 06 3d 00 00 01 fe bd ef <-- frame ok <-- checksum ok <-- playing
7e ff 06 41 00 00 00 fe ba ef <-- frame ok <-- checksum ok <-- process complete, returning: 0

7e ff 06 03 01 00 03 fe f4 ef  --> request
7e ff 06 41 00 00 00 fe ba ef <-- frame ok <-- checksum ok <-- process complete, returning: 0

7e ff 06 03 01 00 05 fe f2 ef  --> request
7e ff 06 3d 00 00 02 fe bc ef <-- frame ok <-- checksum ok <-- playing
7e ff 06 3d 00 00 02 fe bc ef <-- frame ok <-- checksum ok <-- playing
7e ff 06 41 00 00 00 fe ba ef <-- frame ok <-- checksum ok <-- process complete, returning: 0

7e ff 06 03 01 00 04 fe f3 ef  --> request
7e ff 06 41 00 00 00 fe ba ef <-- frame ok <-- checksum ok <-- process complete, returning: 0

7e ff 06 03 01 00 06 fe f1 ef  --> request
7e ff 06 41 00 00 00 fe ba ef <-- frame ok <-- checksum ok <-- process complete, returning: 0

7e ff 06 0f 01 01 01 fe e9 ef  --> request
7e ff 06 41 00 00 00 fe ba ef <-- frame ok <-- checksum ok <-- process complete, returning: 0

7e ff 06 0f 01 01 02 fe e8 ef  --> request
7e ff 06 3d 00 00 01 fe bd ef <-- frame ok <-- checksum ok <-- playing
7e ff 06 3d 00 00 01 fe bd ef <-- frame ok <-- checksum ok <-- playing
7e ff 06 41 00 00 00 fe ba ef <-- frame ok <-- checksum ok <-- process complete, returning: 0

7e ff 06 0f 01 01 03 fe e7 ef  --> request
7e ff 06 41 00 00 00 fe ba ef <-- frame ok <-- checksum ok <-- process complete, returning: 0

7e ff 06 0f 01 01 04 fe e6 ef  --> request
7e ff 06 40 00 00 06 fe b5 ef <-- frame ok <-- checksum ok <-- DFPlayer ERROR: Unknown ERROR 0x06, returning: -4
*/








