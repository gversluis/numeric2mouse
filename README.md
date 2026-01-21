# numeric2mouse

Introduction
------------
I wrote a daemon to map ir-keytable keys to mouse movements and combination keys on my Raspberry PI (though it should work on any Linux device with lirc in the kernel).

It basically:
- Creates a new user input
- Intercepts the old ir receiver input (an other keytable input should work too)
- If it is a numeric keypad number map it to mouse movement
- If it is a predefined key map it to combination keys (i.e. KEY_CLOSE to ALT+F4)
- Pass through everything else

I wrote this for my own use so it is not tested except on Wayland on my Raspberry Pi 4.

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
6. From there on write pairs of keycode and key you want to map per line, i.e. ```0x0C = "KEY_CLOSE"``` and ```0x02 = "KEY_NUMERIC_2"```. The keys can be found at https://github.com/torvalds/linux/blob/master/include/uapi/linux/input-event-codes.h
7. Add ```* * Philips_RCLE013A.toml``` to ```/etc/rc_maps.cfg``` (replace "Philips_RCLE013A" with your own remote name)
8. Load and test by running ```ir-keytable -c -w /etc/rc_keymaps/Philips-RCLE013A.toml -t -s rc3``` (replace rc3 with the device you got on the first line)

Installation
------------
1. Install libyaml (apt-get install libyaml-dev)
2. make && sudo make install
3. Put your YAML config in /etc/numeric2mouse.yaml (layout of file is explained later)
3. numeric2mouse [device] (the default device is /dev/input/by-path/platform-ir-receiver*)
4. Start automatically on boot by adding the following line to your crontab: @reboot \<path to numeric2mouse\> *

* I tried starting it as a systemd service but when starting through systemd it could not access /dev/uinput.

## Configuration File Format

The configuration file (`/etc/numeric2mouse.yaml`) has the following structure:

```yaml
mappings:
  - key: KEY_NAME
    action:
      type: ACTION_TYPE
      # action-specific parameters
```

### Action Types

#### 1. `move_mouse`

Moves the mouse cursor. The x and y values indicate direction (-1, 0, or 1), and the actual speed is calculated based on key repeat state.

```yaml
- key: KEY_NUMERIC_8
  action:
    type: move_mouse
    x: 0      # -1 (left), 0 (none), 1 (right)
    y: 1      # -1 (down), 0 (none), 1 (up)
```

#### 2. `key_combination`

Sends a combination of key presses (like Ctrl+C or Alt+F4).

```yaml
- key: KEY_CLOSE
  action:
    type: key_combination
    keys:
      - KEY_LEFTALT
      - KEY_F4
```

Keys are pressed in order, then released in reverse order (except the last key, which is handled specially for proper event synchronization).

#### 3. `execute`

Executes a shell command. Commands run asynchronously (forked process) and only trigger on key press, not on key repeat or release.

```yaml
- key: KEY_RADIO
  action:
    type: execute
    command: sudo -u username /home/username/radio.sh on
    maxPerSecond: 10    # Optional: rate limit (0 or omit for unlimited)
```

**Parameters:**
- `command`: The command to execute (as if typed in terminal)
- `maxPerSecond`: (Optional) Maximum executions per second
  - `0` or omitted = unlimited
  - Prevents accidental rapid-fire execution from key bouncing
  - Uses a sliding 1-second window

## Supported Key Names

The `parse_key_code()` function currently supports these key names:

- **Numeric keys**: `KEY_NUMERIC_0` through `KEY_NUMERIC_9`
- **Modifier keys**: `KEY_LEFTALT`, `KEY_LEFTCTRL`, `KEY_LEFTSHIFT`, `KEY_LEFTMETA`, `KEY_RIGHTSHIFT`, `KEY_RIGHTMETA`
- **Function keys**: `KEY_F4` (add more as needed)
- **Letter keys**: `KEY_A`, `KEY_C`, `KEY_V`, `KEY_X`, `KEY_Z` (add more as needed)
- **Special keys**: `KEY_CLOSE`, `KEY_RADIO`, `KEY_POWER`, `KEY_SLEEP`, `KEY_MUTE`, `KEY_VOLUMEUP`, `KEY_VOLUMEDOWN`
- **Hex codes**: Any key code in hex format like `0x1c`

### Finding Key Codes

To find the key code for your device:

```sudo evtest /dev/input/by-path/platform-ir-receiver@11-event
```

Press keys and note their codes. You can then either:
1. Add the key name to `parse_key_code()` in the C code
2. Use the hex code directly in your YAML (e.g., `0x4a`)

## Debugging

Enable verbose output by changing the defines at the top of `numeric2mouse.c`:

```c
#define DEBUG 1
#define VERBOSE 1
```

This will show:
- Which keys are being pressed
- Key codes received
- Actions being executed

## Limitations

- Maximum 256 mappings (`MAX_MAPPINGS`)
- Maximum 10 keys per combination (`MAX_KEYS`)
- Maximum 512 characters per execute command
- Execute rate limiting uses a 100-slot circular buffer for timing
- YAML parser is simple and may not handle complex YAML features
- Keys not in the configuration are passed through unchanged

## Troubleshooting

**Config file not loading:**
- Check that `/etc/numeric2mouse.yaml` exists and is readable
- Verify YAML syntax with `yamllint /etc/numeric2mouse.yaml`

**Keys not working:**
- Run with `DEBUG=1` to see which codes are received
- Verify key names match those in `parse_key_code()`
- Check that the key code matches what `evtest` shows

**Execute commands not running:**
- Check command syntax - test it manually in a terminal first
- Verify user permissions (especially with `sudo`)
- Check system logs: `journalctl -f` while testing
- Enable `VERBOSE=1` to see execution attempts
- For GUI apps, ensure `DISPLAY` environment variable is set

**Rate limiting too strict/loose:**
- Adjust `maxPerSecond` value in your YAML
- Set to `0` or omit for no rate limiting
- Rate limit is per key mapping, not global

**Build errors:**
- Ensure `libyaml-dev` is installed
- Check that you have proper kernel headers for `linux/uinput.h`

## Extending the Code

### Adding New Action Types

1. Add to the `action_type_t` enum
2. Add a new union member to `action_t`
3. Parse the new type in `load_config()`
4. Handle the new type in the main event loop

The `execute` action is implemented using this pattern - check the source for reference.

Credits
-------
- Author: Gerben Versluis
- Distributed under GPL3 License


