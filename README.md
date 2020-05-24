# Control-for-2-magnetic-loop-antennas-Arduino-nano
This is a modification of my former control of 3 antennas. 

It fits with the CNC shield V4 for arduino nano


The sofware "controller for 4 antennas" fits perfectly in two differentss configurations of harware:

-CNC shield V3 for arduino uno

-CNC shield V4 for arduino nano


Some people warned me that the sofware "controller for 3 antennas" didn't work in the cnc shield V3 for arduino nano.
The problem was the diode attached to pin 13 in arduino nano. 
The only way to solve it was to swap pins 12 & 13 to pins 4 & 7.
As a result, I have to use the pins 4 & 7 for the endstop switches. Those pins controlled the drivers for the third antenna



Conclusion : only 2 slot for step drivers are abailable.
