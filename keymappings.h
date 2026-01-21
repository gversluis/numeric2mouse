/*
 * Key name to key code mappings for numeric2mouse
 * Based on Linux kernel input event codes
 * Reference: https://docs.kernel.org/userspace-api/media/rc/rc-tables.html
 */

#ifndef KEY_MAPPINGS_H
#define KEY_MAPPINGS_H

#include <linux/input-event-codes.h>
#include <string.h>

typedef struct {
    const char* name;
    int code;
} key_mapping_entry_t;

static const key_mapping_entry_t key_mappings[] = {
    /* Numeric keys */
    {"KEY_NUMERIC_0", KEY_NUMERIC_0},
    {"KEY_NUMERIC_1", KEY_NUMERIC_1},
    {"KEY_NUMERIC_2", KEY_NUMERIC_2},
    {"KEY_NUMERIC_3", KEY_NUMERIC_3},
    {"KEY_NUMERIC_4", KEY_NUMERIC_4},
    {"KEY_NUMERIC_5", KEY_NUMERIC_5},
    {"KEY_NUMERIC_6", KEY_NUMERIC_6},
    {"KEY_NUMERIC_7", KEY_NUMERIC_7},
    {"KEY_NUMERIC_8", KEY_NUMERIC_8},
    {"KEY_NUMERIC_9", KEY_NUMERIC_9},
    
    /* Movie play control */
    {"KEY_FORWARD", KEY_FORWARD},
    {"KEY_BACK", KEY_BACK},
    {"KEY_FASTFORWARD", KEY_FASTFORWARD},
    {"KEY_REWIND", KEY_REWIND},
    {"KEY_NEXT", KEY_NEXT},
    {"KEY_PREVIOUS", KEY_PREVIOUS},
    {"KEY_AGAIN", KEY_AGAIN},
    {"KEY_PAUSE", KEY_PAUSE},
    {"KEY_PLAY", KEY_PLAY},
    {"KEY_PLAYPAUSE", KEY_PLAYPAUSE},
    {"KEY_STOP", KEY_STOP},
    {"KEY_RECORD", KEY_RECORD},
    {"KEY_CAMERA", KEY_CAMERA},
    {"KEY_SHUFFLE", KEY_SHUFFLE},
    {"KEY_TIME", KEY_TIME},
    {"KEY_TITLE", KEY_TITLE},
    {"KEY_SUBTITLE", KEY_SUBTITLE},
    
    /* Image control */
    {"KEY_BRIGHTNESSDOWN", KEY_BRIGHTNESSDOWN},
    {"KEY_BRIGHTNESSUP", KEY_BRIGHTNESSUP},
    {"KEY_ANGLE", KEY_ANGLE},
    {"KEY_EPG", KEY_EPG},
    {"KEY_TEXT", KEY_TEXT},
    
    /* Audio control */
    {"KEY_AUDIO", KEY_AUDIO},
    {"KEY_MUTE", KEY_MUTE},
    {"KEY_VOLUMEDOWN", KEY_VOLUMEDOWN},
    {"KEY_VOLUMEUP", KEY_VOLUMEUP},
    {"KEY_MODE", KEY_MODE},
    {"KEY_LANGUAGE", KEY_LANGUAGE},
    
    /* Channel control */
    {"KEY_CHANNEL", KEY_CHANNEL},
    {"KEY_CHANNELDOWN", KEY_CHANNELDOWN},
    {"KEY_CHANNELUP", KEY_CHANNELUP},
    {"KEY_DIGITS", KEY_DIGITS},
    {"KEY_SEARCH", KEY_SEARCH},
    
    /* Colored keys */
    {"KEY_BLUE", KEY_BLUE},
    {"KEY_GREEN", KEY_GREEN},
    {"KEY_RED", KEY_RED},
    {"KEY_YELLOW", KEY_YELLOW},
    
    /* Media selection */
    {"KEY_CD", KEY_CD},
    {"KEY_DVD", KEY_DVD},
    {"KEY_EJECTCLOSECD", KEY_EJECTCLOSECD},
    {"KEY_MEDIA", KEY_MEDIA},
    {"KEY_PC", KEY_PC},
    {"KEY_RADIO", KEY_RADIO},
    {"KEY_TV", KEY_TV},
    {"KEY_TV2", KEY_TV2},
    {"KEY_VCR", KEY_VCR},
    {"KEY_VIDEO", KEY_VIDEO},
    
    /* Power control */
    {"KEY_POWER", KEY_POWER},
    {"KEY_POWER2", KEY_POWER2},
    {"KEY_SLEEP", KEY_SLEEP},
    {"KEY_SUSPEND", KEY_SUSPEND},
    
    /* Window control */
    {"KEY_CLEAR", KEY_CLEAR},
    {"KEY_CYCLEWINDOWS", KEY_CYCLEWINDOWS},
    {"KEY_FAVORITES", KEY_FAVORITES},
    {"KEY_MENU", KEY_MENU},
    {"KEY_NEW", KEY_NEW},
    {"KEY_OK", KEY_OK},
    {"KEY_ASPECT_RATIO", KEY_ASPECT_RATIO},
    {"KEY_FULL_SCREEN", KEY_FULL_SCREEN},
    
    /* Navigation keys */
    {"KEY_ESC", KEY_ESC},
    {"KEY_HELP", KEY_HELP},
    {"KEY_HOMEPAGE", KEY_HOMEPAGE},
    {"KEY_INFO", KEY_INFO},
    {"KEY_WWW", KEY_WWW},
    {"KEY_UP", KEY_UP},
    {"KEY_DOWN", KEY_DOWN},
    {"KEY_LEFT", KEY_LEFT},
    {"KEY_RIGHT", KEY_RIGHT},
    
    /* Miscellaneous keys */
    {"KEY_DOT", KEY_DOT},
    {"KEY_FN", KEY_FN},
    
    /* Additional common keys */
    {"KEY_ENTER", KEY_ENTER},
    {"KEY_TAB", KEY_TAB},
    {"KEY_SPACE", KEY_SPACE},
    {"KEY_BACKSPACE", KEY_BACKSPACE},
    {"KEY_DELETE", KEY_DELETE},
    {"KEY_INSERT", KEY_INSERT},
    {"KEY_HOME", KEY_HOME},
    {"KEY_END", KEY_END},
    {"KEY_PAGEUP", KEY_PAGEUP},
    {"KEY_PAGEDOWN", KEY_PAGEDOWN},
    
    /* Modifier keys */
    {"KEY_LEFTCTRL", KEY_LEFTCTRL},
    {"KEY_LEFTSHIFT", KEY_LEFTSHIFT},
    {"KEY_LEFTALT", KEY_LEFTALT},
    {"KEY_LEFTMETA", KEY_LEFTMETA},
    {"KEY_RIGHTCTRL", KEY_RIGHTCTRL},
    {"KEY_RIGHTSHIFT", KEY_RIGHTSHIFT},
    {"KEY_RIGHTALT", KEY_RIGHTALT},
    {"KEY_RIGHTMETA", KEY_RIGHTMETA},
    
    /* Function keys */
    {"KEY_F1", KEY_F1},
    {"KEY_F2", KEY_F2},
    {"KEY_F3", KEY_F3},
    {"KEY_F4", KEY_F4},
    {"KEY_F5", KEY_F5},
    {"KEY_F6", KEY_F6},
    {"KEY_F7", KEY_F7},
    {"KEY_F8", KEY_F8},
    {"KEY_F9", KEY_F9},
    {"KEY_F10", KEY_F10},
    {"KEY_F11", KEY_F11},
    {"KEY_F12", KEY_F12},
    
    /* Letter keys */
    {"KEY_A", KEY_A},
    {"KEY_B", KEY_B},
    {"KEY_C", KEY_C},
    {"KEY_D", KEY_D},
    {"KEY_E", KEY_E},
    {"KEY_F", KEY_F},
    {"KEY_G", KEY_G},
    {"KEY_H", KEY_H},
    {"KEY_I", KEY_I},
    {"KEY_J", KEY_J},
    {"KEY_K", KEY_K},
    {"KEY_L", KEY_L},
    {"KEY_M", KEY_M},
    {"KEY_N", KEY_N},
    {"KEY_O", KEY_O},
    {"KEY_P", KEY_P},
    {"KEY_Q", KEY_Q},
    {"KEY_R", KEY_R},
    {"KEY_S", KEY_S},
    {"KEY_T", KEY_T},
    {"KEY_U", KEY_U},
    {"KEY_V", KEY_V},
    {"KEY_W", KEY_W},
    {"KEY_X", KEY_X},
    {"KEY_Y", KEY_Y},
    {"KEY_Z", KEY_Z},
    
    /* Number row keys */
    {"KEY_0", KEY_0},
    {"KEY_1", KEY_1},
    {"KEY_2", KEY_2},
    {"KEY_3", KEY_3},
    {"KEY_4", KEY_4},
    {"KEY_5", KEY_5},
    {"KEY_6", KEY_6},
    {"KEY_7", KEY_7},
    {"KEY_8", KEY_8},
    {"KEY_9", KEY_9},
    
    /* Special keys */
    {"KEY_CLOSE", KEY_CLOSE},
    {"KEY_KPENTER", KEY_KPENTER},
    {"KEY_KPPLUS", KEY_KPPLUS},
    {"KEY_KPMINUS", KEY_KPMINUS},
    {"KEY_KPASTERISK", KEY_KPASTERISK},
    {"KEY_KPSLASH", KEY_KPSLASH},
    
    /* Mouse buttons */
    {"BTN_LEFT", BTN_LEFT},
    {"BTN_RIGHT", BTN_RIGHT},
    {"BTN_MIDDLE", BTN_MIDDLE},
    
    /* Terminator */
    {NULL, -1}
};

static inline int parse_key_code(const char* key_name) {
    // Try hex code first
    if (strncmp(key_name, "0x", 2) == 0) {
        return (int)strtol(key_name, NULL, 16);
    }
    
    // Try decimal code
    if (key_name[0] >= '0' && key_name[0] <= '9') {
        return atoi(key_name);
    }
    
    // Search in the mapping table
    for (int i = 0; key_mappings[i].name != NULL; i++) {
        if (strcmp(key_name, key_mappings[i].name) == 0) {
            return key_mappings[i].code;
        }
    }
    
    return -1;
}

#endif /* KEY_MAPPINGS_H */

