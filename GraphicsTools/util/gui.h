// ============
// common/gui.h
// ============

#ifndef _INCLUDE_COMMON_GUI_H_
#define _INCLUDE_COMMON_GUI_H_

#include "common/common.h"

#if defined(_OPENGL)

// TODO --
// marquee widget
// marquee - render mesh with per-triangle color and sample triangle indices, then add to list of vertices
//   render floor texture with per-texel color and sample texels, expand by 1 to get list of texels
//   now we have an array of {point,normal,AO}'s
//   when the sliders are updated (e.g. AO_distance) we can recalc the AO for this list of points and
//   then update the texture (can we update just a bounding box of it?)

// TODO --
// i think i want to change it so the sizes of shit is computed AFTER we construct all the widgets
// combine functionality of sliders etc.:
//   one or more knobs
//   each knob is draggable (components)
//   each knob has a GUIBounds range for the drag
//   each knob is tied to a floating point value (one for x, one for y .. either one can be nullptr)
//   GUISliderElement creates one with a single knob
//   GUIRangeSliderElement creates one with two knobs -- updating one will change the range for the other
//   GUISliderXYElement creates one with a single knob but can move in 2D
//   GUIBoundsXYElement creates one with two knobs and always draws a rectangle between them
//   GUINumericElement draws text tracking a floating point value
//     specify control of +/-, numdigits, numfractionaldigits, hexadecimal, INF/NaN
//     specify control of whether the name is displayed "name:xxx.xxx", or just the value "xxx.xxx"
//     allow tracking of integers or bools too
//   GUISliderElement - creates GUIGenericSliderElement(1) + GUINumericElement
//   GUIRangeSliderElement - creates GUIGenericSliderElement(2) + GUINumericElement(2)
// GUISliderElement and GUIPointsElement::UpdateDrag should update the thumbs directly with integer offsets, *then* calculate the floating point values (not update the thumbs in UpdateValue/Point)
// make it work with software rendering (not opengl)
// make GUISliderElement work with no text, no track, etc.
// make GUISliderElement work with integers (snap to nearest)
// make GUISliderElement work with bools (snap to 0,1)
// GUICheckboxElement (support multiple on one line)
// GUISliderXYElement
// GUIGraphElement (support realtime or one-shot, support multiple graphs, etc.)
// GUIPanelElement (rollouts?)
// GUIRangeSliderElement (slider with two thumbs)
// support sliders with no actual widget (just a value display)
// support sliders with ticks at specific locations (snapping)
// store default values
// save/load all widget values as XML
// tables or grids
// aligning things left/top/right/bottom/corners/center
// scrollbars?
// load/save file dialogs
// color picker dialogs
// integrate GLUT submenus?

class Pixel32;
class Font;
class Keyboard;

namespace GUI
{
	enum eGUIEvent
	{
		EVENT_NONE,
		EVENT_BEGIN_DRAG,
		EVENT_DRAGGING,
		EVENT_END_DRAG,
		EVENT_CHANGE,
		EVENT_RESET,
	};
	enum eGUIState
	{
		STATE_DEFAULT,
		STATE_HOVER,
		STATE_DRAG,
	};
	enum eGUIAlign
	{
		ALIGN_TOPLEFT,
		ALIGN_TOPRIGHT,
		ALIGN_BOTTOMLEFT,
		ALIGN_BOTTOMRIGHT,
	};
}

class GUIBounds
{
public:
	GUIBounds(int x = 0, int y = 0, int w = 0, int h = 0) : m_x0(x), m_y0(y), m_x1(x + w), m_y1(y + h) {}
	
	inline int GetCenterX() const { return (m_x1 + m_x0)/2; }
	inline int GetCenterY() const { return (m_y1 + m_y0)/2; }
	inline int GetExtentX() const { return (m_x1 - m_x0)/2; }
	inline int GetExtentY() const { return (m_y1 - m_y0)/2; }
	inline int GetX() const { return m_x0; }
	inline int GetY() const { return m_y0; }
	inline int GetW() const { return m_x1 - m_x0; }
	inline int GetH() const { return m_y1 - m_y0; }
	inline int GetR() const { return m_x1; } // right
	inline int GetB() const { return m_y1; } // bottom
	inline int GetArea() const { return GetW()*GetH(); }
	inline void SetX(int x) { m_x1 += x - m_x0; m_x0 = x; }
	inline void SetY(int y) { m_y1 += y - m_y0; m_y0 = y; }
	inline void SetW(int w) { m_x1 = m_x0 + w; }
	inline void SetH(int h) { m_y1 = m_y0 + h; }
	inline void SetR(int r) { m_x1 = r; } // right
	inline void SetB(int b) { m_y1 = b; } // bottom
	inline void OffsetX(int dx) { m_x0 += dx; m_x1 += dx; }
	inline void OffsetY(int dy) { m_y0 += dy; m_y1 += dy; }
	inline void SetCenterX(int x) { OffsetX(x - GetCenterX()); }
	inline void SetCenterY(int y) { OffsetY(y - GetCenterY()); }
	inline bool IsPointInside(int x, int y, int expand = 0) const
	{
		return
			m_x0 - expand <= x &&
			m_y0 - expand <= y &&
			m_x1 + expand >= x &&
			m_y1 + expand >= y;
	}
	
	inline int& x0_ref() { return m_x0; }
	inline int& y0_ref() { return m_y0; }
	inline int& x1_ref() { return m_x1; }
	inline int& y1_ref() { return m_y1; }

private:
	int m_x0;
	int m_y0;
	int m_x1;
	int m_y1;
};

class GUIPoint
{
public:
	GUIPoint() {}
	GUIPoint(float x, float y) : m_x(x), m_y(y) {}
	inline bool operator ==(const GUIPoint& rhs) const { return m_x == rhs.m_x && m_y == rhs.m_y; }
	inline bool operator !=(const GUIPoint& rhs) const { return m_x != rhs.m_x || m_y != rhs.m_y; } 
	float m_x;
	float m_y;
};

class GUIElement;
class GUIElementTrack
{
public:
	GUIElementTrack(GUIElement* element, int component = 0) : m_element(element), m_component(component) {}

	operator bool() const { return m_element != nullptr; }

	void SetState(GUI::eGUIState state) const;
	bool UpdateDrag(int mouseX, int mouseY, int dragStartX, int dragStartY) const;
	void EndDrag(int mouseX, int mouseY) const;

	GUIElement* m_element;
	int m_component; // e.g. which thumb in a multi-point slider
};

class GUIElement : public GUIBounds
{
public:
	GUIElement() : m_visible(true) {}
	GUIElement(int x, int y, int w, int h) : GUIBounds(x, y, w, h), m_visible(true) {}
	virtual ~GUIElement() {}

	virtual void SetState(GUI::eGUIState state, int component) {}
	virtual void Idle() {}
	virtual GUIElementTrack Hover(int mouseX, int mouseY) { return nullptr; }
	virtual GUIElementTrack BeginDrag(int mouseX, int mouseY, int& dragStartX, int& dragStartY) { return nullptr; }
	virtual void UpdateDrag(int mouseX, int mouseY, int dragStartX, int dragStartY, int component) {}
	virtual void EndDrag(int mouseX, int mouseY, int component) {}
	virtual void Render(int x, int y, float opacity = 1.0f, float highlight = 0.0f) const {}
	virtual void ResetToInitialValues() {}

	void RenderBoundsFill(int x, int y, const Pixel32& color, int inset = 1) const;
	void RenderBoundsEdge(int x, int y, const Pixel32& color, int expand = 0) const;

	bool m_visible;
};

class GUIFrame : public GUIElement
{
	// when a standard element is created (e.g. box, text, etc.) the bounds x,y is 0,0 and the w,h gets set in the constructor (fixed)
	// when a frame is created, the bounds x,y is 0,0 and the w,h is set to padding*2 (encompassing zero children)
	// when an element is added to a parent frame ..
	//   the child element bounds x,y gets set relative to the parent frame x,y (and fixed)
	//   the parent frame bounds w,h gets updated according to the child bounds
public:
	GUIFrame(int padding = 2, int spacing = 1, bool vertical = true)
		: GUIElement(0, 0, padding*2, padding*2)
		, m_padding(padding)
		, m_spacing(spacing)
		, m_vertical(vertical)
	{}

	template <typename ElementType> inline ElementType* AddElement(ElementType* element) { return static_cast<ElementType*>(AddElement_(element)); }
	GUIElement* AddElement_(GUIElement* element);
	void ReAddVisibleElements();
	void AlignSliders();

	virtual void Idle();
	virtual GUIElementTrack Hover(int mouseX, int mouseY);
	virtual GUIElementTrack BeginDrag(int mouseX, int mouseY, int& dragStartX, int& dragStartY);
	virtual void Render(int x, int y, float opacity = 1.0f) const;
	virtual void ResetToInitialValues();

	const std::vector<GUIElement*>& GetElements() const { return m_elements; }

private:
	std::vector<GUIElement*> m_elements;
	int m_padding; // padding between frame and child elements
	int m_spacing; // spacing between adjacent child elements
	bool m_vertical;
};

class GUIFrameH : public GUIFrame
{
public:
	GUIFrameH(int padding = 2, int spacing = 1) : GUIFrame(padding, spacing, false) {}
};

class GUIBoxElement : public GUIElement
{
public:
	GUIBoxElement(int w, int h, const Pixel32& fillColor, const Pixel32& edgeColor, int fillInset = 1)
		: GUIElement(0, 0, w, h)
		, m_fillColor(fillColor)
		, m_edgeColor(edgeColor)
		, m_fillInset(fillInset)
	{}

	virtual void Render(int x, int y, float opacity = 1.0f, float highlight = 0.0f) const;
	void RenderEx(int x, int y, float opacity = 1.0f,
		float fillOpacity = 1.0f,
		float fillBrightness = 1.0f,
		float edgeOpacity = 1.0f,
		float edgeBrightness = 1.0f) const;

private:
	Pixel32 m_fillColor;
	Pixel32 m_edgeColor;
	int m_fillInset;
};

class GUITextElement : public GUIElement
{
public:
	static void SetFontPath(const char* path);
	static Font* GetCurrentFont();
	static Pixel32& GetCurrentTextColor();
	static Pixel32& GetCurrentShadowColor();
	static int& GetCurrentPadding();

	GUITextElement(
		const char* text,
		const Font* font = GetCurrentFont(),
		const Pixel32& textColor = GetCurrentTextColor(),
		const Pixel32& shadowColor = GetCurrentShadowColor(),
		int padding = GetCurrentPadding());

	void SetText(const char* text);

	virtual void Render(int x, int y, float opacity = 1.0f, float highlight = 0.0f) const;

	std::string m_text;
	const Font* m_font;
	Pixel32 m_textColor;
	Pixel32 m_shadowColor;
	int m_padding;
	int m_dx;
	int m_dy;
};

class GUINumericElement : public GUITextElement
{
public:
	GUINumericElement(const char* label, float& value);

	uint32 ComputeLabelLength() const;
	uint32 ComputeValueLength(float valueMin, float valueMax) const;
	std::string GetValueStr(int paddedLength = 0) const;
	std::string GetText() const;
	void UpdateValue(float value);
	void Update();

	virtual void Idle();
	virtual void Render(int x, int y, float opacity = 1.0f, float highlight = 0.0f) const;

	enum eNumericType
	{
		TYPE_FLOAT,
		TYPE_INT,
		TYPE_BOOL,
	};
	eNumericType m_type;
	std::map<int,std::string> m_enumStrings; // valid for INT and BOOL types
	float* m_valuePtr;
	std::string m_label;
	uint32 m_labelMaxLength;
	uint32 m_valueMaxLength;
	bool m_isInteger;
};

class GUISliderElement : public GUIElement
{
public:
	GUISliderElement(const char* label, float& value, float valueMin, float valueMax, void (*callback)(GUISliderElement*, GUI::eGUIEvent) = nullptr, float step = 0.0001f, const Pixel32& thumbColor = Pixel32(255,255,255), int w = 200, int h = 10);
	virtual ~GUISliderElement();

	static GUISliderElement* Find(float& value);

	virtual float GetRelativeThumbPosition(float value) const;
	virtual float GetValueFromRelativeThumbPosition(float t) const;
	virtual float GetValueInternal() const;
	virtual void UpdateValueInternal(float value);

	virtual void SetState(GUI::eGUIState state, int component);
	virtual void Idle();
	virtual GUIElementTrack Hover(int mouseX, int mouseY);
	virtual GUIElementTrack BeginDrag(int mouseX, int mouseY, int& dragStartX, int& dragStartY);
	float UpdateDragInternal(int mouseX, int mouseY, int dragStartX, int dragStartY) const;
	virtual void UpdateDrag(int mouseX, int mouseY, int dragStartX, int dragStartY, int component);
	virtual void EndDrag(int mouseX, int mouseY, int component);
	virtual void Render(int x, int y, float opacity = 1.0f, float highlight = 0.0f) const;
	virtual void ResetToInitialValues();

	virtual void LinkToValue(float& value);

	bool m_enabled;
	float m_valueInitial;
	float m_valueCurrent;
	bool m_valueTrack; // thumb tracks variable in memory
	void (*m_valueCallback)(GUISliderElement*, GUI::eGUIEvent);
	float m_valueMin;
	float m_valueMax;
	float m_step;
	bool m_snapToIntegers;
	GUI::eGUIState m_state;
	GUINumericElement m_numeric;
	GUIBoxElement* m_sliderBox;
	GUIBoxElement* m_trackBox;
	GUIBoxElement* m_thumb;
	int m_thumbX0;
	int m_thumbX1;
	int m_thumbExpand;
};

class GUIExpSliderElement : public GUISliderElement
{
public:
	GUIExpSliderElement(const char* label, float& value, float valueMin, float valueMax, bool minValueMappedToZero, void (*callback)(GUISliderElement*, GUI::eGUIEvent) = nullptr, float step = 0.0001f, const Pixel32& thumbColor = Pixel32(255,255,255), int w = 200, int h = 10)
		: GUISliderElement(label, value, minValueMappedToZero ? 0.0f : valueMin, valueMax, callback, step, thumbColor, w, h)
		, m_valueMinExp(valueMin)
	{
		ForceAssert(valueMin > 0.0f); // can't be strictly zero, but we can set minValueMappedToZero = true to emulate it
		UpdateValueInternal(value); // needs to get called explicitly here so that thumb is positioned exponentially
	}

	virtual float GetRelativeThumbPosition(float value) const;
	virtual float GetValueFromRelativeThumbPosition(float t) const;

	float m_valueMinExp;
};

class GUIIntSliderElement : public GUISliderElement
{
public:
	GUIIntSliderElement(const char* label, int& value, int valueMin, int valueMax, void (*callback)(GUISliderElement*, GUI::eGUIEvent) = nullptr, const Pixel32& thumbColor = Pixel32(255,255,255), int w = 200, int h = 10);

	static GUIIntSliderElement* Find(int& value);

	void SetEnum(int value, const char* str);

	virtual float GetValueInternal() const;
	virtual void UpdateValueInternal(float value);

	int* m_valueIntPtr;
	float m_valueFloat;
};

class GUIIntComboSliderElement : public GUIIntSliderElement
{
public:
	GUIIntComboSliderElement(const char* label, int& value, int valueMin, int valueMax, const char** enumStrings, void (*callback)(GUISliderElement*, GUI::eGUIEvent) = nullptr, const Pixel32& thumbColor = Pixel32(255,255,255), int w = 200, int h = 10);
	GUIIntComboSliderElement(const char* label, int& value, int valueMin, int valueMax, const std::vector<std::string>& enumStrings, void (*callback)(GUISliderElement*, GUI::eGUIEvent) = nullptr, const Pixel32& thumbColor = Pixel32(255,255,255), int w = 200, int h = 10);
};

class GUIBoolSliderElement : public GUISliderElement
{
public:
	GUIBoolSliderElement(const char* label, bool& value, void (*callback)(GUISliderElement*, GUI::eGUIEvent) = nullptr, const Pixel32& thumbColor = Pixel32(255,255,255), int w = 20, int h = 10);

	static GUIBoolSliderElement* Find(bool& value);

	virtual float GetValueInternal() const;
	virtual void UpdateValueInternal(float value);

	virtual GUIElementTrack BeginDrag(int mouseX, int mouseY, int& dragStartX, int& dragStartY);
	virtual void UpdateDrag(int mouseX, int mouseY, int dragStartX, int dragStartY, int component);

	bool* m_valueBoolPtr;
	float m_valueFloat;
};

class GUIPointsElement : public GUIElement
{
public:
	GUIPointsElement(GUIPoint* points, int count, int w = 100, int h = 100);
	virtual ~GUIPointsElement();

	void UpdatePoint(const GUIPoint& point, int index);

	virtual void SetState(GUI::eGUIState state, int component);
	virtual void Idle();
	virtual GUIElementTrack Hover(int mouseX, int mouseY);
	virtual GUIElementTrack BeginDrag(int mouseX, int mouseY, int& dragStartX, int& dragStartY);
	virtual void UpdateDrag(int mouseX, int mouseY, int dragStartX, int dragStartY, int component);
	virtual void EndDrag(int mouseX, int mouseY, int component);
	virtual void Render(int x, int y, float opacity = 1.0f, float highlight = 0.0f) const;
	virtual void ResetToInitialValues();

	bool m_enabled;
	GUIPoint* m_pointsPtr;
	int m_pointsCount;
	std::vector<GUIPoint> m_pointsInitial;
	std::vector<GUIPoint> m_pointsCurrent;
	bool m_pointsTrack; // thumb tracks variables in memory
	void (*m_pointsCallback)(GUIPointsElement*, GUI::eGUIEvent, int index);
	std::vector<GUI::eGUIState> m_states;
	GUIBoxElement* m_sliderBox;
	std::vector<GUIBoxElement*> m_thumbs;
	int m_thumbX0;
	int m_thumbX1;
	int m_thumbY0;
	int m_thumbY1;
	int m_thumbSize;
	int m_thumbExpand;
};

class GUIWindow
{
public:
	GUIWindow(int framePadding = 2, int frameSpacing = 1, bool frameVertical = true);

	GUIFrame m_frame;
	Pixel32 m_background;
	int m_renderOriginX;
	int m_renderOriginY;
	int m_scrollY;
	bool m_mouseDown;
	GUIElementTrack m_dragElement;
	GUIElement* m_dragElementLast;
	int m_dragStartX;
	int m_dragStartY;
	GUIElementTrack m_hoverElement;
};

namespace GUI
{
	void RegisterWindow(GUIWindow* window, const Pixel32& background = Pixel32(255,255,255,128));
	int RegisterNewWindow(GUIWindow* window, const char* title);
	GUIWindow* GetWindowFromID(int windowID);
	GUIWindow* GetCurrentWindow();
	bool& GetTrackValuesRef();
	void Idle(const Keyboard* keyboard = nullptr);
	bool MouseButton(int button, int state, int x, int y, uint32 keyboardModifiers);
	bool MouseMotion(int x, int y);
	void RenderBegin(int x, int y, int w = 0, int h = 0);
	void RenderWindow(int x, int y, int w = 0, int h = 0);
	void RenderWindow(eGUIAlign align = ALIGN_TOPRIGHT, int dx = 0, int dy = 0);

	void HandleSliderArrowKey(float delta);
	void HandleSliderKeyboardInput();

	GUIWindow* CreateFrameTest();

	void PrintEvent(const char* msg, eGUIEvent event);
};

namespace Marquee
{
	void Register(bool (*callback)(const GUIBounds& bounds, GUI::eGUIEvent event) = nullptr);
	void UnRegister();
	GUIBounds GetBounds();
	void SetVisible(bool visible);
	bool MouseButton(int button, int state, int x, int y);
	bool MouseMotion(int x, int y);
	void Render(const Pixel32& fillColor = Pixel32(255,255,255,32), const Pixel32& edgeColor = Pixel32(255,255,255,128));
};

#endif // defined(_OPENGL)
#endif // _INCLUDE_COMMON_GUI_H_