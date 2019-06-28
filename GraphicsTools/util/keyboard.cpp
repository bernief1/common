// ===================
// common/keyboard.cpp
// ===================

#include "common/common.h"

#if PLATFORM_PC

#include "crc.h" // for PrintChangedKeyboardState
#include "keyboard.h"

Keyboard::Keyboard() : m_exitCallback(nullptr)
{
	memset(this, 0, sizeof(*this));
}

void Keyboard::Update()
{
	memcpy(m_prevState, m_currState, sizeof(m_prevState));
	GetKeyboardState(m_currState);
	if (IsKeyDown(VK_ESCAPE)) {
		if (m_exitCallback)
			m_exitCallback();
		exit(0);
	}
}

void Keyboard::PrintChangedKeyboardState() const
{
	class VitualKey
	{
	public:
		const char* m_vkString;
		int m_vkCode;
	};
	#ifndef STRING
	#define STRING(x) #x
	#define STRING_DEFINED
	#endif
	const VitualKey virtualKeys[] = // from Windows Kits\8.1\Include\um\WinUser.h
	{
		{ STRING(VK_LBUTTON            ), 0x01 },
		{ STRING(VK_RBUTTON            ), 0x02 },
		{ STRING(VK_CANCEL             ), 0x03 },
		{ STRING(VK_MBUTTON            ), 0x04 },
		{ STRING(VK_XBUTTON1           ), 0x05 },
		{ STRING(VK_XBUTTON2           ), 0x06 },
		{ STRING(VK_BACK               ), 0x08 },
		{ STRING(VK_TAB                ), 0x09 },
		{ STRING(VK_CLEAR              ), 0x0C },
		{ STRING(VK_RETURN             ), 0x0D },
		{ STRING(VK_SHIFT              ), 0x10 },
		{ STRING(VK_CONTROL            ), 0x11 },
		{ STRING(VK_MENU               ), 0x12 },
		{ STRING(VK_PAUSE              ), 0x13 },
		{ STRING(VK_CAPITAL            ), 0x14 },
		{ STRING(VK_ESCAPE             ), 0x1B },
		{ STRING(VK_CONVERT            ), 0x1C },
		{ STRING(VK_NONCONVERT         ), 0x1D },
		{ STRING(VK_ACCEPT             ), 0x1E },
		{ STRING(VK_MODECHANGE         ), 0x1F },
		{ STRING(VK_SPACE              ), 0x20 }, // ' '
		{ STRING(VK_PRIOR              ), 0x21 }, // '!'
		{ STRING(VK_NEXT               ), 0x22 }, // '\"'
		{ STRING(VK_END                ), 0x23 }, // '#'
		{ STRING(VK_HOME               ), 0x24 }, // '$'
		{ STRING(VK_LEFT               ), 0x25 }, // '%'
		{ STRING(VK_UP                 ), 0x26 }, // '&'
		{ STRING(VK_RIGHT              ), 0x27 }, // '''
		{ STRING(VK_DOWN               ), 0x28 }, // '('
		{ STRING(VK_SELECT             ), 0x29 }, // ')'
		{ STRING(VK_PRINT              ), 0x2A }, // '*'
		{ STRING(VK_EXECUTE            ), 0x2B }, // '+'
		{ STRING(VK_SNAPSHOT           ), 0x2C }, // ','
		{ STRING(VK_INSERT             ), 0x2D }, // '-'
		{ STRING(VK_DELETE             ), 0x2E }, // '.'
		{ STRING(VK_HELP               ), 0x2F }, // '/'
		{ STRING(VK_LWIN               ), 0x5B },
		{ STRING(VK_RWIN               ), 0x5C },
		{ STRING(VK_APPS               ), 0x5D },
		{ STRING(VK_SLEEP              ), 0x5F },
		{ STRING(VK_NUMPAD0            ), 0x60 },
		{ STRING(VK_NUMPAD1            ), 0x61 },
		{ STRING(VK_NUMPAD2            ), 0x62 },
		{ STRING(VK_NUMPAD3            ), 0x63 },
		{ STRING(VK_NUMPAD4            ), 0x64 },
		{ STRING(VK_NUMPAD5            ), 0x65 },
		{ STRING(VK_NUMPAD6            ), 0x66 },
		{ STRING(VK_NUMPAD7            ), 0x67 },
		{ STRING(VK_NUMPAD8            ), 0x68 },
		{ STRING(VK_NUMPAD9            ), 0x69 },
		{ STRING(VK_MULTIPLY           ), 0x6A },
		{ STRING(VK_ADD                ), 0x6B },
		{ STRING(VK_SEPARATOR          ), 0x6C },
		{ STRING(VK_SUBTRACT           ), 0x6D },
		{ STRING(VK_DECIMAL            ), 0x6E },
		{ STRING(VK_DIVIDE             ), 0x6F },
		{ STRING(VK_F1                 ), 0x70 },
		{ STRING(VK_F2                 ), 0x71 },
		{ STRING(VK_F3                 ), 0x72 },
		{ STRING(VK_F4                 ), 0x73 },
		{ STRING(VK_F5                 ), 0x74 },
		{ STRING(VK_F6                 ), 0x75 },
		{ STRING(VK_F7                 ), 0x76 },
		{ STRING(VK_F8                 ), 0x77 },
		{ STRING(VK_F9                 ), 0x78 },
		{ STRING(VK_F10                ), 0x79 },
		{ STRING(VK_F11                ), 0x7A },
		{ STRING(VK_F12                ), 0x7B },
		{ STRING(VK_F13                ), 0x7C },
		{ STRING(VK_F14                ), 0x7D },
		{ STRING(VK_F15                ), 0x7E },
		{ STRING(VK_F16                ), 0x7F },
		{ STRING(VK_F17                ), 0x80 },
		{ STRING(VK_F18                ), 0x81 },
		{ STRING(VK_F19                ), 0x82 },
		{ STRING(VK_F20                ), 0x83 },
		{ STRING(VK_F21                ), 0x84 },
		{ STRING(VK_F22                ), 0x85 },
		{ STRING(VK_F23                ), 0x86 },
		{ STRING(VK_F24                ), 0x87 },
		{ STRING(VK_NUMLOCK            ), 0x90 },
		{ STRING(VK_SCROLL             ), 0x91 },
		{ STRING(VK_LSHIFT             ), 0xA0 },
		{ STRING(VK_RSHIFT             ), 0xA1 },
		{ STRING(VK_LCONTROL           ), 0xA2 },
		{ STRING(VK_RCONTROL           ), 0xA3 },
		{ STRING(VK_LMENU              ), 0xA4 },
		{ STRING(VK_RMENU              ), 0xA5 },
		{ STRING(VK_BROWSER_BACK       ), 0xA6 },
		{ STRING(VK_BROWSER_FORWARD    ), 0xA7 },
		{ STRING(VK_BROWSER_REFRESH    ), 0xA8 },
		{ STRING(VK_BROWSER_STOP       ), 0xA9 },
		{ STRING(VK_BROWSER_SEARCH     ), 0xAA },
		{ STRING(VK_BROWSER_FAVORITES  ), 0xAB },
		{ STRING(VK_BROWSER_HOME       ), 0xAC },
		{ STRING(VK_VOLUME_MUTE        ), 0xAD },
		{ STRING(VK_VOLUME_DOWN        ), 0xAE },
		{ STRING(VK_VOLUME_UP          ), 0xAF },
		{ STRING(VK_MEDIA_NEXT_TRACK   ), 0xB0 },
		{ STRING(VK_MEDIA_PREV_TRACK   ), 0xB1 },
		{ STRING(VK_MEDIA_STOP         ), 0xB2 },
		{ STRING(VK_MEDIA_PLAY_PAUSE   ), 0xB3 },
		{ STRING(VK_LAUNCH_MAIL        ), 0xB4 },
		{ STRING(VK_LAUNCH_MEDIA_SELECT), 0xB5 },
		{ STRING(VK_LAUNCH_APP1        ), 0xB6 },
		{ STRING(VK_LAUNCH_APP2        ), 0xB7 },
		{ STRING(VK_OEM_1              ), 0xBA }, // ';:' for US
		{ STRING(VK_OEM_PLUS           ), 0xBB }, // '+' any country
		{ STRING(VK_OEM_COMMA          ), 0xBC }, // ',' any country
		{ STRING(VK_OEM_MINUS          ), 0xBD }, // '-' any country
		{ STRING(VK_OEM_PERIOD         ), 0xBE }, // '.' any country
		{ STRING(VK_OEM_2              ), 0xBF }, // '/?' for US
		{ STRING(VK_OEM_3              ), 0xC0 }, // '`~' for US
		{ STRING(VK_OEM_4              ), 0xDB }, // '[{' for US
		{ STRING(VK_OEM_5              ), 0xDC }, // '\|' for US
		{ STRING(VK_OEM_6              ), 0xDD }, // ']}' for US
		{ STRING(VK_OEM_7              ), 0xDE }, // ''"' for US
		{ STRING(VK_OEM_8              ), 0xDF },
		{ STRING(VK_PROCESSKEY         ), 0xE5 },
		{ STRING(VK_ICO_CLEAR          ), 0xE6 },
		{ STRING(VK_PACKET             ), 0xE7 },
		{ STRING(VK_ATTN               ), 0xF6 },
		{ STRING(VK_CRSEL              ), 0xF7 },
		{ STRING(VK_EXSEL              ), 0xF8 },
		{ STRING(VK_EREOF              ), 0xF9 },
		{ STRING(VK_PLAY               ), 0xFA },
		{ STRING(VK_ZOOM               ), 0xFB },
		{ STRING(VK_NONAME             ), 0xFC },
		{ STRING(VK_PA1                ), 0xFD },
		{ STRING(VK_OEM_CLEAR          ), 0xFE },
	};
	#ifdef STRING_DEFINED
	#undef STRING
	#endif
	static uint64 prevStateHash = 0;
	uint8 bits[32];
	memset(bits, 0, sizeof(bits));
	char state[512] = "";
	for (uint32 k = 0; k < 256; k++) {
		if (m_currState[k] & 0x80) {
			const char* s = "???";
			for (uint32 i = 0; i < countof(virtualKeys); i++) {
				if (virtualKeys[i].m_vkCode == k) {
					s = virtualKeys[i].m_vkString;
					break;
				}
			}
			char temp[32];
			if (k >= 32 && k < 127) // printable ASCII
				sprintf(temp, "key_0x%02X(%s = '%s%c')", k, s, (k == '\\' || k == '\"') ? "\\" : "", k);
			else
				sprintf(temp, "key_0x%02X(%s)", k, s);
			if (state[0])
				strcat(state, " ");
			strcat(state, temp);
			bits[k>>3] |= BIT(k&7);
		}
	}
	const uint64 currStateHash = Crc64(bits, sizeof(bits));
	if (prevStateHash != currStateHash) {
		prevStateHash = currStateHash;
		printf("keyboard state = %s\n", state);
	}
}

bool Keyboard::IsKeyDownModifier(uint32 modifiers) const
{
	if (!!(modifiers & MODIFIER_SHIFT) != IsShiftDown() && !(modifiers & MODIFIER_IGNORE_SHIFT))
		return false;
	else if (!!(modifiers & MODIFIER_CONTROL) != IsControlDown() && !(modifiers & MODIFIER_IGNORE_CONTROL))
		return false;
	else if (!!(modifiers & MODIFIER_ALT) != IsAltDown() && !(modifiers & MODIFIER_IGNORE_ALT))
		return false;
	else
		return true;
}

bool Keyboard::IsShiftDown() const
{
	return !!(m_currState[VK_SHIFT] & 0x80);
}

bool Keyboard::IsControlDown() const
{
	return !!(m_currState[VK_LCONTROL] & 0x80) || !!(m_currState[VK_RCONTROL] & 0x80);
}

bool Keyboard::IsAltDown() const
{
	// VK_LMENU is really VK_LALT
	// VK_RMENU is really VK_RALT
	return !!(m_currState[VK_LMENU] & 0x80) || !!(m_currState[VK_RMENU] & 0x80);
}

uint32 Keyboard::GetModifiers() const
{
	uint32 modifiers = 0;
	if (IsShiftDown())
		modifiers |= MODIFIER_SHIFT;
	if (IsControlDown())
		modifiers |= MODIFIER_CONTROL;
	if (IsAltDown())
		modifiers |= MODIFIER_ALT;
	return modifiers;
}

#endif // PLATFORM_PC