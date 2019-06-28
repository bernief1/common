// ==============
// common/gui.cpp
// ==============

#if defined(_OPENGL)

#include "font.h"
#include "gui.h"
#include "imageutil.h"
#include "keyboard.h"
#include "stringutil.h"

#include "vmath/vmath_color.h"

#include <iostream>

void GUIElementTrack::SetState(GUI::eGUIState state) const
{
	if (m_element)
		m_element->SetState(state, m_component);
}

bool GUIElementTrack::UpdateDrag(int mouseX, int mouseY, int dragStartX, int dragStartY) const
{
	if (m_element) {
		m_element->UpdateDrag(mouseX, mouseY, dragStartX, dragStartY, m_component);
		return true;
	} else
		return false;
}

void GUIElementTrack::EndDrag(int mouseX, int mouseY) const
{
	if (m_element)
		m_element->EndDrag(mouseX, mouseY, m_component);
}

void GUIElement::RenderBoundsFill(int x, int y, const Pixel32& color, int inset) const
{
	if (color.a > 0) {
		const int x0 = x + GetX();
		const int y0 = y + GetY();
		const int x1 = x0 + GetW();
		const int y1 = y0 + GetH();
		glColor4ub(color.r, color.g, color.b, color.a);
		glBegin(GL_QUADS);
		glVertex2i(x0 + inset, y0 + inset);
		glVertex2i(x1 - inset, y0 + inset);
		glVertex2i(x1 - inset, y1 - inset);
		glVertex2i(x0 + inset, y1 - inset);
		glEnd();
	}
}

void GUIElement::RenderBoundsEdge(int x, int y, const Pixel32& color, int expand) const
{
	if (color.a > 0) {
		const int x0 = x + GetX();
		const int y0 = y + GetY();
		const int x1 = x0 + GetW();
		const int y1 = y0 + GetH();
		glColor4ub(color.r, color.g, color.b, color.a);
		glBegin(GL_LINE_LOOP);
		glVertex2f(0.5f + (float)(x0 - 0 - expand), 0.5f + (float)(y0 - 0 - expand));
		glVertex2f(0.5f + (float)(x1 - 1 + expand), 0.5f + (float)(y0 - 0 - expand));
		glVertex2f(0.5f + (float)(x1 - 1 + expand), 0.5f + (float)(y1 - 1 + expand));
		glVertex2f(0.5f + (float)(x0 - 0 - expand), 0.5f + (float)(y1 - 1 + expand));
		glEnd();
	}
}

void GUIFrame::ReAddVisibleElements()
{
	SetW(m_padding*2); // reset dimensions
	SetH(m_padding*2);
	bool first = true;
	for (uint32 i = 0; i < m_elements.size(); i++) {
		GUIElement* element = m_elements[i];
		if (element->m_visible) {
			const int elementW = element->GetW();
			const int elementH = element->GetH();
			int frameW = GetW();
			int frameH = GetH();
			if (m_vertical) {
				frameW = Max(elementW + m_padding*2, frameW);
				frameH += elementH + (first ? 0 : m_spacing);
				element->SetX(m_padding);
				element->SetY(frameH - m_padding - elementH);
			} else {
				frameH = Max(elementH + m_padding*2, frameH);
				frameW += elementW + (first ? 0 : m_spacing);
				element->SetY(m_padding);
				element->SetX(frameW - m_padding - elementW);
			}
			SetW(frameW);
			SetH(frameH);
			first = false;
		}
	}
	//AlignSliders();
}

GUIElement* GUIFrame::AddElement_(GUIElement* element)
{
	const int elementW = element->GetW();
	const int elementH = element->GetH();
	int frameW = GetW();
	int frameH = GetH();
	if (m_vertical) {
		frameW = Max(elementW + m_padding*2, frameW);
		frameH += elementH + (m_elements.empty() ? 0 : m_spacing);
		element->SetX(m_padding);
		element->SetY(frameH - m_padding - elementH);
	} else {
		frameH = Max(elementH + m_padding*2, frameH);
		frameW += elementW + (m_elements.empty() ? 0 : m_spacing);
		element->SetY(m_padding);
		element->SetX(frameW - m_padding - elementW);
	}
	SetW(frameW);
	SetH(frameH);
	m_elements.push_back(element);
	return element;
}

void GUIFrame::AlignSliders()
{
	uint32 labelMaxLength = 0;
	uint32 valueMaxLength = 0;
	for (uint32 i = 0; i < m_elements.size(); i++) {
		const GUISliderElement* slider = dynamic_cast<const GUISliderElement*>(m_elements[i]);
		if (slider && slider->m_visible) {
			labelMaxLength = Max(slider->m_numeric.ComputeLabelLength(), labelMaxLength);
			valueMaxLength = Max(slider->m_numeric.ComputeValueLength(slider->m_valueMin, slider->m_valueMax), valueMaxLength);
		}
	}
	int frameR = GetR();
	for (uint32 i = 0; i < m_elements.size(); i++) {
		GUISliderElement* slider = dynamic_cast<GUISliderElement*>(m_elements[i]);
		if (slider && slider->m_visible) {
			slider->m_numeric.m_labelMaxLength = labelMaxLength;
			slider->m_numeric.m_valueMaxLength = valueMaxLength;
			slider->m_numeric.Update();
			const int offsetX = slider->m_numeric.GetW() - slider->m_sliderBox->GetX(); // move all the slider bits this amount
			slider->m_sliderBox->OffsetX(offsetX);
			slider->m_trackBox->OffsetX(offsetX);
			slider->m_thumb->OffsetX(offsetX);
			slider->m_thumbX0 += offsetX;
			slider->m_thumbX1 += offsetX;
			slider->SetW(slider->m_numeric.GetW() + slider->m_sliderBox->GetW());
			frameR = Max(slider->GetR(), frameR);
		}
	}
	SetR(frameR);
}

void GUIFrame::Idle()
{
	for (uint32 i = 0; i < m_elements.size(); i++) {
		GUIElement* element = m_elements[i];
		if (element->m_visible)
			element->Idle();
	}
}

GUIElementTrack GUIFrame::Hover(int mouseX, int mouseY)
{
	mouseX -= GetX();
	mouseY -= GetY();
	GUIElementTrack hoverElement = nullptr;
	for (uint32 i = 0; i < m_elements.size(); i++) {
		GUIElement* element = m_elements[i];
		if (element->m_visible && element->IsPointInside(mouseX, mouseY)) {
			hoverElement = element->Hover(mouseX, mouseY);
			if (hoverElement)
				break;
		}
	}
	return hoverElement;
}

GUIElementTrack GUIFrame::BeginDrag(int mouseX, int mouseY, int& dragStartX, int& dragStartY)
{
	mouseX -= GetX();
	mouseY -= GetY();
	GUIElementTrack dragElement = nullptr;
	for (uint32 i = 0; i < m_elements.size(); i++) {
		GUIElement* element = m_elements[i];
		if (element->m_visible && element->IsPointInside(mouseX, mouseY)) {
			dragElement = element->BeginDrag(mouseX, mouseY, dragStartX, dragStartY);
			if (dragElement)
				break;
		}
	}
	return dragElement;
}

void GUIFrame::Render(int x, int y, float opacity) const
{
	x += GetX();
	y += GetY();
	for (uint32 i = 0; i < m_elements.size(); i++) {
		GUIElement* element = m_elements[i];
		if (element->m_visible) {
			float highlight = 0.0f;
			if (GUI::GetCurrentWindow()->m_hoverElement.m_element == element)
				highlight = 0.5f;
			element->Render(x, y, opacity, highlight);
		}
	}
}

void GUIFrame::ResetToInitialValues()
{
	for (uint32 i = 0; i < m_elements.size(); i++) {
		GUIElement* element = m_elements[i];
		element->ResetToInitialValues();
	}
}

void GUIBoxElement::Render(int x, int y, float opacity, float highlight) const
{
	RenderBoundsFill(x, y, m_fillColor.ScaleAlpha(opacity), m_fillInset);
	RenderBoundsEdge(x, y, m_edgeColor.ScaleAlpha(opacity));
}

void GUIBoxElement::RenderEx(int x, int y, float opacity, float fillOpacity, float fillBrightness, float edgeOpacity, float edgeBrightness) const
{
	RenderBoundsFill(x, y, m_fillColor.ScaleColor(fillBrightness).ScaleAlpha(fillOpacity*opacity), m_fillInset);
	RenderBoundsEdge(x, y, m_edgeColor.ScaleColor(edgeBrightness).ScaleAlpha(edgeOpacity*opacity));
}

static char s_GUITextFontPath[512] = "font/font_lucida_console_14.fnt";

void GUITextElement::SetFontPath(const char* path)
{
	strcpy(s_GUITextFontPath, path);
}

Font* GUITextElement::GetCurrentFont()
{
#if 1
	static Font* s_currentFont = nullptr;
	if (s_currentFont == nullptr) {
		s_currentFont = Font::Load(s_GUITextFontPath);
		if (s_currentFont && 0)
			s_currentFont->SaveImage("font_a.png", true);
	}
	return s_currentFont;
#else
	static std::map<int,Font*> s_fonts; // why doesn't the effing font display in the other GLUT windows?? do i really need to load Font* separately? doesn't even help!!
	Font* font = nullptr;
	const int windowID = glutGetWindow();
	const auto it = s_fonts.find(windowID);
	if (it == s_fonts.end()) {
		font = Font::Load(s_GUITextFontPath);
		s_fonts[windowID] = font;
	} else
		font = it->second;
	return font;
#endif
}

Pixel32& GUITextElement::GetCurrentTextColor()
{
	static Pixel32 s_currentTextColor = Pixel32(255,255,255);
	return s_currentTextColor;
}

Pixel32& GUITextElement::GetCurrentShadowColor()
{
	static Pixel32 s_currentShadowColor = Pixel32(0,0,0,80);
	return s_currentShadowColor;
}

int& GUITextElement::GetCurrentPadding()
{
	static int s_currentPadding = 2;
	return s_currentPadding;
}

GUITextElement::GUITextElement(const char* text, const Font* font, const Pixel32& textColor, const Pixel32& shadowColor, int padding)
	: m_font(font)
	, m_textColor(textColor)
	, m_shadowColor(shadowColor)
	, m_padding(padding)
{
	SetText(text);
}

void GUITextElement::SetText(const char* text)
{
	m_text = text;
	int sw = 0;
	int sh = 0;
	int dx = 0;
	int dy = 0;
	if (m_font)
		m_font->GetStringDimensions(sw, sh, dy, text);
	SetW(sw + m_padding*2);
	SetH(sh + m_padding*2);
	m_dx = dx + m_padding;
	m_dy = dy + m_padding;
}

void GUITextElement::Render(int x, int y, float opacity, float highlight) const
{
	if (m_font) {
		x += GetX();
		y += GetY();
		if (m_shadowColor.a > 0)
			m_font->DrawStringGL((float)(x + m_dx + 1), (float)(y + m_dy + 1), 1.0f, m_shadowColor.ScaleAlpha(opacity), m_text.c_str());
		if (m_textColor.a > 0) {
			Vec4V colorAndAlpha = m_textColor.ScaleAlpha(opacity);
			if (highlight > 0.0f) {
				Vec3V color = colorAndAlpha.xyz();
				float alpha = colorAndAlpha.wf();
				color *= (1.0f + highlight*0.5f); // brighter ..
				color += (Vec3V(V_ONE) - color)*highlight; // .. and white-er ..
				alpha = Min(alpha*2.0f, 1.0f); // .. and more opaque
				colorAndAlpha = Vec4V(color, alpha);
			}
			m_font->DrawStringGL((float)(x + m_dx), (float)(y + m_dy), 1.0f, colorAndAlpha, m_text.c_str());
		}
	}
}

static void CenterVertically(GUIElement* a, GUIElement* b)
{
	if (b->GetH() > a->GetH())
		a->SetY((b->GetH() - a->GetH())/2);
	else if (b->GetH() < a->GetH())
		b->SetY((a->GetH() - b->GetH())/2);
}

GUINumericElement::GUINumericElement(const char* label, float& value)
	: GUITextElement("")
	, m_type(TYPE_FLOAT)
	, m_valuePtr(&value)
	, m_label(label)
	, m_labelMaxLength(0)
	, m_valueMaxLength(0)
	, m_isInteger(false)
{
	Update();
}

uint32 GUINumericElement::ComputeLabelLength() const
{
	if (m_label.empty())
		return 0;
	else
		return (uint32)(m_label.size() + strlen(":"));
}

static void GUINumericPrintFloatValue(char* str, const float& value)
{
	const bool neg = !!(((*(const uint32*)&value)) & 0x80000000); // indicates floating point will print with a preceding '-'
	sprintf(str, "%s%.5f", neg ? "" : " ", value);
}

static void GUINumericPrintIntValue(char* str, const int& value)
{
	const bool neg = value < 0;
	sprintf(str, "%s%d", neg ? "" : " ", value);
}

uint32 GUINumericElement::ComputeValueLength(float valueMin, float valueMax) const
{
	char valueMinStr[64];
	char valueMaxStr[64];
	uint32 length = 0;
	if (m_type == TYPE_FLOAT) {
		GUINumericPrintFloatValue(valueMinStr, valueMin);
		GUINumericPrintFloatValue(valueMaxStr, valueMax);
		length = Max((uint32)strlen(valueMinStr), (uint32)strlen(valueMaxStr));
	} else {
		GUINumericPrintIntValue(valueMinStr, (int)valueMin);
		GUINumericPrintIntValue(valueMaxStr, (int)valueMax);
		length = Max((uint32)strlen(valueMinStr), (uint32)strlen(valueMaxStr));
		for (auto it = m_enumStrings.begin(); it != m_enumStrings.end(); ++it)
			length = Max((uint32)(it->second.size() + strlen(" ")), length); 
	}
	return length;
}

std::string GUINumericElement::GetValueStr(int paddedLength) const
{
	char valueStr[256] = "";
	const float valueFloat = *m_valuePtr;
	if (m_type == TYPE_FLOAT)
		GUINumericPrintFloatValue(valueStr, valueFloat);
	else {
		const int valueInt = (m_type == TYPE_BOOL) ? (valueFloat > 0.5f ? 1 : 0) : (int)Round(valueFloat);
		const auto it = m_enumStrings.find(valueInt);
		if (it != m_enumStrings.end())
			sprintf(valueStr, " %s", it->second.c_str());
		else
			GUINumericPrintIntValue(valueStr, valueInt);
	}
	while ((int)strlen(valueStr) < paddedLength)
		strcat(valueStr, " ");
	return valueStr;
}

std::string GUINumericElement::GetText() const
{
	std::string valueStr = GetValueStr(m_valueMaxLength);
	char text[256] = "";
	if (!m_label.empty())
		sprintf(text, "%s:", m_label.c_str());
	while (strlen(text) < m_labelMaxLength)
		strcat(text, " ");
	strcat(text, valueStr.c_str());
	return text;
}

void GUINumericElement::UpdateValue(float value)
{
	*m_valuePtr = value;
	Update();
}

void GUINumericElement::Update()
{
	SetText(GetText().c_str());
}

void GUINumericElement::Idle()
{
	Update();
}

void GUINumericElement::Render(int x, int y, float opacity, float highlight) const
{
	GUITextElement::Render(x, y, opacity, highlight);
}

namespace GUI
{
	static std::map<float*,GUISliderElement*> g_GUISliderMap;
	static std::map<int*,GUIIntSliderElement*> g_GUIIntSliderMap;
	static std::map<bool*,GUIBoolSliderElement*> g_GUIBoolSliderMap;
	static bool g_trackValues = true;
}

GUISliderElement::GUISliderElement(const char* label, float& value, float valueMin, float valueMax, void (*callback)(GUISliderElement*, GUI::eGUIEvent), float step, const Pixel32& thumbColor, int w, int h)
	: m_enabled(true)
	//, m_valueInitial(value) // value gets clamped destructively first
	//, m_valueCurrent(value)
	, m_valueCallback(callback)
	, m_valueMin(valueMin)
	, m_valueMax(valueMax)
	, m_step(step)
	, m_snapToIntegers(false)
	, m_state(GUI::STATE_DEFAULT)
	, m_numeric(label, value)
{
	value = m_valueInitial = m_valueCurrent = Clamp(value, valueMin, valueMax);

	GUI::g_GUISliderMap[&value] = this;

	const int thumbW = 8;
	const int thumbH = 16;
	const int trackH = 4;
	const int trackW = w - h + trackH;

	m_sliderBox = new GUIBoxElement(w, h, 0, Pixel32(255,255,255,128));
	m_sliderBox->SetX(m_numeric.GetW());
	CenterVertically(m_sliderBox, &m_numeric);
	SetW(m_numeric.GetW() + m_sliderBox->GetW());
	SetH(Max(m_numeric.GetH(), m_sliderBox->GetH()));

	m_trackBox = new GUIBoxElement(trackW, trackH, Pixel32(255,255,255,64), 0, 0);
	m_trackBox->SetX(m_sliderBox->GetX() + (w - trackW)/2);
	m_trackBox->SetY(m_sliderBox->GetY() + (h - trackH)/2);
	m_thumb = new GUIBoxElement(thumbW, thumbH, thumbColor, Pixel32(255,255,255), 0);
	m_thumbX0 = m_sliderBox->GetX() + 2;
	m_thumbX1 = m_sliderBox->GetR() - thumbW - 2;
	m_thumbExpand = 2; // make it slightly easier to grab
	m_thumb->SetCenterY(m_sliderBox->GetCenterY());

	UpdateValueInternal(value);
}

GUISliderElement::~GUISliderElement()
{
	if (m_sliderBox)
		delete m_sliderBox;
	if (m_trackBox)
		delete m_trackBox;
	delete m_thumb;
}

GUISliderElement* GUISliderElement::Find(float& value)
{
	const auto it = GUI::g_GUISliderMap.find(&value);
	if (it != GUI::g_GUISliderMap.end())
		return it->second;
	else
		return nullptr;
}

float GUISliderElement::GetRelativeThumbPosition(float value) const
{
	return Saturate((value - m_valueMin)/(m_valueMax - m_valueMin));
}

float GUISliderElement::GetValueFromRelativeThumbPosition(float t) const
{
	return Clamp(m_valueMin + (m_valueMax - m_valueMin)*t, m_valueMin, m_valueMax);
}

float GUIExpSliderElement::GetRelativeThumbPosition(float value) const
{
	if (m_valueMin < m_valueMinExp)
		value = value*(m_valueMax - m_valueMinExp)/m_valueMax + m_valueMinExp; // adjust for zero
	const float valueMinLog2 = log2f(m_valueMinExp);
	const float valueMaxLog2 = log2f(m_valueMax);
	const float valueLog2 = log2f(value);
	const float t = Saturate((valueLog2 - valueMinLog2)/(valueMaxLog2 - valueMinLog2));
	return t;
}

float GUIExpSliderElement::GetValueFromRelativeThumbPosition(float t) const
{
	float valueExp = m_valueMinExp*powf(m_valueMax/m_valueMinExp, t);
	if (m_valueMin < m_valueMinExp)
		valueExp = (valueExp - m_valueMinExp)*m_valueMax/(m_valueMax - m_valueMinExp); // adjust for zero
	return Clamp(valueExp, m_valueMin, m_valueMax);
}

float GUISliderElement::GetValueInternal() const
{
	return *m_numeric.m_valuePtr;
}

void GUISliderElement::UpdateValueInternal(float value)
{
	m_valueCurrent = value;
	m_numeric.UpdateValue(value);
	const float t = GetRelativeThumbPosition(value);
	m_thumb->SetX(m_thumbX0 + (int)Round((float)(m_thumbX1 - m_thumbX0)*t));
}

void GUISliderElement::SetState(GUI::eGUIState state, int component)
{
	m_state = state;
}

void GUISliderElement::Idle()
{
	if (GUI::g_trackValues) {
		const float value = GetValueInternal();
		if (m_valueCurrent != value) {
			UpdateValueInternal(value);
			m_numeric.Idle();
			if (m_valueCallback)
				m_valueCallback(this, GUI::EVENT_CHANGE);
		}
	}
}

GUIElementTrack GUISliderElement::Hover(int mouseX, int mouseY)
{
	if (m_enabled) {
		mouseX -= GetX();
		mouseY -= GetY();
		if (m_thumb->IsPointInside(mouseX, mouseY, m_thumbExpand))
			return this;
	}
	return nullptr;
}

GUIElementTrack GUISliderElement::BeginDrag(int mouseX, int mouseY, int& dragStartX, int& dragStartY)
{
	if (m_enabled) {
		mouseX -= GetX();
		mouseY -= GetY();
		if (m_thumb->IsPointInside(mouseX, mouseY, m_thumbExpand)) {
			dragStartX = mouseX - m_thumb->GetX();
			dragStartY = 0;
			m_state = GUI::STATE_DRAG;
			if (m_valueCallback)
				m_valueCallback(this, GUI::EVENT_BEGIN_DRAG);
			return this;
		}
	}
	return nullptr;
}

float GUISliderElement::UpdateDragInternal(int mouseX, int mouseY, int dragStartX, int dragStartY) const
{
	mouseX -= GetX() + dragStartX;
	mouseY -= GetY() + dragStartY;
	const float t = (float)(mouseX - m_thumbX0)/(float)(m_thumbX1 - m_thumbX0); // [0..1]
	float value = GetValueFromRelativeThumbPosition(t);
	if (m_snapToIntegers)
		value = Round(value);
	return value;
}

void GUISliderElement::UpdateDrag(int mouseX, int mouseY, int dragStartX, int dragStartY, int component)
{
	float value = UpdateDragInternal(mouseX, mouseY, dragStartX, dragStartY);
	if (value != m_valueCurrent) {
		UpdateValueInternal(value);
		if (m_valueCallback)
			m_valueCallback(this, GUI::EVENT_DRAGGING);
	}
}

void GUISliderElement::EndDrag(int mouseX, int mouseY, int component)
{
	if (m_valueCallback)
		m_valueCallback(this, GUI::EVENT_END_DRAG);
	m_state = GUI::STATE_DEFAULT;
}

void GUISliderElement::Render(int x, int y, float opacity, float highlight) const
{
	x += GetX();
	y += GetY();
	if (!m_enabled)
		opacity *= 0.5f;
	m_numeric.Render(x, y, opacity, highlight);
	if (m_sliderBox)
		m_sliderBox->Render(x, y, opacity);
	if (m_trackBox)
		m_trackBox->Render(x, y, opacity);
	m_thumb->RenderEx(x, y, opacity,
		1.0f, // fillOpacity
		1.0f, // fillBrightness
		(m_state == GUI::STATE_HOVER || m_state == GUI::STATE_DRAG) ? 1.0f : 0.5f, // edgeOpacity
		(m_state == GUI::STATE_DRAG) ? 1.0f : 0.75f); // edgeBrightness
}

void GUISliderElement::ResetToInitialValues()
{
	UpdateValueInternal(m_valueInitial);
	if (m_valueCallback)
		m_valueCallback(this, GUI::EVENT_RESET);
}

void GUISliderElement::LinkToValue(float& value)
{
	m_numeric.m_valuePtr = &value;
	UpdateValueInternal(value);
}

GUIIntSliderElement::GUIIntSliderElement(const char* label, int& value, int valueMin, int valueMax, void (*callback)(GUISliderElement*, GUI::eGUIEvent), const Pixel32& thumbColor, int w, int h)
	: GUISliderElement(label, m_valueFloat = (float)value, (float)valueMin, (float)valueMax, callback, 1, thumbColor, w, h)
	, m_valueIntPtr(&value)
	, m_valueFloat((float)value)
{
	GUI::g_GUIIntSliderMap[&value] = this;

	m_numeric.m_type = GUINumericElement::TYPE_INT;
	m_snapToIntegers = true;
}

GUIIntSliderElement* GUIIntSliderElement::Find(int& value)
{
	const auto it = GUI::g_GUIIntSliderMap.find(&value);
	if (it != GUI::g_GUIIntSliderMap.end())
		return it->second;
	else
		return nullptr;
}

void GUIIntSliderElement::SetEnum(int value, const char* str)
{
	m_numeric.m_enumStrings[value] = str;
	if (value == *m_valueIntPtr)
		m_numeric.Update();
}

float GUIIntSliderElement::GetValueInternal() const
{
	return (float)*m_valueIntPtr;
}

void GUIIntSliderElement::UpdateValueInternal(float value)
{
	value = Round(value);
	*m_valueIntPtr = (int)value;
	GUISliderElement::UpdateValueInternal(value);
}

GUIIntComboSliderElement::GUIIntComboSliderElement(const char* label, int& value, int valueMin, int valueMax, const char** enumStrings, void (*callback)(GUISliderElement*, GUI::eGUIEvent), const Pixel32& thumbColor, int w, int h)
	: GUIIntSliderElement(label, value, valueMin, valueMax, callback, thumbColor, w, h)
{
	//ForceAssert(valueMin == 0); // TODO -- remove this assert?
	for (int i = valueMin; i <= valueMax; i++)
		SetEnum(i, enumStrings[i - valueMin]);
}

GUIIntComboSliderElement::GUIIntComboSliderElement(const char* label, int& value, int valueMin, int valueMax, const std::vector<std::string>& enumStrings, void (*callback)(GUISliderElement*, GUI::eGUIEvent), const Pixel32& thumbColor, int w, int h)
	: GUIIntSliderElement(label, value, valueMin, valueMax, callback, thumbColor, w, h)
{
	//ForceAssert(valueMin == 0); // TODO -- remove this assert?
	for (int i = valueMin; i <= valueMax; i++)
		SetEnum(i, enumStrings[i - valueMin].c_str());
}

GUIBoolSliderElement::GUIBoolSliderElement(const char* label, bool& value, void (*callback)(GUISliderElement*, GUI::eGUIEvent), const Pixel32& thumbColor, int w, int h)
	: GUISliderElement(label, m_valueFloat = (value ? 1.0f : 0.0f), 0.0f, 1.0f, callback, 1, thumbColor, w, h)
	, m_valueBoolPtr(&value)
	, m_valueFloat(value ? 1.0f : 0.0f)
{
	GUI::g_GUIBoolSliderMap[&value] = this;

	m_numeric.m_type = GUINumericElement::TYPE_BOOL;
	m_snapToIntegers = true;
}

GUIBoolSliderElement* GUIBoolSliderElement::Find(bool& value)
{
	const auto it = GUI::g_GUIBoolSliderMap.find(&value);
	if (it != GUI::g_GUIBoolSliderMap.end())
		return it->second;
	else
		return nullptr;
}

float GUIBoolSliderElement::GetValueInternal() const
{
	return *m_valueBoolPtr ? 1.0f : 0.0f;
}

void GUIBoolSliderElement::UpdateValueInternal(float value)
{
	if (value > 0.5f) {
		value = 1.0f;
		*m_valueBoolPtr = true;
	} else {
		value = 0.0f;
		*m_valueBoolPtr = false;
	}
	GUISliderElement::UpdateValueInternal(value);
}

GUIElementTrack GUIBoolSliderElement::BeginDrag(int mouseX, int mouseY, int& dragStartX, int& dragStartY)
{
	if (m_enabled) {
		bool& value = *m_valueBoolPtr;
		value = !value; // toggle it
		GUIElementTrack track = GUISliderElement::BeginDrag(mouseX, mouseY, dragStartX, dragStartY);
		GUISliderElement::UpdateDrag(mouseX, mouseY, dragStartX, dragStartY, 0);
		return track;
	} else
		return nullptr;
}

void GUIBoolSliderElement::UpdateDrag(int mouseX, int mouseY, int dragStartX, int dragStartY, int component)
{
	// do nothing
}

GUIPointsElement::GUIPointsElement(GUIPoint* points, int count, int w, int h)
	: m_enabled(true)
	, m_pointsPtr(points)
	, m_pointsCount(count)
	, m_pointsInitial(points, points + count)
	, m_pointsCurrent(points, points + count)
	, m_pointsCallback(nullptr)
	, m_states(count, GUI::STATE_DEFAULT)
{
	m_sliderBox = new GUIBoxElement(w, h, 0, Pixel32(255,255,255,128));
	SetW(m_sliderBox->GetW());
	SetH(m_sliderBox->GetH());
	m_thumbSize = 9;
	m_thumbX0 = m_sliderBox->GetX() + 2;
	m_thumbX1 = m_sliderBox->GetR() - 2 - m_thumbSize;
	m_thumbY0 = m_sliderBox->GetY() + 2;
	m_thumbY1 = m_sliderBox->GetB() - 2 - m_thumbSize;
	m_thumbExpand = 2; // make it slightly easier to grab it
	m_thumbs.resize(count);
	for (int index = 0; index < count; index++) {
		const Pixel32 thumbColor = Pixel32(0,0,255,255);
		GUIBoxElement* thumb = new GUIBoxElement(m_thumbSize, m_thumbSize, thumbColor, Pixel32(255,255,255), 0);
		thumb->SetCenterX(m_thumbX0);
		thumb->SetCenterY(m_thumbY0);
		m_thumbs[index] = thumb;
		UpdatePoint(points[index], index);
	}
}

GUIPointsElement::~GUIPointsElement()
{
	if (m_sliderBox)
		delete m_sliderBox;
	for (int index = 0; index < m_thumbs.size(); index++)
		delete m_thumbs[index];
}

void GUIPointsElement::UpdatePoint(const GUIPoint& point, int index)
{
	ForceAssert(index >= 0 && index < m_pointsCount);
	m_pointsPtr[index] = point;
	m_pointsCurrent[index] = point;
	const float tx = Saturate(point.m_x);
	const float ty = Saturate(point.m_y);
	m_thumbs[index]->SetX(m_thumbX0 + (int)Round((float)(m_thumbX1 - m_thumbX0)*tx));
	m_thumbs[index]->SetY(m_thumbY0 + (int)Round((float)(m_thumbY1 - m_thumbY0)*ty));
}

void GUIPointsElement::SetState(GUI::eGUIState state, int component)
{
	m_states[component] = state;
}

void GUIPointsElement::Idle()
{
	if (GUI::g_trackValues) {
		for (int index = 0; index < m_pointsCount; index++) {
			const GUIPoint point = m_pointsPtr[index];
			if (m_pointsCurrent[index] != point)
				UpdatePoint(point, index);
		}
	}
}

GUIElementTrack GUIPointsElement::Hover(int mouseX, int mouseY)
{
	if (m_enabled) {
		mouseX -= GetX();
		mouseY -= GetY();
		for (int index = 0; index < m_pointsCount; index++) {
			if (m_thumbs[index]->IsPointInside(mouseX, mouseY, m_thumbExpand))
				return GUIElementTrack(this, index);
		}
	}
	return nullptr;
}

GUIElementTrack GUIPointsElement::BeginDrag(int mouseX, int mouseY, int& dragStartX, int& dragStartY)
{
	if (m_enabled) {
		mouseX -= GetX();
		mouseY -= GetY();
		for (int index = 0; index < m_pointsCount; index++) {
			if (m_thumbs[index]->IsPointInside(mouseX, mouseY, m_thumbExpand)) {
				dragStartX = mouseX - m_thumbs[index]->GetX();
				dragStartY = mouseY - m_thumbs[index]->GetY();
				m_states[index] = GUI::STATE_DRAG;
				if (m_pointsCallback)
					m_pointsCallback(this, GUI::EVENT_BEGIN_DRAG, index);
				return GUIElementTrack(this, index);
			}
		}
	}
	return nullptr;
}

void GUIPointsElement::UpdateDrag(int mouseX, int mouseY, int dragStartX, int dragStartY, int component)
{
	mouseX -= GetX() + dragStartX;
	mouseY -= GetY() + dragStartY;
	const float tx = (float)(mouseX - m_thumbX0)/(float)(m_thumbX1 - m_thumbX0); // [0..1]
	const float ty = (float)(mouseY - m_thumbY0)/(float)(m_thumbY1 - m_thumbY0); // [0..1]
	UpdatePoint(GUIPoint(Saturate(tx), Saturate(ty)), component);
	if (m_pointsCallback)
		m_pointsCallback(this, GUI::EVENT_DRAGGING, component);
}

void GUIPointsElement::EndDrag(int mouseX, int mouseY, int component)
{
	if (m_pointsCallback)
		m_pointsCallback(this, GUI::EVENT_END_DRAG, component);
	m_states[component] = GUI::STATE_DEFAULT;
}

void GUIPointsElement::Render(int x, int y, float opacity, float highlight) const
{
	x += GetX();
	y += GetY();
	if (!m_enabled)
		opacity *= 0.5f;
	if (m_sliderBox)
		m_sliderBox->Render(x, y, opacity, highlight);
	for (int index = 0; index < m_pointsCount; index++) {
		const GUI::eGUIState state = m_states[index];
		m_thumbs[index]->RenderEx(x, y, opacity,
			1.0f, // fillOpacity
			1.0f, // fillBrightness
			(state == GUI::STATE_HOVER || state == GUI::STATE_DRAG) ? 1.0f : 0.5f, // edgeOpacity
			(state == GUI::STATE_HOVER) ? 1.0f : 0.75f); // edgeBrightness
	}
}

void GUIPointsElement::ResetToInitialValues()
{
	for (int index = 0; index < m_pointsCount; index++) {
		UpdatePoint(m_pointsInitial[index], index);
		if (m_pointsCallback)
			m_pointsCallback(this, GUI::EVENT_RESET, index);
	}
}

GUIWindow::GUIWindow(int framePadding, int frameSpacing, bool frameVertical)
	: m_frame(framePadding, frameSpacing, frameVertical)
	, m_background(0)
	, m_renderOriginX(0)
	, m_renderOriginY(0)
	, m_scrollY(0)
	, m_mouseDown(false)
	, m_dragElement(nullptr)
	, m_dragElementLast(nullptr)
	, m_dragStartX(0)
	, m_dragStartY(0)
	, m_hoverElement(nullptr)
{}

namespace GUI
{
	static std::map<int,GUIWindow*> g_GUIWindowMap;
}

void GUI::RegisterWindow(GUIWindow* window, const Pixel32& background)
{
	const int windowID = glutGetWindow();
	if (window) {
		DEBUG_ASSERT(g_GUIWindowMap.find(windowID) == g_GUIWindowMap.end());
		window->m_background = background;
		g_GUIWindowMap[windowID] = window;
	}
}

int GUI::RegisterNewWindow(GUIWindow* window, const char* title)
{
	class callbacks
	{
	public:
		static void ReshapeFunc(int width, int height)
		{
			glutReshapeWindow(width, height);
		}

		static void DisplayFunc()
		{
			Vec4V color;
			glGetFloatv(GL_COLOR_CLEAR_VALUE, (GLfloat*)&color);
			glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);
			glClearColor(VEC4V_ARGS(color)); // restore
			RenderWindow(0, 0);
			glutSwapBuffers();
		}

		static void IdleFunc()
		{
			glutPostRedisplay();
		}

		static void KeyboardFunc(unsigned char key, int x, int y)
		{
			if (key == 0x1B) // ESC
				exit(0);
		}

		static void MouseFunc(int button, int state, int x, int y)
		{
			const uint32 keyboardModifiers = 0;
			GUI::MouseButton(button, state, x, y, keyboardModifiers);
		}

		static void MotionFunc(int x, int y)
		{
			GUI::MouseMotion(x, y);
		}

		static void VisibilityFunc(int vis)
		{
			if (vis == GLUT_VISIBLE)
				glutIdleFunc(IdleFunc);
			else
				glutIdleFunc(NULL);
		}
	};
 
	static int windowPosX = 10;
	static int windowPosY = 10;
	windowPosX += 20;
	windowPosY += 20;
	glutInitDisplayMode(GLUT_DOUBLE); // no depth
	glutInitWindowSize(window->m_frame.GetW(), window->m_frame.GetH());
	glutInitWindowPosition(windowPosX, windowPosY);
	const int prevWindowID = glutGetWindow();
	const int windowID = glutCreateWindow(title);
	glutReshapeFunc(callbacks::ReshapeFunc);
	glutDisplayFunc(callbacks::DisplayFunc);
	glutKeyboardFunc(callbacks::KeyboardFunc);
	glutMouseFunc(callbacks::MouseFunc);
	glutMotionFunc(callbacks::MotionFunc);
	glutPassiveMotionFunc(callbacks::MotionFunc);
	glutVisibilityFunc(callbacks::VisibilityFunc);
	RegisterWindow(window);
	glutSetWindow(prevWindowID);
	return windowID;
}

GUIWindow* GUI::GetWindowFromID(int windowID)
{
	const auto it = g_GUIWindowMap.find(windowID);
	if (it != g_GUIWindowMap.end())
		return it->second;
	else
		return nullptr;
}

GUIWindow* GUI::GetCurrentWindow()
{
	return GetWindowFromID(glutGetWindow());
}

bool& GUI::GetTrackValuesRef()
{
	return g_trackValues;
}

void GUI::Idle(const Keyboard* keyboard)
{
	GUIWindow* window = GetCurrentWindow();
	if (window) {
		window->m_frame.Idle();
		if (keyboard) {
			const float singleStep = 1.0f;
			const float bigStep = 10.0f;
			if      (keyboard->IsKeyPressed(VK_LEFT))                            HandleSliderArrowKey(-singleStep);
			else if (keyboard->IsKeyPressed(VK_LEFT, Keyboard::MODIFIER_SHIFT))  HandleSliderArrowKey(-bigStep);
			else if (keyboard->IsKeyPressed(VK_RIGHT))                           HandleSliderArrowKey(+singleStep);
			else if (keyboard->IsKeyPressed(VK_RIGHT, Keyboard::MODIFIER_SHIFT)) HandleSliderArrowKey(+bigStep);
			else if (keyboard->IsKeyPressed(VK_UP))                              window->m_scrollY += 16;
			else if (keyboard->IsKeyPressed(VK_UP, Keyboard::MODIFIER_SHIFT))    window->m_scrollY += 64;
			else if (keyboard->IsKeyPressed(VK_DOWN))                            window->m_scrollY -= 16;
			else if (keyboard->IsKeyPressed(VK_DOWN, Keyboard::MODIFIER_SHIFT))  window->m_scrollY -= 64;
			else if (keyboard->IsKeyPressed(VK_BACKTICK_TILDE))                  HandleSliderKeyboardInput();
		}
	}
}

bool GUI::MouseButton(int button, int state, int x, int y, uint32 keyboardModifiers)
{
	if (Marquee::MouseButton(button, state, x, y))
		return true;
	else if (keyboardModifiers == 0) {
		GUIWindow* window = GetCurrentWindow();
		if (window) {
			x -= window->m_renderOriginX;
			y -= window->m_renderOriginY + window->m_scrollY;
			if (button == GLUT_LEFT_BUTTON) {
				if (state == GLUT_DOWN) {
					window->m_mouseDown = true;
					GUIElementTrack dragElement = window->m_frame.BeginDrag(x, y, window->m_dragStartX, window->m_dragStartY);
					if (dragElement) {
						window->m_dragElementLast = dragElement.m_element;
						window->m_dragElement = dragElement;
						return true;
					}
				} else if (state == GLUT_UP) {
					window->m_mouseDown = false;
					if (window->m_dragElement) {
						window->m_dragElement.EndDrag(x, y);
						window->m_dragElement = nullptr;
						return true;
					}
				}
			} else if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) { // reset to initial value
				GUIElementTrack element = window->m_frame.BeginDrag(x, y, window->m_dragStartX, window->m_dragStartY);
				if (element) {
					GUISliderElement* slider = dynamic_cast<GUISliderElement*>(element.m_element);
					if (slider) {
						slider->ResetToInitialValues();
						return true;
					}
				}
			}
		}
	}
	return false;
}

bool GUI::MouseMotion(int x, int y)
{
	if (Marquee::MouseMotion(x, y))
		return true;
	GUIWindow* window = GetCurrentWindow();
	if (window) {
		x -= window->m_renderOriginX;
		y -= window->m_renderOriginY + window->m_scrollY;
		if (window->m_mouseDown)
			return window->m_dragElement.UpdateDrag(x, y, window->m_dragStartX, window->m_dragStartY);
		else {
			window->m_hoverElement.SetState(GUI::STATE_DEFAULT);
			window->m_hoverElement = window->m_frame.Hover(x, y);
			window->m_hoverElement.SetState(GUI::STATE_HOVER);
		}
	}
	return false;
}

void GUI::RenderBegin(int x, int y, int w, int h)
{
	GLint viewport[4]; // x,y,w,h
	glGetIntegerv(GL_VIEWPORT, viewport);
	if (w == 0)
		w = viewport[2];
	if (h == 0)
		h = viewport[3];
	glViewport(x, viewport[3] - h - y, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0f, (float)w, (float)h, 0.0f, 1.0f, -1.0f); // now we can draw in integer pixel coords
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void GUI::RenderWindow(int x, int y, int w, int h)
{
	Marquee::Render();
	GUIWindow* window = GetCurrentWindow();
	if (window) {
		if (w == 0)
			w = window->m_frame.GetW();
		if (h == 0)
			h = window->m_frame.GetH();
		GLint viewport[4]; // x,y,w,h
		glGetIntegerv(GL_VIEWPORT, viewport);
		RenderBegin(x, y, w, h);
		GUIBoxElement(w, h, window->m_background, 0, 0).Render(0, 0); // clear background
		window->m_renderOriginX = x;
		window->m_renderOriginY = y;
		window->m_frame.Render(0, window->m_scrollY);
		glViewport(viewport[0], viewport[1], viewport[2], viewport[3]); // restore
	}
}

void GUI::RenderWindow(eGUIAlign align, int dx, int dy)
{
	GUIWindow* window = GetCurrentWindow();
	if (window) {
		GLint viewport[4]; // x,y,w,h
		glGetIntegerv(GL_VIEWPORT, viewport);
		const int w = viewport[2] - window->m_frame.GetW();
		const int h = viewport[3] - window->m_frame.GetH();
		switch (align) {
		case ALIGN_TOPLEFT    : RenderWindow(dx + 0, dy + 0); break;
		case ALIGN_TOPRIGHT   : RenderWindow(dx + w, dy + 0); break;
		case ALIGN_BOTTOMLEFT : RenderWindow(dx + 0, dy + h); break;
		case ALIGN_BOTTOMRIGHT: RenderWindow(dx + w, dy + h); break;
		}
	}
}

void GUI::HandleSliderArrowKey(float delta)
{
	GUIWindow* window = GetCurrentWindow();
	if (window) {
		GUISliderElement* slider = dynamic_cast<GUISliderElement*>(window->m_dragElementLast);
		if (slider) {
			const float valuePrev = slider->m_valueCurrent;
			const float valueCurr = Clamp(valuePrev + delta*slider->m_step, slider->m_valueMin, slider->m_valueMax);
			if (valueCurr != valuePrev) {
				slider->UpdateValueInternal((float)valueCurr);
				if (slider->m_valueCallback)
					slider->m_valueCallback(slider, GUI::EVENT_END_DRAG);
				// note: was EVENT_CHANGE, but this causes problems when i filter out
				// EVENT_CHANGE in my callbacks to prevent feedback loops
			}
		}
	}
}

static const std::string PromptForConsoleInput(const char* format, ...)
{
	GET_STR_VARARGS(temp,256,format);
	fprintf(stdout, "%s", temp);
	fflush(stdout);
	std::cin.getline(temp, sizeof(temp));
	return temp;
}

void GUI::HandleSliderKeyboardInput()
{
	GUIWindow* window = GetCurrentWindow();
	if (window) {
		GUISliderElement* slider = dynamic_cast<GUISliderElement*>(window->m_dragElementLast);
		if (slider) {
			const float valuePrev = slider->m_valueCurrent;
			const std::string str = PromptForConsoleInput("enter new value for '%s' (currently %f): ", slider->m_numeric.m_label.c_str(), valuePrev);
			if (!str.empty()) {
				float valueCurr;
				if (strcmp(str.c_str(), "DEFAULT") == 0)
					valueCurr = slider->m_valueInitial;
				else
					valueCurr = Clamp((float)atof(str.c_str()), slider->m_valueMin, slider->m_valueMax);
				if (valueCurr != valuePrev) {
					slider->UpdateValueInternal((float)valueCurr);
					if (slider->m_valueCallback)
						slider->m_valueCallback(slider, GUI::EVENT_CHANGE);
					printf("set value to %f.\n", slider->m_valueCurrent);
				} else
					printf("no change.\n");
			}
		}
	}
}

GUIWindow* GUI::CreateFrameTest()
{
	GUIWindow* window = new GUIWindow();
	GUITextElement::GetCurrentTextColor() = Pixel32(64,64,192);
	float* sliders = new float[3];
	sliders[0] = 0.2f;
	sliders[1] = 0.7f;
	sliders[2] = 0.9f;
	window->m_frame.AddElement(new GUISliderElement("slider", sliders[0], 0.0f, 1.0f, nullptr, 0.1f, Pixel32(255,0,0)));
	window->m_frame.AddElement(new GUISliderElement("test1", sliders[1], 0.0f, 1.0f, nullptr, 0.1f, Pixel32(0,255,0)));
	window->m_frame.AddElement(new GUISliderElement("tracking", sliders[2], -1.0f, 1.0f, nullptr, 0.1f, Pixel32(0,0,255)));
	window->m_frame.AlignSliders();
	return window;
}

void GUI::PrintEvent(const char* msg, GUI::eGUIEvent event)
{
	switch (event) {
	case GUI::EVENT_NONE       : printf("%s:EVENT_NONE\n", msg); break;
	case GUI::EVENT_BEGIN_DRAG : printf("%s:EVENT_BEGIN_DRAG\n", msg); break;
	case GUI::EVENT_DRAGGING   : printf("%s:EVENT_DRAGGING\n", msg); break;
	case GUI::EVENT_END_DRAG   : printf("%s:EVENT_END_DRAG\n", msg); break;
	case GUI::EVENT_CHANGE     : printf("%s:EVENT_CHANGE\n", msg); break;
	case GUI::EVENT_RESET      : printf("%s:EVENT_RESET\n", msg); break;
	default                    : printf("%s:EVENT_UNKNOWN(%d)\n", msg, event); break;
	}
}

namespace Marquee
{
	static int g_windowID = -1;
	static bool (*g_callback)(const GUIBounds& bounds, GUI::eGUIEvent event) = nullptr;
	static bool g_visible = false;
	static bool g_dragging = false;
	static int g_dragStartX = 0;
	static int g_dragStartY = 0;
	static int g_dragCurrentX = 0;
	static int g_dragCurrentY = 0;
}

void Marquee::Register(bool (*callback)(const GUIBounds& bounds, GUI::eGUIEvent event))
{
	ForceAssert(g_windowID == -1);
	g_windowID = glutGetWindow();
	g_callback = callback;
}

void Marquee::UnRegister()
{
	g_windowID = -1;
}

GUIBounds Marquee::GetBounds()
{
	GLint viewport[4]; // x,y,w,h
	glGetIntegerv(GL_VIEWPORT, viewport);
	const int x0 = Max(Min(g_dragCurrentX, g_dragStartX), viewport[0]);
	const int y0 = Max(Min(g_dragCurrentY, g_dragStartY), viewport[1]);
	const int x1 = Min(Max(g_dragCurrentX, g_dragStartX), viewport[0] + viewport[2]);
	const int y1 = Min(Max(g_dragCurrentY, g_dragStartY), viewport[1] + viewport[3]);
	return GUIBounds(x0, y0, x1 - x0, y1 - y0);
}

void Marquee::SetVisible(bool visible)
{
	g_visible = visible;
}

bool Marquee::MouseButton(int button, int state, int x, int y)
{
	if (glutGetWindow() == g_windowID) {
		if (state == GLUT_DOWN && button == GLUT_LEFT_BUTTON) {
			bool dragging = true;
			if (g_callback)
				dragging = g_callback(GUIBounds(x, y), GUI::EVENT_BEGIN_DRAG);
			if (dragging) {
				g_dragging = g_visible = true;
				g_dragStartX = g_dragCurrentX = x;
				g_dragStartY = g_dragCurrentY = y;
				return true;
			}
		} else if (state == GLUT_UP && button == GLUT_LEFT_BUTTON) {
			if (g_dragging) {
				g_dragging = false;
				if (g_callback)
					g_visible = g_callback(GetBounds(), GUI::EVENT_END_DRAG);
				else
					g_visible = false;
				return true;
			}
		}
	}
	return false;
}

bool Marquee::MouseMotion(int x, int y)
{
	if (g_dragging && glutGetWindow() == g_windowID) {
		g_dragCurrentX = x;
		g_dragCurrentY = y;
		if (g_callback)
			g_callback(GetBounds(), GUI::EVENT_DRAGGING);
		return true;
	} else
		return false;
}

void Marquee::Render(const Pixel32& fillColor, const Pixel32& edgeColor)
{
	if (g_visible && glutGetWindow() == g_windowID) {
		const GUIBounds b = GetBounds();
		GUI::RenderBegin(0, 0, 0, 0);
		GUIBoxElement(b.GetW(), b.GetH(), fillColor, edgeColor, 0).Render(b.GetX(), b.GetY());
	}
}

#endif // defined(_OPENGL)