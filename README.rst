=====================
MoodlightUSB Firmware
=====================


How to use your Moodlight in 8 easy steps
#########################################

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

6. Write initial colour sequence to eeprom::

     dfu-programmer atmega32u2 flash-eeprom Moodlight.eep

7. Reset your Moodlight by pressing the RESET button.

8. Tell your Moodlight to blink for you (see PyMood Project)

TODO
####

- Check if logarithm table is okay, also whether to use it for HSV or not.
  HSV hue fading looks better when not using logarithmic PWM.

- Introduce instruction to write eeprom from application
  
  Something like "write n bytes following this to address p"

- Move LUFA interfacing code further away from genericHID echo demo

Moodlight Sequence Programming
##############################

*This description is focused on the lowlevel programming of colour sequences.*

The Moodlight uses a simple bytecode format to execute sequences of colour changes. The instructions are inspired by the BlinkM_ project.

Initially, execution starts at the beginning of the built-in EEPROM,
but every HID report sent from connected host devices is immediatly executed
and may run until the next report arrives or jump back into EEPROM.


Bytecode Format
---------------

+----------------------------+----------+--------------------------------------+
| Format                     | Name     | Description                          |
+============================+==========+======================================+
| 0x00                       | Halt     | Stops execution of program           |
+----------------------------+----------+--------------------------------------+
| 0x10 <r> <g> <b>           | SetRGB   | Set RGB colour to (r, g, b) instantly|
+----------------------------+----------+--------------------------------------+
| 0x11 <h> <s> <v>           | SetHSV   | Same for HSV                         |
+----------------------------+----------+--------------------------------------+
| 0x20 <r> <g> <b> <d1> <d2> | FadeRGB  | Fades to (r, g, b)                   |
|                            |          | The fade duration is given in        |
|                            |          | milliseconds and stored in d1 and d2,|
|                            |          | d1 being the high byte.              |
+----------------------------+----------+--------------------------------------+
| 0x21 <h> <s> <v> <d1> <d2> | FadeHSV  | Same for HSV                         |
+----------------------------+----------+--------------------------------------+
| 0x30 <d1> <d2>             | Wait     | Wait for `(d1 << 8) | d2`            |
|                            |          | milliseconds                         |
+----------------------------+----------+--------------------------------------+
| 0x80 <t1> <t2>             | Goto     | Jump to address `(t1 << 8) | t2`     |
|                            |          | in current address space.            |
+----------------------------+----------+--------------------------------------+
| 0x81 <t1> <t2>             | Goto ROM | Jump to address in ROM               |
+----------------------------+----------+--------------------------------------+


References
##########

.. _BlinkM: http://thingm.com/products/blinkm

