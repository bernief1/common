// =================
// common/keyboard.h
// =================

#ifndef _INCLUDE_COMMON_KEYBOARD_H_
#define _INCLUDE_COMMON_KEYBOARD_H_

#include "common/common.h"

#if PLATFORM_PC

// a few extra defines ..
#define VK_LALT                    VK_LMENU
#define VK_RALT                    VK_RMENU
#define VK_ALT                     VK_MENU
#define VK_PAGEUP                  VK_PRIOR
#define VK_PAGEDOWN                VK_NEXT
#define VK_BACKTICK_TILDE          VK_OEM_3
#define VK_MINUS_UNDERSCORE        VK_OEM_MINUS
#define VK_EQUALS_PLUS             VK_OEM_PLUS
#define VK_LEFTBRACKET_LEFTCURLY   VK_OEM_4
#define VK_RIGHTBRACKET_RIGHTCURLY VK_OEM_6
#define VK_BACKSLASH_BAR           VK_OEM_5
#define VK_SEMICOLON_COLON         VK_OEM_1
#define VK_SINGLEQUOTE_DOUBLEQUOTE VK_OEM_7
#define VK_COMMA_LESSTHAN          VK_OEM_COMMA
#define VK_PERIOD_GREATERTHAN      VK_OEM_PERIOD
#define VK_SLASH_QUESTION          VK_OEM_2

class Keyboard
{
public:
	enum
	{
		MODIFIER_NONE           = 0,
		MODIFIER_SHIFT          = BIT(0),
		MODIFIER_CONTROL        = BIT(1),
		MODIFIER_ALT            = BIT(2),
		MODIFIER_IGNORE_SHIFT   = BIT(3),
		MODIFIER_IGNORE_CONTROL = BIT(4),
		MODIFIER_IGNORE_ALT     = BIT(5),
		MODIFIER_ANY            = MODIFIER_IGNORE_SHIFT | MODIFIER_IGNORE_CONTROL | MODIFIER_IGNORE_ALT,
	};
	Keyboard();

	void Update();

	bool IsKeyDownModifier(uint32 modifiers) const;
	bool IsShiftDown() const;
	bool IsControlDown() const;
	bool IsAltDown() const;
	uint32 GetModifiers() const;

	inline bool IsKeyDown(int vk, uint32 modifiers = MODIFIER_NONE) const { return !!(m_currState[vk] & 0x80) && IsKeyDownModifier(modifiers); }
	inline bool WasKeyDown(int vk) const { return !!(m_prevState[vk] & 0x80); }
	inline bool IsKeyPressed(int vk, uint32 modifiers = MODIFIER_NONE) const { return IsKeyDown(vk, modifiers) && !WasKeyDown(vk); }
	inline bool IsKeyReleased(int vk) const { return WasKeyDown(vk) && !IsKeyDown(vk); }

	void PrintChangedKeyboardState() const;

	uint8 m_prevState[256];
	uint8 m_currState[256];
	void (*m_exitCallback)();
};

#endif // PLATFORM_PC
#endif // _INCLUDE_COMMON_KEYBOARD_H_