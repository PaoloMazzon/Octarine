/// \brief This is a 1:1 copy of SDL's constants to allow users to not need to include anything but Octarine
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/// \brief Keyboard keys
typedef enum
{
    OCT_KEY_UNKNOWN = 0,

    /**
     *  \name Usage page 0x07
     *
     *  These values are from usage page 0x07 (USB keyboard page).
     */
    /* @{ */

    OCT_KEY_A = 4,
    OCT_KEY_B = 5,
    OCT_KEY_C = 6,
    OCT_KEY_D = 7,
    OCT_KEY_E = 8,
    OCT_KEY_F = 9,
    OCT_KEY_G = 10,
    OCT_KEY_H = 11,
    OCT_KEY_I = 12,
    OCT_KEY_J = 13,
    OCT_KEY_K = 14,
    OCT_KEY_L = 15,
    OCT_KEY_M = 16,
    OCT_KEY_N = 17,
    OCT_KEY_O = 18,
    OCT_KEY_P = 19,
    OCT_KEY_Q = 20,
    OCT_KEY_R = 21,
    OCT_KEY_S = 22,
    OCT_KEY_T = 23,
    OCT_KEY_U = 24,
    OCT_KEY_V = 25,
    OCT_KEY_W = 26,
    OCT_KEY_X = 27,
    OCT_KEY_Y = 28,
    OCT_KEY_Z = 29,

    OCT_KEY_1 = 30,
    OCT_KEY_2 = 31,
    OCT_KEY_3 = 32,
    OCT_KEY_4 = 33,
    OCT_KEY_5 = 34,
    OCT_KEY_6 = 35,
    OCT_KEY_7 = 36,
    OCT_KEY_8 = 37,
    OCT_KEY_9 = 38,
    OCT_KEY_0 = 39,

    OCT_KEY_RETURN = 40,
    OCT_KEY_ESCAPE = 41,
    OCT_KEY_BACKSPACE = 42,
    OCT_KEY_TAB = 43,
    OCT_KEY_SPACE = 44,

    OCT_KEY_MINUS = 45,
    OCT_KEY_EQUALS = 46,
    OCT_KEY_LEFTBRACKET = 47,
    OCT_KEY_RIGHTBRACKET = 48,
    OCT_KEY_BACKSLASH = 49, /**< Located at the lower left of the return
                                  *   key on ISO keyboards and at the right end
                                  *   of the QWERTY row on ANSI keyboards.
                                  *   Produces REVERSE SOLIDUS (backslash) and
                                  *   VERTICAL LINE in a US layout, REVERSE
                                  *   SOLIDUS and VERTICAL LINE in a UK Mac
                                  *   layout, NUMBER SIGN and TILDE in a UK
                                  *   Windows layout, DOLLAR SIGN and POUND SIGN
                                  *   in a Swiss German layout, NUMBER SIGN and
                                  *   APOSTROPHE in a German layout, GRAVE
                                  *   ACCENT and POUND SIGN in a French Mac
                                  *   layout, and ASTERISK and MICRO SIGN in a
                                  *   French Windows layout.
                                  */
    OCT_KEY_NONUSHASH = 50, /**< ISO USB keyboards actually use this code
                                  *   instead of 49 for the same key, but all
                                  *   OSes I've seen treat the two codes
                                  *   identically. So, as an implementor, unless
                                  *   your keyboard generates both of those
                                  *   codes and your OS treats them differently,
                                  *   you should generate OCT_KEY_BACKSLASH
                                  *   instead of this code. As a user, you
                                  *   should not rely on this code because SDL
                                  *   will never generate it with most (all?)
                                  *   keyboards.
                                  */
    OCT_KEY_SEMICOLON = 51,
    OCT_KEY_APOSTROPHE = 52,
    OCT_KEY_GRAVE = 53, /**< Located in the top left corner (on both ANSI
                              *   and ISO keyboards). Produces GRAVE ACCENT and
                              *   TILDE in a US Windows layout and in US and UK
                              *   Mac layouts on ANSI keyboards, GRAVE ACCENT
                              *   and NOT SIGN in a UK Windows layout, SECTION
                              *   SIGN and PLUS-MINUS SIGN in US and UK Mac
                              *   layouts on ISO keyboards, SECTION SIGN and
                              *   DEGREE SIGN in a Swiss German layout (Mac:
                              *   only on ISO keyboards), CIRCUMFLEX ACCENT and
                              *   DEGREE SIGN in a German layout (Mac: only on
                              *   ISO keyboards), SUPERSCRIPT TWO and TILDE in a
                              *   French Windows layout, COMMERCIAL AT and
                              *   NUMBER SIGN in a French Mac layout on ISO
                              *   keyboards, and LESS-THAN SIGN and GREATER-THAN
                              *   SIGN in a Swiss German, German, or French Mac
                              *   layout on ANSI keyboards.
                              */
    OCT_KEY_COMMA = 54,
    OCT_KEY_PERIOD = 55,
    OCT_KEY_SLASH = 56,

    OCT_KEY_CAPSLOCK = 57,

    OCT_KEY_F1 = 58,
    OCT_KEY_F2 = 59,
    OCT_KEY_F3 = 60,
    OCT_KEY_F4 = 61,
    OCT_KEY_F5 = 62,
    OCT_KEY_F6 = 63,
    OCT_KEY_F7 = 64,
    OCT_KEY_F8 = 65,
    OCT_KEY_F9 = 66,
    OCT_KEY_F10 = 67,
    OCT_KEY_F11 = 68,
    OCT_KEY_F12 = 69,

    OCT_KEY_PRINTSCREEN = 70,
    OCT_KEY_SCROLLLOCK = 71,
    OCT_KEY_PAUSE = 72,
    OCT_KEY_INSERT = 73, /**< insert on PC, help on some Mac keyboards (but
                                   does send code 73, not 117) */
    OCT_KEY_HOME = 74,
    OCT_KEY_PAGEUP = 75,
    OCT_KEY_DELETE = 76,
    OCT_KEY_END = 77,
    OCT_KEY_PAGEDOWN = 78,
    OCT_KEY_RIGHT = 79,
    OCT_KEY_LEFT = 80,
    OCT_KEY_DOWN = 81,
    OCT_KEY_UP = 82,

    OCT_KEY_NUMLOCKCLEAR = 83, /**< num lock on PC, clear on Mac keyboards
                                     */
    OCT_KEY_KP_DIVIDE = 84,
    OCT_KEY_KP_MULTIPLY = 85,
    OCT_KEY_KP_MINUS = 86,
    OCT_KEY_KP_PLUS = 87,
    OCT_KEY_KP_ENTER = 88,
    OCT_KEY_KP_1 = 89,
    OCT_KEY_KP_2 = 90,
    OCT_KEY_KP_3 = 91,
    OCT_KEY_KP_4 = 92,
    OCT_KEY_KP_5 = 93,
    OCT_KEY_KP_6 = 94,
    OCT_KEY_KP_7 = 95,
    OCT_KEY_KP_8 = 96,
    OCT_KEY_KP_9 = 97,
    OCT_KEY_KP_0 = 98,
    OCT_KEY_KP_PERIOD = 99,

    OCT_KEY_NONUSBACKSLASH = 100, /**< This is the additional key that ISO
                                        *   keyboards have over ANSI ones,
                                        *   located between left shift and Y.
                                        *   Produces GRAVE ACCENT and TILDE in a
                                        *   US or UK Mac layout, REVERSE SOLIDUS
                                        *   (backslash) and VERTICAL LINE in a
                                        *   US or UK Windows layout, and
                                        *   LESS-THAN SIGN and GREATER-THAN SIGN
                                        *   in a Swiss German, German, or French
                                        *   layout. */
    OCT_KEY_APPLICATION = 101, /**< windows contextual menu, compose */
    OCT_KEY_POWER = 102, /**< The USB document says this is a status flag,
                               *   not a physical key - but some Mac keyboards
                               *   do have a power key. */
    OCT_KEY_KP_EQUALS = 103,
    OCT_KEY_F13 = 104,
    OCT_KEY_F14 = 105,
    OCT_KEY_F15 = 106,
    OCT_KEY_F16 = 107,
    OCT_KEY_F17 = 108,
    OCT_KEY_F18 = 109,
    OCT_KEY_F19 = 110,
    OCT_KEY_F20 = 111,
    OCT_KEY_F21 = 112,
    OCT_KEY_F22 = 113,
    OCT_KEY_F23 = 114,
    OCT_KEY_F24 = 115,
    OCT_KEY_EXECUTE = 116,
    OCT_KEY_HELP = 117,    /**< AL Integrated Help Center */
    OCT_KEY_MENU = 118,    /**< Menu (show menu) */
    OCT_KEY_SELECT = 119,
    OCT_KEY_STOP = 120,    /**< AC Stop */
    OCT_KEY_AGAIN = 121,   /**< AC Redo/Repeat */
    OCT_KEY_UNDO = 122,    /**< AC Undo */
    OCT_KEY_CUT = 123,     /**< AC Cut */
    OCT_KEY_COPY = 124,    /**< AC Copy */
    OCT_KEY_PASTE = 125,   /**< AC Paste */
    OCT_KEY_FIND = 126,    /**< AC Find */
    OCT_KEY_MUTE = 127,
    OCT_KEY_VOLUMEUP = 128,
    OCT_KEY_VOLUMEDOWN = 129,
/* not sure whether there's a reason to enable these */
/*     OCT_KEY_LOCKINGCAPSLOCK = 130,  */
/*     OCT_KEY_LOCKINGNUMLOCK = 131, */
/*     OCT_KEY_LOCKINGSCROLLLOCK = 132, */
    OCT_KEY_KP_COMMA = 133,
    OCT_KEY_KP_EQUALSAS400 = 134,

    OCT_KEY_INTERNATIONAL1 = 135, /**< used on Asian keyboards, see
                                            footnotes in USB doc */
    OCT_KEY_INTERNATIONAL2 = 136,
    OCT_KEY_INTERNATIONAL3 = 137, /**< Yen */
    OCT_KEY_INTERNATIONAL4 = 138,
    OCT_KEY_INTERNATIONAL5 = 139,
    OCT_KEY_INTERNATIONAL6 = 140,
    OCT_KEY_INTERNATIONAL7 = 141,
    OCT_KEY_INTERNATIONAL8 = 142,
    OCT_KEY_INTERNATIONAL9 = 143,
    OCT_KEY_LANG1 = 144, /**< Hangul/English toggle */
    OCT_KEY_LANG2 = 145, /**< Hanja conversion */
    OCT_KEY_LANG3 = 146, /**< Katakana */
    OCT_KEY_LANG4 = 147, /**< Hiragana */
    OCT_KEY_LANG5 = 148, /**< Zenkaku/Hankaku */
    OCT_KEY_LANG6 = 149, /**< reserved */
    OCT_KEY_LANG7 = 150, /**< reserved */
    OCT_KEY_LANG8 = 151, /**< reserved */
    OCT_KEY_LANG9 = 152, /**< reserved */

    OCT_KEY_ALTERASE = 153,    /**< Erase-Eaze */
    OCT_KEY_SYSREQ = 154,
    OCT_KEY_CANCEL = 155,      /**< AC Cancel */
    OCT_KEY_CLEAR = 156,
    OCT_KEY_PRIOR = 157,
    OCT_KEY_RETURN2 = 158,
    OCT_KEY_SEPARATOR = 159,
    OCT_KEY_OUT = 160,
    OCT_KEY_OPER = 161,
    OCT_KEY_CLEARAGAIN = 162,
    OCT_KEY_CRSEL = 163,
    OCT_KEY_EXSEL = 164,

    OCT_KEY_KP_00 = 176,
    OCT_KEY_KP_000 = 177,
    OCT_KEY_THOUSANDSSEPARATOR = 178,
    OCT_KEY_DECIMALSEPARATOR = 179,
    OCT_KEY_CURRENCYUNIT = 180,
    OCT_KEY_CURRENCYSUBUNIT = 181,
    OCT_KEY_KP_LEFTPAREN = 182,
    OCT_KEY_KP_RIGHTPAREN = 183,
    OCT_KEY_KP_LEFTBRACE = 184,
    OCT_KEY_KP_RIGHTBRACE = 185,
    OCT_KEY_KP_TAB = 186,
    OCT_KEY_KP_BACKSPACE = 187,
    OCT_KEY_KP_A = 188,
    OCT_KEY_KP_B = 189,
    OCT_KEY_KP_C = 190,
    OCT_KEY_KP_D = 191,
    OCT_KEY_KP_E = 192,
    OCT_KEY_KP_F = 193,
    OCT_KEY_KP_XOR = 194,
    OCT_KEY_KP_POWER = 195,
    OCT_KEY_KP_PERCENT = 196,
    OCT_KEY_KP_LESS = 197,
    OCT_KEY_KP_GREATER = 198,
    OCT_KEY_KP_AMPERSAND = 199,
    OCT_KEY_KP_DBLAMPERSAND = 200,
    OCT_KEY_KP_VERTICALBAR = 201,
    OCT_KEY_KP_DBLVERTICALBAR = 202,
    OCT_KEY_KP_COLON = 203,
    OCT_KEY_KP_HASH = 204,
    OCT_KEY_KP_SPACE = 205,
    OCT_KEY_KP_AT = 206,
    OCT_KEY_KP_EXCLAM = 207,
    OCT_KEY_KP_MEMSTORE = 208,
    OCT_KEY_KP_MEMRECALL = 209,
    OCT_KEY_KP_MEMCLEAR = 210,
    OCT_KEY_KP_MEMADD = 211,
    OCT_KEY_KP_MEMSUBTRACT = 212,
    OCT_KEY_KP_MEMMULTIPLY = 213,
    OCT_KEY_KP_MEMDIVIDE = 214,
    OCT_KEY_KP_PLUSMINUS = 215,
    OCT_KEY_KP_CLEAR = 216,
    OCT_KEY_KP_CLEARENTRY = 217,
    OCT_KEY_KP_BINARY = 218,
    OCT_KEY_KP_OCTAL = 219,
    OCT_KEY_KP_DECIMAL = 220,
    OCT_KEY_KP_HEXADECIMAL = 221,

    OCT_KEY_LCTRL = 224,
    OCT_KEY_LSHIFT = 225,
    OCT_KEY_LALT = 226, /**< alt, option */
    OCT_KEY_LGUI = 227, /**< windows, command (apple), meta */
    OCT_KEY_RCTRL = 228,
    OCT_KEY_RSHIFT = 229,
    OCT_KEY_RALT = 230, /**< alt gr, option */
    OCT_KEY_RGUI = 231, /**< windows, command (apple), meta */

    OCT_KEY_MODE = 257,    /**< I'm not sure if this is really not covered
                                 *   by any of the above, but since there's a
                                 *   special KMOD_MODE for it I'm adding it here
                                 */

    /* @} *//* Usage page 0x07 */

    /**
     *  \name Usage page 0x0C
     *
     *  These values are mapped from usage page 0x0C (USB consumer page).
     *  See https://usb.org/sites/default/files/hut1_2.pdf
     *
     *  There are way more keys in the spec than we can represent in the
     *  current scancode range, so pick the ones that commonly come up in
     *  real world usage.
     */
    /* @{ */

    OCT_KEY_AUDIONEXT = 258,
    OCT_KEY_AUDIOPREV = 259,
    OCT_KEY_AUDIOSTOP = 260,
    OCT_KEY_AUDIOPLAY = 261,
    OCT_KEY_AUDIOMUTE = 262,
    OCT_KEY_MEDIASELECT = 263,
    OCT_KEY_WWW = 264,             /**< AL Internet Browser */
    OCT_KEY_MAIL = 265,
    OCT_KEY_CALCULATOR = 266,      /**< AL Calculator */
    OCT_KEY_COMPUTER = 267,
    OCT_KEY_AC_SEARCH = 268,       /**< AC Search */
    OCT_KEY_AC_HOME = 269,         /**< AC Home */
    OCT_KEY_AC_BACK = 270,         /**< AC Back */
    OCT_KEY_AC_FORWARD = 271,      /**< AC Forward */
    OCT_KEY_AC_STOP = 272,         /**< AC Stop */
    OCT_KEY_AC_REFRESH = 273,      /**< AC Refresh */
    OCT_KEY_AC_BOOKMARKS = 274,    /**< AC Bookmarks */

    /* @} *//* Usage page 0x0C */

    /**
     *  \name Walther keys
     *
     *  These are values that Christian Walther added (for mac keyboard?).
     */
    /* @{ */

    OCT_KEY_BRIGHTNESSDOWN = 275,
    OCT_KEY_BRIGHTNESSUP = 276,
    OCT_KEY_DISPLAYSWITCH = 277, /**< display mirroring/dual display
                                           switch, video mode switch */
    OCT_KEY_KBDILLUMTOGGLE = 278,
    OCT_KEY_KBDILLUMDOWN = 279,
    OCT_KEY_KBDILLUMUP = 280,
    OCT_KEY_EJECT = 281,
    OCT_KEY_SLEEP = 282,           /**< SC System Sleep */

    OCT_KEY_APP1 = 283,
    OCT_KEY_APP2 = 284,

    /* @} *//* Walther keys */

    /**
     *  \name Usage page 0x0C (additional media keys)
     *
     *  These values are mapped from usage page 0x0C (USB consumer page).
     */
    /* @{ */

    OCT_KEY_AUDIOREWIND = 285,
    OCT_KEY_AUDIOFASTFORWARD = 286,

    /* @} *//* Usage page 0x0C (additional media keys) */

    /**
     *  \name Mobile keys
     *
     *  These are values that are often used on mobile phones.
     */
    /* @{ */

    OCT_KEY_SOFTLEFT = 287, /**< Usually situated below the display on phones and
                                      used as a multi-function feature key for selecting
                                      a software defined function shown on the bottom left
                                      of the display. */
    OCT_KEY_SOFTRIGHT = 288, /**< Usually situated below the display on phones and
                                       used as a multi-function feature key for selecting
                                       a software defined function shown on the bottom right
                                       of the display. */
    OCT_KEY_CALL = 289, /**< Used for accepting phone calls. */
    OCT_KEY_ENDCALL = 290, /**< Used for rejecting phone calls. */

    /* @} *//* Mobile keys */

    /* Add any other keys here. */
    OCT_KEY_COUNT = 512;
} Oct_Key;

/// \brief Gamepad buttons
typedef enum
{
    OCT_GAMEPAD_BUTTON_INVALID = -1,
    OCT_GAMEPAD_BUTTON_A,
    OCT_GAMEPAD_BUTTON_B,
    OCT_GAMEPAD_BUTTON_X,
    OCT_GAMEPAD_BUTTON_Y,
    OCT_GAMEPAD_BUTTON_BACK,
    OCT_GAMEPAD_BUTTON_GUIDE,
    OCT_GAMEPAD_BUTTON_START,
    OCT_GAMEPAD_BUTTON_LEFTSTICK,
    OCT_GAMEPAD_BUTTON_RIGHTSTICK,
    OCT_GAMEPAD_BUTTON_LEFTSHOULDER,
    OCT_GAMEPAD_BUTTON_RIGHTSHOULDER,
    OCT_GAMEPAD_BUTTON_DPAD_UP,
    OCT_GAMEPAD_BUTTON_DPAD_DOWN,
    OCT_GAMEPAD_BUTTON_DPAD_LEFT,
    OCT_GAMEPAD_BUTTON_DPAD_RIGHT,
    OCT_GAMEPAD_BUTTON_MISC1,    /* Xbox Series X share button, PS5 microphone button, Nintendo Switch Pro capture button, Amazon Luna microphone button */
    OCT_GAMEPAD_BUTTON_PADDLE1,  /* Xbox Elite paddle P1 (upper left, facing the back) */
    OCT_GAMEPAD_BUTTON_PADDLE2,  /* Xbox Elite paddle P3 (upper right, facing the back) */
    OCT_GAMEPAD_BUTTON_PADDLE3,  /* Xbox Elite paddle P2 (lower left, facing the back) */
    OCT_GAMEPAD_BUTTON_PADDLE4,  /* Xbox Elite paddle P4 (lower right, facing the back) */
    OCT_GAMEPAD_BUTTON_TOUCHPAD, /* PS4/PS5 touchpad button */
    OCT_GAMEPAD_BUTTON_MAX = 32
} Oct_GamepadButton;

/// \brief Mouse buttons
typedef enum {
    OCT_MOUSE_BUTTON_INVALID = -1,
    OCT_MOUSE_BUTTON_LEFT = 0,
    OCT_MOUSE_BUTTON_RIGHT = 1,
    OCT_MOUSE_BUTTON_MIDDLE = 2,
    OCT_MOUSE_BUTTON_MAX = 8
} Oct_MouseButton;

#ifdef __cplusplus
};
#endif