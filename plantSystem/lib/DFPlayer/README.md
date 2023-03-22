# DFPlayer

## This library was developed for testing the capabilities of the DFPlayer (FN-M16P)

When I first heard about the DFPlayer, it sounded like it was just what I needed to build my next project. That project was going to be a Photon that could sound chimes and alarms based on events that other Photon projects have detected ... for example:

*	Sound a string of chimes when a garage door opens.
*	Sound a string of beeps if a garage door is open for more than 3 minutes between 8PM and 6AM
*	Sound a bong when the power company cuts power to our boiler, and sound chirping birds when the power is restored.

The new project would be placed in a central portion of the house, so the sound could be heard everywhere.

Testing proved that the DFPlayer was suitable for this purpose, but also revealed that the player has its share of warts. Most of these stem from poor documentation and an incohesive command structure.



One of the first things I learned while testing was that the serial connection to the DFPlayer carried was not as simple as I expected.

*	We send commands to the player
*	The player responds to those commands
*	The player also sends unsolicited messages ...
	*	an initialization message when the device boots or resets
	*	an "I'm playing" message every second while playing a track

I decided to build a library to help make sense of the traffic and the command structure. That library is still evolving, but I decided to release it ASIS in case it helps others in their testing ... Perhaps we can work together to make sense of this player.


