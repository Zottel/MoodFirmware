=====================
MoodlightUSB Firmware
=====================


How to use your Moodlight in 7 easy steps
-----------------------------------------

1. Get LUFA library::

     git submodule init
     git submodule update

2. Compile the software::

     make

3. Set your Moodlight to programming mode by holding the HWB button and pressing the RESET button.

4. Erase the old binary from your Moodlight (may require root privilege)::

     dfu-programmer atmega32u2 erase

5. Flash the new binary onto your Moodlight (may require root privilege)::

     dfu-programmer atmega32u2 flash Moodlight.hex 

6. Reset your Moodlight by pressing the RESET button.

7. Tell your Moodlight to blink for you (see PyMood Project)


Moodlight Sequence Programming
------------------------------

The Moodlight uses a simple bytecode format to execute sequences of colour changes. 

TODO: document bytecode format