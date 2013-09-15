UbarCDC
=======

This is an Arduino based CD changer / AUX-in enhancer for A2DP Bluetooth mediaplayers with a companion app for Android.

The main Sketch is UbarCDC.ino. As of now it supports the Alpine MBus CD changer protocol I found in an 1991 Jaguar XJS as well as IBus integration for a 2005 BMW Z4. 

The Alpine implementation is installed in parallel to an existing CD changer. Insert a self burnt disc with 15 tracks, each 5 minutes long and without a gap. The MBus implementation will detect this disc and initiate the A2DP playback.

The BMW IBus implementation is different. UbarCDC is connected to AUX-in of the radio unit as well as the IBus. When you select AUX in as the audio source you can change tracks with the radio buttons and steering wheel controls. If you are using an Android device you should also install the Android app as this allows you to assign Albums and Playlists to the 1-6 disc select buttons.

The Sketch includes drivers for an OVC 3860 and RN-52 bluetooth module. The RN52 implementation is slightly more advanced, as the OVC 3860 lacks bi-directional SPP support.
