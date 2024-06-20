# numeric2mouse

Introduction
------------
I wrote a daemon to map ir-keytable keys to mouse movements and combination keys.

It basically:
- Creates a new user input
- Intercepts the old ir receiver input (an other keytable input should work too)
- If it is a numeric keypad number map it to mouse movement
- If it is a predefined key map it to combination keys (i.e. KEY_CLOSE to ALT+F4)
- Pass through everything else

I wrote this for my own use so it is not tested except for my own Raspbian.

History
-------
- Raspbian includes a LIRC driver. Installation of LIRC is not required.
- I created my own /etc/rc_keymaps/<remote-name>.toml file using ```ir-keytable -c -t -s rc3``` (your system device may differ, find out by running ir-keytable)
- Added the file to /etc/rc_maps.cfg and loaded it with ```ir-keytable -c -w /etc/rc_keymaps/<remote-name>.toml -t -s rc3```
- I wanted to move my mouse like I could do with LIRC using lircmd (part of package lirc)
- lircmd would eat the input and the predefined would not work anymore

Creating an ir-keytable map
--------------------------
The lines below will map keycodes to your remote.

1. Get your system device using ```ir-keytable 2>&1 | grep -B1 "gpio_ir_recv" | grep -Po "rc\d+"```
2. Run ```ir-keytable -s rc3 -t -p all``` (replace rc3 with the device you got on the first line)
3. Press keys on your remote aiming at your ir receiver and write down each key and code
4. Run ```nano /etc/rc_keymaps/Philips_RCLE013A.toml``` (replace "Philips_RCLE013A" with your own remote name)
5. Write/copy the following (and replace "Philips_RCLE013A" with your own remote name)
```
[[protocols]]
name = "Philips_RCLE013A"
protocol = "rc-5"
variant = "rc-5"
[protocols.scancodes]
```
6. From there on write pairs of keycode and key you want to map, i.e. ```0x0C = "KEY_CLOSE"```. The keys can be found at https://github.com/torvalds/linux/blob/master/include/uapi/linux/input-event-codes.h
7. Add ```* * Philips_RCLE013A.toml``` to ```/etc/rc_maps.cfg``` (replace "Philips_RCLE013A" with your own remote name)
8. Load and test by running ```ir-keytable -c -w /etc/rc_keymaps/Philips-RCLE013A.toml -t -s rc3``` (replace rc3 with the device you got on the first line)

Installation
------------
1. gcc numeric2mouse.c -o numeric2mouse
2. numeric2mouse [device] (the default device is /dev/input/by-path/platform-ir-receiver*)

Credits
-------
- Author: Gerben Versluis
- Distributed under GPL3 License


