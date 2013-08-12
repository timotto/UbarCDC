UbarCDC
=======

Arduino based CD changer enhancer for A2DP Bluetooth or SD/MP3 Music Shields

The first Sketch will be using either a Roving Network RN-52 (https://www.sparkfun.com/products/11678) or OVC3860 (check eBay) Bluetooth module. The RN-52 features an additional SPP Bluetooth profile, so the CD changer slot buttons on the radio can be used to switch between playlists using a simple app on the Smartphone. 

My first CD changer is an Alpine CHM-S601 using the M-Bus protocol. To connect the 1 wire Bus with ~12V levels to the 5V Arduino I use a L9637d from STMicroelectronics that I soldered onto some SSOP like breakout board. The RN-52 will suffer from the same fate, and I hope one day I'll be able to make my own PCBs. The M-Bus protocol is handled by the MBus Arduino library by Olstyle (https://github.com/Olstyle/MBus). Most functions are self explaining and can be easily changed to reflect another kind of CDC protocol. Some time later I will extend this to the BMW I-Bus protocol and some VW radios too.

The RN-52 outputs differential analog, SPDIF or I2S. The differential analog output requires an Op-Amp to use it on the car stereo and I already failed miserably soldering one without really knowing what I was doing. So I settled for SPDIF and bought one of those Toslink & Coaxial SPDIF to Analog converters. If I remove the connectors it makes a perfectly small PCB with all the electronics already in place.

