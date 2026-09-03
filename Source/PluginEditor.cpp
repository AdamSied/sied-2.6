#include "PluginEditor.h"
#include <SiedFactoryData.h>

namespace
{
constexpr auto background = 0xff20211f;
constexpr auto panel = 0xff292b27;
constexpr auto panelRaised = 0xff33352f;
constexpr auto line = 0xff565850;
constexpr auto text = 0xfff0ebe1;
constexpr auto muted = 0xffa39f95;
constexpr auto cyan = 0xff94c8bb;
constexpr auto violet = 0xffb6a9c8;
constexpr auto coral = 0xffd39a7d;
constexpr auto ice = 0xfffffbf1;
constexpr auto ink = 0xff121311;

juce::Font uiFont (float height, int style = juce::Font::plain)
{
    return juce::Font (juce::FontOptions (height, style));
}

void setParameterValue (juce::AudioProcessorValueTreeState& state,
                        const juce::String& id, float actualValue)
{
    if (auto* parameter = state.getParameter (id))
        parameter->setValueNotifyingHost (parameter->convertTo0to1 (actualValue));
}

float normalisedValue (const juce::Slider& slider)
{
    const auto length = slider.getMaximum() - slider.getMinimum();
    return length > 0.0 ? static_cast<float> ((slider.getValue() - slider.getMinimum()) / length)
                        : 0.0f;
}

void drawSoftGlow (juce::Graphics& g, juce::Rectangle<float> area,
                   juce::Colour accent, float strength, float expansion = 18.0f)
{
    strength = juce::jlimit (0.0f, 1.0f, strength);
    if (strength <= 0.002f)
        return;
    for (int layer = 3; layer >= 0; --layer)
    {
        const auto proportion = static_cast<float> (layer + 1) / 4.0f;
        const auto glow = area.expanded (expansion * proportion);
        g.setColour (accent.withAlpha (0.010f * strength * (4.0f - layer)));
        g.fillRoundedRectangle (glow, 10.0f + expansion * proportion * 0.24f);
    }
}

void drawPanelSurface (juce::Graphics& g, juce::Rectangle<float> area,
                       juce::Colour accent, float activity, float corner = 10.0f)
{
    activity = juce::jlimit (0.0f, 1.0f, activity);
    drawSoftGlow (g, area, accent, activity * 0.18f, 6.0f);
    g.setColour (juce::Colour (panel));
    g.fillRoundedRectangle (area, corner);
    g.setColour (juce::Colour (line).interpolatedWith (accent, activity * 0.42f));
    g.drawRoundedRectangle (area.reduced (0.5f), corner, 0.75f + activity * 0.35f);

    g.setColour (accent.withAlpha (0.035f + activity * 0.10f));
    g.fillRoundedRectangle (area.getX() + 12.0f, area.getY() + 1.0f,
                            area.getWidth() - 24.0f, 1.0f, 0.5f);
}

juce::Colour accentForLayer (SiedLayer layer)
{
    if (layer == SiedLayer::oneShotB) return juce::Colour (violet);
    if (layer == SiedLayer::texture) return juce::Colour (coral);
    return juce::Colour (cyan);
}

}

SiedLookAndFeel::SiedLookAndFeel()
{
    valueTypeface = juce::Typeface::createSystemTypefaceFor (
        SiedFactoryData::DejaVuSansMonoBold_ttf,
        static_cast<size_t> (SiedFactoryData::DejaVuSansMonoBold_ttfSize));
    setColour (juce::PopupMenu::backgroundColourId, juce::Colour (panel));
    setColour (juce::PopupMenu::textColourId, juce::Colour (text));
    setColour (juce::PopupMenu::highlightedBackgroundColourId,
               juce::Colour (cyan).withAlpha (0.18f));
    setColour (juce::PopupMenu::highlightedTextColourId, juce::Colour (text));
    setColour (juce::PopupMenu::headerTextColourId, juce::Colour (muted));
}

juce::Font SiedLookAndFeel::valueFont (float height) const
{
    return valueTypeface != nullptr
               ? juce::Font (juce::FontOptions (valueTypeface).withHeight (height))
               : uiFont (height, juce::Font::bold);
}

void SiedLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                                       float position, float startAngle, float endAngle,
                                       juce::Slider& slider)
{
    const auto hero = slider.getName() == "SHIMMER";
    const auto reactive = static_cast<bool> (
        slider.getProperties().getWithDefault ("reactiveGlow", false));
    const auto bipolar = static_cast<bool> (
        slider.getProperties().getWithDefault ("bipolarCentre", false));
    auto bounds = juce::Rectangle<float> (static_cast<float> (x), static_cast<float> (y),
                                           static_cast<float> (width), static_cast<float> (height));
    bounds = bounds.reduced (hero ? 19.0f : 11.0f);
    const auto diameter = juce::jmin (bounds.getWidth(), bounds.getHeight());
    const auto dial = juce::Rectangle<float> (diameter, diameter).withCentre (bounds.getCentre());
    const auto radius = diameter * 0.5f;
    const auto centre = dial.getCentre();
    const auto angle = startAngle + position * (endAngle - startAngle);
    const auto accent = slider.findColour (juce::Slider::thumbColourId);
    const auto activity = bipolar ? std::abs (position - 0.5f) * 2.0f : position;
    const auto glow = reactive ? std::pow (juce::jlimit (0.0f, 1.0f, activity), 0.82f)
                               : 0.0f;

    if (glow > 0.001f)
        drawSoftGlow (g, dial, accent, (hero ? 0.72f : 0.38f) * glow,
                      (hero ? 24.0f : 10.0f) * glow + 4.0f);

    const auto tickRadius = radius - (hero ? 2.0f : 1.5f);
    const auto tickCount = hero ? 21 : 13;
    for (int tick = 0; tick < tickCount; ++tick)
    {
        const auto tickPosition = static_cast<float> (tick) / static_cast<float> (tickCount - 1);
        const auto tickAngle = startAngle + tickPosition * (endAngle - startAngle);
        const auto major = tick == 0 || tick == tickCount - 1 || tick == (tickCount - 1) / 2;
        const auto active = bipolar ? (position >= 0.5f
                                           ? tickPosition >= 0.5f && tickPosition <= position
                                           : tickPosition <= 0.5f && tickPosition >= position)
                                    : tickPosition <= position;
        const auto outer = centre + juce::Point<float> (std::sin (tickAngle),
                                                        -std::cos (tickAngle)) * tickRadius;
        const auto inner = centre + juce::Point<float> (std::sin (tickAngle),
                                                        -std::cos (tickAngle))
                                      * (tickRadius - (major ? 5.0f : 3.0f));
        g.setColour (active ? accent.withAlpha (0.48f + glow * 0.46f)
                            : juce::Colour (line).withAlpha (major ? 0.78f : 0.46f));
        g.drawLine ({ inner, outer }, major ? 1.35f : 0.72f);
    }

    juce::Path track;
    track.addCentredArc (centre.x, centre.y, radius - (hero ? 9.5f : 6.5f),
                         radius - (hero ? 9.5f : 6.5f), 0.0f,
                         startAngle, endAngle, true);
    g.setColour (juce::Colour (line).withAlpha (0.72f));
    g.strokePath (track, juce::PathStrokeType (hero ? 6.5f : 4.0f,
                                               juce::PathStrokeType::curved,
                                               juce::PathStrokeType::rounded));

    juce::Path valueArc;
    const auto valueStart = bipolar && position < 0.5f
                                ? angle : bipolar ? startAngle + 0.5f * (endAngle - startAngle)
                                                  : startAngle;
    const auto valueEnd = bipolar && position < 0.5f
                              ? startAngle + 0.5f * (endAngle - startAngle) : angle;
    valueArc.addCentredArc (centre.x, centre.y, radius - (hero ? 9.5f : 6.5f),
                            radius - (hero ? 9.5f : 6.5f), 0.0f,
                            valueStart, valueEnd, true);
    g.setColour (accent.withAlpha (0.30f + glow * 0.62f));
    g.strokePath (valueArc, juce::PathStrokeType (hero ? 6.5f : 3.6f,
                                                  juce::PathStrokeType::curved,
                                                  juce::PathStrokeType::rounded));

    const auto face = dial.reduced (hero ? 19.0f : 12.0f);
    juce::ColourGradient faceGradient (juce::Colour (panelRaised).brighter (0.05f),
                                       face.getX(), face.getY(), juce::Colour (ink),
                                       face.getX(), face.getBottom(), false);
    faceGradient.addColour (0.62, juce::Colour (panel));
    g.setGradientFill (faceGradient);
    g.fillEllipse (face);
    g.setColour (juce::Colour (line).interpolatedWith (accent, glow * 0.26f));
    g.drawEllipse (face, hero ? 1.4f : 0.9f);
    juce::Path faceHighlight;
    faceHighlight.addCentredArc (face.getCentreX(), face.getCentreY(),
                                 face.getWidth() * 0.5f - 2.0f,
                                 face.getHeight() * 0.5f - 2.0f, 0.0f,
                                 juce::MathConstants<float>::pi * 1.08f,
                                 juce::MathConstants<float>::pi * 1.92f, true);
    g.setColour (juce::Colours::white.withAlpha (0.025f));
    g.strokePath (faceHighlight, juce::PathStrokeType (1.0f));

    juce::Path marker;
    const auto markerLength = radius * (hero ? 0.46f : 0.43f);
    const auto markerStart = radius * (hero ? 0.15f : 0.13f);
    marker.startNewSubPath (centre.x + std::sin (angle) * markerStart,
                            centre.y - std::cos (angle) * markerStart);
    marker.lineTo (centre.x + std::sin (angle) * markerLength,
                   centre.y - std::cos (angle) * markerLength);
    g.setColour (accent.brighter (0.32f).withAlpha (0.82f + glow * 0.18f));
    g.strokePath (marker, juce::PathStrokeType (hero ? 3.0f : 2.0f,
                                                juce::PathStrokeType::curved,
                                                juce::PathStrokeType::rounded));
    const auto capSize = hero ? 8.0f : 5.0f;
    g.setColour (juce::Colour (ink));
    g.fillEllipse (centre.x - capSize * 0.5f, centre.y - capSize * 0.5f,
                   capSize, capSize);
    g.setColour (accent.withAlpha (0.62f + glow * 0.38f));
    g.drawEllipse (centre.x - capSize * 0.5f, centre.y - capSize * 0.5f,
                   capSize, capSize, hero ? 1.6f : 1.0f);
}

void SiedLookAndFeel::drawLinearSlider (juce::Graphics& g, int x, int y, int width, int height,
                                        float sliderPosition, float minSliderPosition,
                                        float maxSliderPosition,
                                        juce::Slider::SliderStyle style, juce::Slider& slider)
{
    if (style != juce::Slider::LinearHorizontal)
    {
        juce::LookAndFeel_V4::drawLinearSlider (g, x, y, width, height, sliderPosition,
                                                 minSliderPosition, maxSliderPosition,
                                                 style, slider);
        return;
    }
    const auto centreY = static_cast<float> (y + height / 2);
    const auto track = juce::Rectangle<float> (static_cast<float> (x + 7), centreY - 2.0f,
                                                static_cast<float> (width - 14), 4.0f);
    g.setColour (slider.findColour (juce::Slider::backgroundColourId));
    g.fillRoundedRectangle (track, 2.0f);
    const auto filled = track.withRight (juce::jlimit (track.getX(), track.getRight(),
                                                       sliderPosition));
    g.setColour (slider.findColour (juce::Slider::trackColourId).withAlpha (0.72f));
    g.fillRoundedRectangle (filled, 2.0f);
    const auto thumb = juce::Rectangle<float> (10.0f, 18.0f)
                           .withCentre ({ sliderPosition, centreY });
    g.setColour (juce::Colour (ink));
    g.fillRoundedRectangle (thumb, 4.0f);
    g.setColour (slider.findColour (juce::Slider::thumbColourId));
    g.drawRoundedRectangle (thumb.reduced (0.5f), 4.0f, 1.0f);
}

void SiedLookAndFeel::drawButtonBackground (juce::Graphics& g, juce::Button& button,
                                            const juce::Colour&, bool highlighted, bool down)
{
    const auto accentButton = button.getProperties().getWithDefault ("accent", false);
    const auto active = static_cast<bool> (accentButton) || button.getToggleState();
    const auto bounds = button.getLocalBounds().toFloat().reduced (0.5f);
    const auto tab = static_cast<bool> (button.getProperties().getWithDefault ("tab", false));
    if (active)
        drawSoftGlow (g, bounds, juce::Colour (cyan), highlighted ? 0.34f : 0.18f, 6.0f);
    auto topColour = active ? juce::Colour (cyan).withMultipliedSaturation (0.40f).darker (0.58f)
                            : juce::Colour (panelRaised);
    auto bottomColour = active ? topColour.darker (0.06f) : juce::Colour (panel);
    if (highlighted)
    {
        topColour = topColour.brighter (0.08f);
        bottomColour = bottomColour.brighter (0.05f);
    }
    if (down)
    {
        topColour = topColour.darker (0.16f);
        bottomColour = bottomColour.darker (0.12f);
    }
    juce::ColourGradient surface (topColour, bounds.getX(), bounds.getY(), bottomColour,
                                  bounds.getX(), bounds.getBottom(), false);
    g.setGradientFill (surface);
    g.fillRoundedRectangle (bounds, tab ? 6.0f : 8.0f);
    g.setColour (active ? juce::Colour (cyan).withAlpha (0.66f)
                        : juce::Colour (line).withAlpha (0.92f));
    g.drawRoundedRectangle (bounds, tab ? 6.0f : 8.0f, active ? 1.25f : 0.85f);
    g.setColour (juce::Colours::white.withAlpha (highlighted ? 0.045f : 0.015f));
    g.fillRoundedRectangle (bounds.getX() + 8.0f, bounds.getY() + 1.0f,
                            bounds.getWidth() - 16.0f, 1.0f, 0.5f);
    if (active)
    {
        juce::ColourGradient signal (juce::Colour (cyan).withAlpha (0.0f), bounds.getX(), 0.0f,
                                     juce::Colour (cyan).withAlpha (0.72f),
                                     bounds.getCentreX(), 0.0f, false);
        signal.addColour (1.0, juce::Colour (cyan).withAlpha (0.0f));
        g.setGradientFill (signal);
        g.fillRoundedRectangle (bounds.getX() + 9.0f, bounds.getBottom() - 1.5f,
                                bounds.getWidth() - 18.0f, 1.5f, 0.75f);
    }
}

void SiedLookAndFeel::drawButtonText (juce::Graphics& g, juce::TextButton& button,
                                      bool, bool)
{
    if (button.getProperties().getWithDefault ("undo", false))
    {
        const auto bounds = button.getLocalBounds().toFloat();
        const auto centre = bounds.getCentre();
        juce::Path arrow;
        arrow.startNewSubPath (centre.x - 7.5f, centre.y - 1.0f);
        arrow.lineTo (centre.x - 2.0f, centre.y - 6.0f);
        arrow.lineTo (centre.x - 2.0f, centre.y - 2.5f);
        arrow.cubicTo (centre.x + 7.0f, centre.y - 3.0f,
                       centre.x + 9.0f, centre.y + 6.0f,
                       centre.x + 2.5f, centre.y + 7.0f);
        g.setColour (button.isEnabled() ? juce::Colour (text)
                                        : juce::Colour (muted).withAlpha (0.38f));
        g.strokePath (arrow, juce::PathStrokeType (1.8f,
                                                    juce::PathStrokeType::curved,
                                                    juce::PathStrokeType::rounded));
        return;
    }

    const auto arrowDirection = static_cast<int> (
        button.getProperties().getWithDefault ("arrowDirection", 0));
    if (arrowDirection != 0)
    {
        const auto centre = button.getLocalBounds().toFloat().getCentre();
        juce::Path arrow;
        const auto direction = static_cast<float> (arrowDirection);
        arrow.startNewSubPath (centre.x - direction * 3.0f, centre.y - 5.0f);
        arrow.lineTo (centre.x + direction * 2.0f, centre.y);
        arrow.lineTo (centre.x - direction * 3.0f, centre.y + 5.0f);
        g.setColour (juce::Colour (text));
        g.strokePath (arrow, juce::PathStrokeType (1.7f,
                                                    juce::PathStrokeType::curved,
                                                    juce::PathStrokeType::rounded));
        return;
    }

    if (button.getProperties().getWithDefault ("dice", false))
    {
        const auto side = juce::jmin (34.0f,
                                     static_cast<float> (juce::jmin (button.getWidth(),
                                                                     button.getHeight()) - 12));
        const auto die = juce::Rectangle<float> (side, side)
                             .withCentre (button.getLocalBounds().toFloat().getCentre());
        drawSoftGlow (g, die, juce::Colour (cyan), 0.48f, 8.0f);
        g.setColour (juce::Colour (ink));
        g.fillRoundedRectangle (die, 7.0f);
        g.setColour (juce::Colour (cyan));
        g.drawRoundedRectangle (die.reduced (0.5f), 7.0f, 1.3f);
        const auto pipRadius = 1.75f;
        const std::array<juce::Point<float>, 5> pips
        {{
            { die.getX() + 8.0f, die.getY() + 8.0f },
            { die.getRight() - 8.0f, die.getY() + 8.0f },
            die.getCentre(),
            { die.getX() + 8.0f, die.getBottom() - 8.0f },
            { die.getRight() - 8.0f, die.getBottom() - 8.0f }
        }};
        for (const auto& pip : pips)
            g.fillEllipse (pip.x - pipRadius, pip.y - pipRadius,
                           pipRadius * 2.0f, pipRadius * 2.0f);
        return;
    }

    auto content = button.getLocalBounds().reduced (10, 2);
    g.setColour (button.findColour (button.getToggleState()
                                        ? juce::TextButton::textColourOnId
                                        : juce::TextButton::textColourOffId));
    g.setFont (getTextButtonFont (button, button.getHeight()));
    g.drawFittedText (button.getButtonText(), content,
                      juce::Justification::centred, 1);
}

void SiedLookAndFeel::drawComboBox (juce::Graphics& g, int width, int height, bool isButtonDown,
                                    int, int, int, int, juce::ComboBox& box)
{
    auto bounds = juce::Rectangle<float> (0.5f, 0.5f, static_cast<float> (width - 1),
                                           static_cast<float> (height - 1));
    g.setColour (isButtonDown ? juce::Colour (panelRaised).brighter (0.05f)
                              : juce::Colour (panelRaised));
    g.fillRoundedRectangle (bounds, 7.0f);
    g.setColour (box.findColour (juce::ComboBox::outlineColourId));
    g.drawRoundedRectangle (bounds, 7.0f, 1.0f);
    g.setColour (juce::Colours::white.withAlpha (0.035f));
    g.fillRoundedRectangle (bounds.getX() + 8.0f, bounds.getY() + 1.0f,
                            bounds.getWidth() - 31.0f, 1.0f, 0.5f);
    g.setColour (juce::Colour (line).withAlpha (0.65f));
    g.drawVerticalLine (width - 30, 6.0f, static_cast<float> (height - 6));

    juce::Path arrow;
    const auto centreX = static_cast<float> (width - 16);
    const auto centreY = static_cast<float> (height) * 0.5f;
    arrow.startNewSubPath (centreX - 4.0f, centreY - 2.0f);
    arrow.lineTo (centreX, centreY + 2.0f);
    arrow.lineTo (centreX + 4.0f, centreY - 2.0f);
    g.setColour (isButtonDown ? juce::Colour (cyan) : juce::Colour (muted));
    g.strokePath (arrow, juce::PathStrokeType (1.5f, juce::PathStrokeType::curved,
                                               juce::PathStrokeType::rounded));
}

void SiedLookAndFeel::drawToggleButton (juce::Graphics& g, juce::ToggleButton& button,
                                        bool highlighted, bool down)
{
    auto bounds = button.getLocalBounds().toFloat().reduced (0.5f);
    const auto on = button.getToggleState();
    if (button.getProperties().getWithDefault ("lockButton", false))
    {
        if (on)
            drawSoftGlow (g, bounds, juce::Colour (cyan), 0.28f, 4.0f);
        g.setColour (highlighted ? juce::Colour (panelRaised).brighter (0.08f)
                                 : juce::Colour (panel));
        g.fillRoundedRectangle (bounds, 5.0f);
        g.setColour (on ? juce::Colour (cyan) : juce::Colour (muted).withAlpha (0.54f));
        g.drawRoundedRectangle (bounds, 5.0f, on ? 1.1f : 0.8f);
        const auto cx = bounds.getCentreX();
        const auto body = juce::Rectangle<float> (cx - 4.4f, bounds.getCentreY() - 0.5f,
                                                   8.8f, 7.0f);
        g.fillRoundedRectangle (body, 1.8f);
        juce::Path shackle;
        shackle.startNewSubPath (cx - 3.2f, body.getY() + 0.5f);
        shackle.cubicTo (cx - 3.2f, body.getY() - 5.0f, cx + 3.2f, body.getY() - 5.0f,
                         cx + 3.2f, body.getY() + (on ? 0.5f : -1.2f));
        g.strokePath (shackle, juce::PathStrokeType (1.5f));
        if (down)
        {
            g.setColour (juce::Colours::black.withAlpha (0.18f));
            g.fillRoundedRectangle (bounds, 5.0f);
        }
        return;
    }
    if (on)
        drawSoftGlow (g, bounds, juce::Colour (cyan), highlighted ? 0.48f : 0.26f, 6.0f);
    auto fill = on ? juce::Colour (cyan).darker (0.58f) : juce::Colour (panelRaised);
    if (highlighted) fill = fill.brighter (0.08f);
    if (down) fill = fill.darker (0.12f);
    g.setColour (fill);
    g.fillRoundedRectangle (bounds, 8.0f);
    g.setColour (on ? juce::Colour (cyan) : juce::Colour (line));
    g.drawRoundedRectangle (bounds, 8.0f, on ? 1.25f : 0.8f);
    const auto lightSize = juce::jmin (6.0f, bounds.getHeight() * 0.22f);
    g.setColour (on ? juce::Colour (ice) : juce::Colour (muted).withAlpha (0.36f));
    g.fillEllipse (bounds.getX() + 9.0f, bounds.getCentreY() - lightSize * 0.5f,
                   lightSize, lightSize);
    g.setColour (on ? juce::Colour (text) : juce::Colour (muted));
    g.setFont (valueFont (11.0f));
    g.drawText (button.getButtonText(), button.getLocalBounds().withTrimmedLeft (8),
                juce::Justification::centred);
}

juce::Font SiedLookAndFeel::getTextButtonFont (juce::TextButton&, int buttonHeight)
{
    return valueFont (juce::jmin (11.5f, static_cast<float> (buttonHeight) * 0.34f));
}

juce::Font SiedLookAndFeel::getComboBoxFont (juce::ComboBox&)
{
    return valueFont (11.5f);
}

juce::Font SiedLookAndFeel::getPopupMenuFont()
{
    return valueFont (12.0f);
}

juce::Font SiedLookAndFeel::getLabelFont (juce::Label& label)
{
    if (dynamic_cast<juce::Slider*> (label.getParentComponent()) != nullptr)
        return valueFont (10.5f);
    return valueFont (11.5f);
}

juce::Font SiedLookAndFeel::getSliderPopupFont (juce::Slider&)
{
    return valueFont (14.0f);
}

void SiedLookAndFeel::drawPopupMenuItem (juce::Graphics& g,
                                         const juce::Rectangle<int>& area,
                                         bool isSeparator, bool isActive,
                                         bool isHighlighted, bool isTicked,
                                         bool hasSubMenu, const juce::String& itemText,
                                         const juce::String& shortcutKeyText,
                                         const juce::Drawable* icon,
                                         const juce::Colour* itemTextColour)
{
    if (isSeparator)
    {
        g.setColour (juce::Colour (line));
        g.fillRect (area.reduced (10, 0).withHeight (1).withCentre (area.getCentre()));
        return;
    }

    const auto item = area.reduced (4, 2);
    if (isHighlighted && isActive)
    {
        g.setColour (juce::Colour (cyan).withAlpha (0.17f));
        g.fillRoundedRectangle (item.toFloat(), 4.0f);
    }

    auto colour = itemTextColour != nullptr ? *itemTextColour : juce::Colour (text);
    if (! isActive)
        colour = juce::Colour (muted).withAlpha (0.54f);
    g.setColour (colour);
    g.setFont (valueFont (11.5f));

    auto content = item.reduced (10, 0);
    if (isTicked)
    {
        g.setColour (juce::Colour (cyan));
        g.fillEllipse (static_cast<float> (content.getX()),
                       static_cast<float> (content.getCentreY() - 3), 6.0f, 6.0f);
    }
    if (icon != nullptr)
        icon->drawWithin (g, content.removeFromLeft (18).toFloat(),
                          juce::RectanglePlacement::centred, 1.0f);
    else
        content.removeFromLeft (isTicked ? 14 : 2);

    g.setColour (colour);
    g.drawFittedText (itemText, content, juce::Justification::centredLeft, 1);
    if (shortcutKeyText.isNotEmpty())
        g.drawText (shortcutKeyText, content, juce::Justification::centredRight);
    if (hasSubMenu)
    {
        juce::Path arrow;
        const auto x = static_cast<float> (content.getRight() - 3);
        const auto y = static_cast<float> (content.getCentreY());
        arrow.startNewSubPath (x - 4.0f, y - 4.0f);
        arrow.lineTo (x, y);
        arrow.lineTo (x - 4.0f, y + 4.0f);
        g.strokePath (arrow, juce::PathStrokeType (1.4f));
    }
}

void SiedLookAndFeel::drawPopupMenuBackground (juce::Graphics& g, int width, int height)
{
    const auto bounds = juce::Rectangle<float> (0.0f, 0.0f,
                                                 static_cast<float> (width),
                                                 static_cast<float> (height));
    g.setColour (juce::Colour (panel));
    g.fillRect (bounds);
    g.setColour (juce::Colour (cyan).withAlpha (0.28f));
    g.fillRect (0.0f, 0.0f, static_cast<float> (width), 1.0f);
    g.setColour (juce::Colour (line));
    g.drawRect (bounds, 1.0f);
}

void SiedLookAndFeel::drawLabel (juce::Graphics& g, juce::Label& label)
{
    const auto* slider = dynamic_cast<juce::Slider*> (label.getParentComponent());
    const auto bounds = label.getLocalBounds().toFloat();
    if (slider != nullptr)
    {
        const auto accent = slider->findColour (juce::Slider::thumbColourId);
        const auto pill = bounds.reduced (juce::jmax (1.0f, bounds.getWidth() * 0.10f), 0.5f);
        g.setColour (juce::Colour (panelRaised));
        g.fillRoundedRectangle (pill, pill.getHeight() * 0.38f);
        g.setColour (juce::Colour (line).interpolatedWith (accent, 0.22f));
        g.drawRoundedRectangle (pill, pill.getHeight() * 0.38f, 0.75f);
    }

    if (! label.isBeingEdited())
    {
        const auto alpha = label.isEnabled() ? 1.0f : 0.45f;
        g.setColour (label.findColour (juce::Label::textColourId).withMultipliedAlpha (alpha));
        g.setFont (getLabelFont (label));
        auto textArea = label.getBorderSize().subtractedFrom (label.getLocalBounds());
        g.drawFittedText (label.getText(), textArea, label.getJustificationType(),
                          juce::jmax (1, static_cast<int> (textArea.getHeight()
                                                          / getLabelFont (label).getHeight())),
                          label.getMinimumHorizontalScale());
    }
}

void SiedLookAndFeel::drawTooltip (juce::Graphics& g, const juce::String& tooltipText,
                                   int width, int height)
{
    const auto bounds = juce::Rectangle<float> (0.5f, 0.5f,
                                                 static_cast<float> (width - 1),
                                                 static_cast<float> (height - 1));
    g.setColour (juce::Colours::black.withAlpha (0.32f));
    g.fillRoundedRectangle (bounds.translated (0.0f, 2.0f), 7.0f);
    g.setColour (juce::Colour (panel));
    g.fillRoundedRectangle (bounds, 7.0f);
    g.setColour (juce::Colour (cyan).withAlpha (0.48f));
    g.drawRoundedRectangle (bounds, 7.0f, 0.9f);
    g.setColour (juce::Colour (text));
    g.setFont (valueFont (12.0f));
    g.drawFittedText (tooltipText, juce::Rectangle<int> (10, 4, width - 20, height - 8),
                      juce::Justification::centred, 2);
}

SampleWaveformComponent::SampleWaveformComponent (SiedAudioProcessor& owner,
                                                   SiedLayer sampleLayer,
                                                   const juce::String& startParameterID,
                                                   const juce::String& endParameterID)
    : processor (owner), layer (sampleLayer)
{
    startParameter = processor.getParameters().getParameter (startParameterID);
    if (endParameterID.isNotEmpty())
        endParameter = processor.getParameters().getParameter (endParameterID);
    setMouseCursor (juce::MouseCursor::LeftRightResizeCursor);
    startTimerHz (60);
    rebuildWaveform();
}

SampleWaveformComponent::~SampleWaveformComponent()
{
    stopTimer();
}

juce::Rectangle<float> SampleWaveformComponent::getWaveformBounds() const
{
    return getLocalBounds().toFloat().reduced (9.0f, 8.0f);
}

float SampleWaveformComponent::getStart() const
{
    return startParameter != nullptr ? startParameter->getValue() : 0.0f;
}

float SampleWaveformComponent::getEnd() const
{
    return endParameter != nullptr ? endParameter->getValue() : 1.0f;
}

void SampleWaveformComponent::paint (juce::Graphics& g)
{
    const auto bounds = getWaveformBounds();
    const auto outer = getLocalBounds().toFloat();
    const auto accent = accentForLayer (layer);
    g.setColour (juce::Colour (ink).withAlpha (0.82f));
    g.fillRoundedRectangle (outer, 8.0f);
    g.setColour (juce::Colour (line).interpolatedWith (accent, 0.12f));
    g.drawRoundedRectangle (outer.reduced (0.5f), 8.0f, 0.9f);

    g.setColour (juce::Colour (line).withAlpha (0.24f));
    g.drawHorizontalLine (static_cast<int> (bounds.getCentreY()), bounds.getX(), bounds.getRight());
    for (int division = 1; division < 8; ++division)
    {
        const auto x = bounds.getX() + bounds.getWidth() * static_cast<float> (division) / 8.0f;
        g.setColour (juce::Colour (line).withAlpha (division % 2 == 0 ? 0.26f : 0.13f));
        g.drawVerticalLine (static_cast<int> (x), bounds.getY(), bounds.getBottom());
    }

    if (waveformPeaks.empty())
    {
        g.setColour (juce::Colour (muted));
        g.setFont (uiFont (11.0f));
        g.drawText ("DROP OR LOAD AUDIO", getLocalBounds(), juce::Justification::centred);
        return;
    }

    juce::Path waveformFill;
    juce::Path waveformTop;
    const auto centreY = bounds.getCentreY();
    waveformFill.startNewSubPath (bounds.getX(), centreY);
    for (size_t i = 0; i < waveformPeaks.size(); ++i)
    {
        const auto x = juce::jmap (static_cast<float> (i), 0.0f,
                                   static_cast<float> (waveformPeaks.size() - 1),
                                   bounds.getX(), bounds.getRight());
        const auto peakHeight = waveformPeaks[i] * bounds.getHeight() * 0.43f;
        if (i == 0) waveformTop.startNewSubPath (x, centreY - peakHeight);
        else        waveformTop.lineTo (x, centreY - peakHeight);
        waveformFill.lineTo (x, centreY - peakHeight);
    }
    for (size_t i = waveformPeaks.size(); i-- > 0;)
    {
        const auto x = juce::jmap (static_cast<float> (i), 0.0f,
                                   static_cast<float> (waveformPeaks.size() - 1),
                                   bounds.getX(), bounds.getRight());
        waveformFill.lineTo (x, centreY + waveformPeaks[i] * bounds.getHeight() * 0.43f);
    }
    waveformFill.closeSubPath();
    juce::ColourGradient waveformGradient (accent.withAlpha (0.40f), bounds.getCentreX(),
                                            bounds.getY(), accent.withAlpha (0.055f),
                                            bounds.getCentreX(), bounds.getBottom(), false);
    g.setGradientFill (waveformGradient);
    g.fillPath (waveformFill);
    g.setColour (accent.withAlpha (0.86f));
    g.strokePath (waveformTop, juce::PathStrokeType (0.9f, juce::PathStrokeType::curved));

    const auto startX = bounds.getX() + bounds.getWidth() * getStart();
    const auto endX = bounds.getX() + bounds.getWidth() * getEnd();
    g.setColour (juce::Colour (background).withAlpha (0.72f));
    g.fillRect (bounds.withRight (startX));
    if (endParameter != nullptr)
        g.fillRect (bounds.withLeft (endX));
    g.setColour (accent.brighter (0.15f));
    g.fillRect (juce::Rectangle<float> (startX - 1.0f, bounds.getY(), 2.0f, bounds.getHeight()));
    if (endParameter != nullptr)
    {
        g.setColour (accent.interpolatedWith (juce::Colour (violet), 0.42f));
        g.fillRect (juce::Rectangle<float> (endX - 1.0f, bounds.getY(), 2.0f, bounds.getHeight()));
    }

    const auto playhead = animatedPlayhead;
    if (playhead >= 0.0f)
    {
        const auto playheadX = bounds.getX() + bounds.getWidth() * playhead;
        g.setColour (accent.withAlpha (0.055f));
        g.fillRect (bounds.withRight (playheadX));
        juce::ColourGradient glow (accent.withAlpha (0.44f), playheadX,
                                   bounds.getCentreY(), accent.withAlpha (0.0f),
                                   playheadX + 15.0f, bounds.getCentreY(), true);
        g.setGradientFill (glow);
        g.fillRect (juce::Rectangle<float> (playheadX - 15.0f, bounds.getY(),
                                            30.0f, bounds.getHeight()));
        g.setColour (accent.withAlpha (0.34f));
        g.fillRect (juce::Rectangle<float> (playheadX - 2.5f, bounds.getY(),
                                            5.0f, bounds.getHeight()));
        g.setColour (juce::Colour (text).withAlpha (0.94f));
        g.fillRect (juce::Rectangle<float> (playheadX - 0.65f, bounds.getY(),
                                            1.3f, bounds.getHeight()));
        juce::Path marker;
        marker.addTriangle (playheadX - 4.5f, bounds.getY(),
                            playheadX + 4.5f, bounds.getY(),
                            playheadX, bounds.getY() + 7.0f);
        g.setColour (accent);
        g.fillPath (marker);
        g.setColour (juce::Colour (ice));
        g.fillEllipse (playheadX - 2.0f, bounds.getCentreY() - 2.0f, 4.0f, 4.0f);
    }

    g.setColour (accent.withAlpha (0.52f));
    g.setFont (uiFont (7.5f, juce::Font::bold));
    g.drawText (layer == SiedLayer::texture ? "LOOP" : "SOURCE",
                static_cast<int> (bounds.getX() + 5.0f),
                static_cast<int> (bounds.getBottom() - 12.0f), 46, 10,
                juce::Justification::centredLeft);
}

void SampleWaveformComponent::timerCallback()
{
    const auto current = processor.getSampleData (layer);
    if (current != displayedSample)
        rebuildWaveform();

    const auto rawPlayhead = processor.getPlaybackPosition (layer);
    if (rawPlayhead >= 0.0f)
    {
        inactivePlayheadFrames = 0;
        if (animatedPlayhead < 0.0f || std::abs (rawPlayhead - animatedPlayhead) > 0.42f)
            animatedPlayhead = rawPlayhead;
        else
            animatedPlayhead += (rawPlayhead - animatedPlayhead) * 0.62f;
    }
    else if (++inactivePlayheadFrames > 2)
    {
        animatedPlayhead = -1.0f;
    }
    repaint();
}

void SampleWaveformComponent::rebuildWaveform()
{
    displayedSample = processor.getSampleData (layer);
    waveformPeaks.clear();
    if (displayedSample == nullptr || displayedSample->audio.getNumSamples() < 2)
    {
        repaint();
        return;
    }

    constexpr int pointCount = 300;
    waveformPeaks.resize (pointCount, 0.0f);
    const auto length = displayedSample->audio.getNumSamples();
    float overallPeak = 0.000001f;
    for (int point = 0; point < pointCount; ++point)
    {
        const auto start = point * length / pointCount;
        const auto end = (point + 1) * length / pointCount;
        float peak = 0.0f;
        for (int channel = 0; channel < displayedSample->audio.getNumChannels(); ++channel)
            peak = juce::jmax (peak, displayedSample->audio.getMagnitude (channel, start,
                                                                          juce::jmax (1, end - start)));
        waveformPeaks[static_cast<size_t> (point)] = peak;
        overallPeak = juce::jmax (overallPeak, peak);
    }
    for (auto& peak : waveformPeaks)
        peak = juce::jlimit (0.0f, 1.0f, peak / overallPeak);
    repaint();
}

void SampleWaveformComponent::mouseDown (const juce::MouseEvent& event)
{
    const auto bounds = getWaveformBounds();
    const auto startX = bounds.getX() + bounds.getWidth() * getStart();
    const auto endX = bounds.getX() + bounds.getWidth() * getEnd();
    draggedHandle = endParameter != nullptr && std::abs (event.position.x - endX)
                                                    < std::abs (event.position.x - startX)
                        ? Handle::end : Handle::start;
    if (draggedHandle == Handle::start && startParameter != nullptr)
        startParameter->beginChangeGesture();
    else if (draggedHandle == Handle::end && endParameter != nullptr)
        endParameter->beginChangeGesture();
    updateHandleFromX (event.position.x);
}

void SampleWaveformComponent::mouseDrag (const juce::MouseEvent& event)
{
    updateHandleFromX (event.position.x);
}

void SampleWaveformComponent::mouseUp (const juce::MouseEvent&)
{
    if (draggedHandle == Handle::start && startParameter != nullptr)
        startParameter->endChangeGesture();
    else if (draggedHandle == Handle::end && endParameter != nullptr)
        endParameter->endChangeGesture();
    draggedHandle = Handle::none;
}

void SampleWaveformComponent::mouseDoubleClick (const juce::MouseEvent&)
{
    if (startParameter != nullptr)
        startParameter->setValueNotifyingHost (0.0f);
    if (endParameter != nullptr)
        endParameter->setValueNotifyingHost (1.0f);
}

void SampleWaveformComponent::updateHandleFromX (float x)
{
    const auto bounds = getWaveformBounds();
    auto value = juce::jlimit (0.0f, 1.0f, (x - bounds.getX()) / bounds.getWidth());
    if (draggedHandle == Handle::start && startParameter != nullptr)
    {
        value = juce::jmin (value, getEnd() - 0.005f);
        startParameter->setValueNotifyingHost (value);
    }
    else if (draggedHandle == Handle::end && endParameter != nullptr)
    {
        value = juce::jmax (value, getStart() + 0.005f);
        endParameter->setValueNotifyingHost (value);
    }
    repaint();
}

LibraryBrowserComponent::LibraryBrowserComponent (SiedAudioProcessor& owner)
    : processor (owner)
{
    searchBox.setTextToShowWhenEmpty ("Search by name or category", juce::Colour (muted));
    searchBox.setColour (juce::TextEditor::backgroundColourId, juce::Colour (panel));
    searchBox.setColour (juce::TextEditor::textColourId, juce::Colour (text));
    searchBox.setColour (juce::TextEditor::outlineColourId, juce::Colour (line));
    searchBox.setColour (juce::TextEditor::focusedOutlineColourId,
                         juce::Colour (cyan).withAlpha (0.70f));
    searchBox.setFont (uiFont (13.0f));
    searchBox.onTextChange = [this] { rebuildResults(); };

    sourceBox.addItemList ({ "ONE-SHOTS", "TEXTURES" }, 1);
    sourceBox.setSelectedId (1, juce::dontSendNotification);
    sourceBox.onChange = [this]
    {
        targetBox.setEnabled (sourceBox.getSelectedId() == 1);
        rebuildCategories();
        rebuildResults();
    };
    targetBox.addItemList ({ "LOAD TO A", "LOAD TO B" }, 1);
    targetBox.setSelectedId (1, juce::dontSendNotification);
    categoryBox.onChange = [this] { rebuildResults(); };
    favouritesButton.onClick = [this] { rebuildResults(); };
    loadButton.onClick = [this] { loadSelected(); };

    for (auto* box : { &sourceBox, &targetBox, &categoryBox })
    {
        box->setColour (juce::ComboBox::backgroundColourId, juce::Colour (panelRaised));
        box->setColour (juce::ComboBox::textColourId, juce::Colour (text));
        box->setColour (juce::ComboBox::outlineColourId, juce::Colour (line));
        box->setColour (juce::ComboBox::arrowColourId, juce::Colour (muted));
        addAndMakeVisible (*box);
    }
    favouritesButton.getProperties().set ("favouriteFilter", true);
    addAndMakeVisible (searchBox);
    addAndMakeVisible (favouritesButton);
    addAndMakeVisible (loadButton);

    resultList.setRowHeight (44);
    resultList.setColour (juce::ListBox::backgroundColourId, juce::Colours::transparentBlack);
    resultList.setColour (juce::ListBox::outlineColourId, juce::Colour (line));
    resultList.setOutlineThickness (1);
    addAndMakeVisible (resultList);

    resultCount.setColour (juce::Label::textColourId, juce::Colour (muted));
    resultCount.setFont (uiFont (10.5f));
    resultCount.setJustificationType (juce::Justification::centredLeft);
    addAndMakeVisible (resultCount);
    rebuildCategories();
    rebuildResults();
}

void LibraryBrowserComponent::paint (juce::Graphics& g)
{
    const auto bounds = getLocalBounds().toFloat();
    g.setColour (juce::Colour (panel));
    g.fillRoundedRectangle (bounds, 11.0f);
    g.setColour (juce::Colour (line));
    g.drawRoundedRectangle (bounds.reduced (0.5f), 11.0f, 0.8f);
    g.setColour (juce::Colour (text));
    g.setFont (uiFont (16.0f, juce::Font::bold));
    g.drawText ("LIBRARY", 22, 14, 180, 24, juce::Justification::centredLeft);
}

void LibraryBrowserComponent::resized()
{
    const auto width = getWidth();
    searchBox.setBounds (22, 48, width - 44, 38);
    sourceBox.setBounds (22, 98, 150, 32);
    targetBox.setBounds (182, 98, 150, 32);
    categoryBox.setBounds (342, 98, 190, 32);
    favouritesButton.setBounds (542, 98, 126, 32);
    loadButton.setBounds (width - 118, 98, 96, 32);
    resultList.setBounds (22, 144, width - 44, getHeight() - 190);
    resultCount.setBounds (22, getHeight() - 40, width - 44, 24);
}

int LibraryBrowserComponent::getNumRows()
{
    return static_cast<int> (filteredIndices.size());
}

void LibraryBrowserComponent::paintListBoxItem (int row, juce::Graphics& g,
                                                 int width, int height, bool selected)
{
    if (! juce::isPositiveAndBelow (row, static_cast<int> (filteredIndices.size())))
        return;
    const auto texture = sourceBox.getSelectedId() == 2;
    const auto index = filteredIndices[static_cast<size_t> (row)];
    const auto name = texture ? processor.getTextureName (index)
                              : processor.getOneShotName (index);
    const auto category = texture ? processor.getTextureCategory (index)
                                  : processor.getOneShotCategory (index);
    const auto layer = texture ? SiedLayer::texture : SiedLayer::oneShotA;
    const auto favourite = processor.isFavourite (layer, index);
    const auto rowBounds = juce::Rectangle<float> (0.0f, 0.0f,
                                                    static_cast<float> (width),
                                                    static_cast<float> (height));
    if (selected)
    {
        g.setColour (juce::Colour (cyan).withAlpha (0.11f));
        g.fillRect (rowBounds);
    }
    else if ((row & 1) != 0)
    {
        g.setColour (juce::Colours::white.withAlpha (0.018f));
        g.fillRect (rowBounds);
    }
    g.setColour (juce::Colour (line).withAlpha (0.52f));
    g.drawHorizontalLine (height - 1, 12.0f, static_cast<float> (width - 12));

    juce::Path star;
    star.addStar ({ 23.0f, height * 0.5f }, 5, 4.0f, 8.0f,
                  -juce::MathConstants<float>::halfPi);
    g.setColour (favourite ? juce::Colour (coral) : juce::Colour (muted).withAlpha (0.50f));
    if (favourite) g.fillPath (star); else g.strokePath (star, juce::PathStrokeType (1.0f));
    g.setColour (selected ? juce::Colour (text) : juce::Colour (text).withAlpha (0.82f));
    g.setFont (uiFont (12.5f, selected ? juce::Font::bold : juce::Font::plain));
    g.drawText (name, 48, 3, width - 230, height - 6, juce::Justification::centredLeft);
    g.setColour (juce::Colour (muted));
    g.setFont (uiFont (9.5f, juce::Font::bold));
    g.drawText (category.toUpperCase(), width - 180, 3, 158, height - 6,
                juce::Justification::centredRight);
}

void LibraryBrowserComponent::listBoxItemClicked (int row, const juce::MouseEvent& event)
{
    if (! juce::isPositiveAndBelow (row, static_cast<int> (filteredIndices.size())))
        return;
    if (event.x < 44)
    {
        const auto layer = sourceBox.getSelectedId() == 2 ? SiedLayer::texture
                                                          : SiedLayer::oneShotA;
        processor.toggleFavourite (layer, filteredIndices[static_cast<size_t> (row)]);
        if (favouritesButton.getToggleState())
            rebuildResults();
        else
            resultList.repaintRow (row);
    }
}

void LibraryBrowserComponent::listBoxItemDoubleClicked (int row, const juce::MouseEvent&)
{
    resultList.selectRow (row);
    loadSelected();
}

void LibraryBrowserComponent::selectedRowsChanged (int)
{
    loadButton.setEnabled (resultList.getSelectedRow() >= 0);
}

void LibraryBrowserComponent::rebuildCategories()
{
    const auto texture = sourceBox.getSelectedId() == 2;
    juce::StringArray categories;
    const auto count = texture ? processor.getTextureCount() : processor.getOneShotCount();
    for (int index = 0; index < count; ++index)
        categories.addIfNotAlreadyThere (texture ? processor.getTextureCategory (index)
                                                 : processor.getOneShotCategory (index), true);
    categories.sort (true);
    categoryBox.clear (juce::dontSendNotification);
    categoryBox.addItem ("ALL CATEGORIES", 1);
    for (int index = 0; index < categories.size(); ++index)
        categoryBox.addItem (categories[index].toUpperCase(), index + 2);
    categoryBox.setSelectedId (1, juce::dontSendNotification);
}

void LibraryBrowserComponent::rebuildResults()
{
    filteredIndices.clear();
    const auto texture = sourceBox.getSelectedId() == 2;
    const auto count = texture ? processor.getTextureCount() : processor.getOneShotCount();
    const auto query = searchBox.getText().trim();
    const auto category = categoryBox.getSelectedId() <= 1 ? juce::String()
                                                            : categoryBox.getText();
    const auto layer = texture ? SiedLayer::texture : SiedLayer::oneShotA;
    for (int index = 0; index < count; ++index)
    {
        const auto name = texture ? processor.getTextureName (index)
                                  : processor.getOneShotName (index);
        const auto itemCategory = texture ? processor.getTextureCategory (index)
                                          : processor.getOneShotCategory (index);
        if (query.isNotEmpty() && ! name.containsIgnoreCase (query)
            && ! itemCategory.containsIgnoreCase (query))
            continue;
        if (category.isNotEmpty() && itemCategory.compareIgnoreCase (category) != 0)
            continue;
        if (favouritesButton.getToggleState() && ! processor.isFavourite (layer, index))
            continue;
        filteredIndices.push_back (index);
    }
    resultList.updateContent();
    resultList.deselectAllRows();
    resultCount.setText (juce::String (filteredIndices.size())
                         + (texture ? " textures" : " one-shots"),
                         juce::dontSendNotification);
}

void LibraryBrowserComponent::loadSelected()
{
    const auto row = resultList.getSelectedRow();
    if (! juce::isPositiveAndBelow (row, static_cast<int> (filteredIndices.size())))
        return;
    const auto index = filteredIndices[static_cast<size_t> (row)];
    const auto loaded = sourceBox.getSelectedId() == 2
        ? processor.loadTexture (index)
        : processor.loadOneShot (targetBox.getSelectedId() == 2 ? SiedLayer::oneShotB
                                                                 : SiedLayer::oneShotA,
                                 index);
    if (loaded && onSoundLoaded)
        onSoundLoaded();
}

void LibraryBrowserComponent::refresh()
{
    rebuildCategories();
    rebuildResults();
}

SiedAudioProcessorEditor::SiedAudioProcessorEditor (SiedAudioProcessor& owner)
    : AudioProcessorEditor (&owner), processor (owner),
      waveformA (owner, SiedLayer::oneShotA, "sampleStart", "sampleEnd"),
      waveformB (owner, SiedLayer::oneShotB, "sample2Start", "sample2End"),
      waveformTexture (owner, SiedLayer::texture, "textureStart"),
      libraryBrowser (owner)
{
    setLookAndFeel (&lookAndFeel);
    setOpaque (true);
    setSize (1120, 760);
    setResizable (true, true);
    setResizeLimits (1120, 680, 1400, 940);

    configureKnob (shimmer, "SHIMMER", juce::Colour (cyan), {}, true);
    configureKnob (shimmerMix, "MIX", juce::Colour (cyan), {}, true);
    configureKnob (oneShotALevel, "LEVEL", juce::Colour (cyan), " dB");
    configureKnob (oneShotBLevel, "LEVEL", juce::Colour (violet), " dB");
    configureKnob (textureLevel, "LEVEL", juce::Colour (coral), " dB");
    configureKnob (oneShotATranspose, "TRANSPOSE", juce::Colour (cyan), " st");
    configureKnob (oneShotAFine, "FINE", juce::Colour (cyan), " ct");
    configureKnob (oneShotBTranspose, "TRANSPOSE", juce::Colour (violet), " st");
    configureKnob (oneShotBFine, "FINE", juce::Colour (violet), " ct");
    configureKnob (textureTranspose, "TRANSPOSE", juce::Colour (coral), " st");
    configureKnob (textureFine, "FINE", juce::Colour (coral), " ct");
    for (auto* tuning : { &oneShotATranspose, &oneShotAFine, &oneShotBTranspose,
                          &oneShotBFine, &textureTranspose, &textureFine })
        tuning->setNumDecimalPlacesToDisplay (0);
    configureKnob (textureStart, "START", juce::Colour (coral), {}, true);
    configureKnob (textureRandom, "RANDOM START", juce::Colour (coral), {}, true);
    configureKnob (voices, "VOICES", juce::Colour (cyan));
    voices.setNumDecimalPlacesToDisplay (0);
    configureKnob (glide, "GLIDE", juce::Colour (violet), " s");
    configureKnob (attack, "ATTACK", juce::Colour (muted), " s");
    configureKnob (decay, "DECAY", juce::Colour (muted), " s");
    configureKnob (sustain, "SUSTAIN", juce::Colour (muted), {}, true);
    configureKnob (release, "RELEASE", juce::Colour (muted), " s");
    configureKnob (tone, "TONE", juce::Colour (cyan), {}, true);
    configureKnob (transpose, "TRANSPOSE", juce::Colour (violet), " st");
    transpose.setNumDecimalPlacesToDisplay (0);
    configureKnob (velocity, "VELOCITY", juce::Colour (muted), {}, true);
    configureKnob (output, "OUTPUT", juce::Colour (cyan), " dB");
    configureKnob (chorus, "CHORUS", juce::Colour (cyan), {}, true);
    configureKnob (delay, "DELAY", juce::Colour (violet), {}, true);
    configureKnob (delayTimeMs, "DELAY TIME", juce::Colour (violet), " ms");
    delayTimeMs.setNumDecimalPlacesToDisplay (0);
    configureKnob (crush, "CRUSH", juce::Colour (coral), {}, true);
    configureKnob (reverb, "REVERB", juce::Colour (cyan), {}, true);
    configureKnob (drive, "DRIVE", juce::Colour (coral), {}, true);
    configureKnob (phaser, "PHASER", juce::Colour (violet), {}, true);
    configureKnob (flanger, "FLANGER", juce::Colour (cyan), {}, true);
    configureKnob (tremolo, "TREMOLO", juce::Colour (coral), {}, true);
    configureKnob (width, "WIDTH", juce::Colour (cyan));
    configureKnob (lowpass, "LOW-PASS", juce::Colour (violet), {}, true);
    configureKnob (highpass, "HIGH-PASS", juce::Colour (violet), {}, true);
    configureKnob (compressor, "COMPRESSOR", juce::Colour (cyan), {}, true);
    configureKnob (pan, "PAN", juce::Colour (muted));
    configureKnob (chorusMix, "MIX", juce::Colour (cyan), {}, true);
    configureKnob (delayMix, "MIX", juce::Colour (violet), {}, true);
    configureKnob (crushMix, "MIX", juce::Colour (coral), {}, true);
    configureKnob (reverbMix, "MIX", juce::Colour (cyan), {}, true);
    configureKnob (driveMix, "MIX", juce::Colour (coral), {}, true);
    configureKnob (phaserMix, "MIX", juce::Colour (violet), {}, true);
    configureKnob (flangerMix, "MIX", juce::Colour (cyan), {}, true);
    configureKnob (tremoloMix, "MIX", juce::Colour (coral), {}, true);
    configureKnob (widthMix, "MIX", juce::Colour (cyan), {}, true);
    configureKnob (lowpassMix, "MIX", juce::Colour (violet), {}, true);
    configureKnob (highpassMix, "MIX", juce::Colour (violet), {}, true);
    configureKnob (compressorMix, "MIX", juce::Colour (cyan), {}, true);

    for (auto* effect : { &shimmer, &chorus, &delay, &crush, &reverb, &drive,
                          &phaser, &flanger, &tremolo, &width, &lowpass,
                          &highpass, &compressor })
        effect->getProperties().set ("reactiveGlow", true);
    width.getProperties().set ("bipolarCentre", true);

    const std::array<std::pair<const char*, juce::Slider*>, 48> sliderMap
    {{
        { "halo", &shimmer }, { "shimmerMix", &shimmerMix },
        { "oneShotALevel", &oneShotALevel },
        { "oneShotBLevel", &oneShotBLevel }, { "textureLevel", &textureLevel },
        { "oneShotATranspose", &oneShotATranspose }, { "oneShotAFine", &oneShotAFine },
        { "oneShotBTranspose", &oneShotBTranspose }, { "oneShotBFine", &oneShotBFine },
        { "textureTranspose", &textureTranspose }, { "textureFine", &textureFine },
        { "textureStart", &textureStart }, { "textureRandom", &textureRandom },
        { "voices", &voices }, { "glide", &glide }, { "attack", &attack },
        { "decay", &decay }, { "sustain", &sustain }, { "release", &release },
        { "tone", &tone }, { "transpose", &transpose }, { "velocity", &velocity },
        { "output", &output }, { "drift", &chorus }, { "ghost", &delay },
        { "delayTimeMs", &delayTimeMs },
        { "crush", &crush }, { "reverb", &reverb }, { "drive", &drive },
        { "phaser", &phaser }, { "flanger", &flanger }, { "tremolo", &tremolo },
        { "width", &width }, { "lowpass", &lowpass }, { "highpass", &highpass },
        { "compressor", &compressor },
        { "chorusMix", &chorusMix }, { "delayMix", &delayMix },
        { "crushMix", &crushMix }, { "reverbMix", &reverbMix },
        { "driveMix", &driveMix }, { "phaserMix", &phaserMix },
        { "flangerMix", &flangerMix }, { "tremoloMix", &tremoloMix },
        { "widthMix", &widthMix }, { "lowpassMix", &lowpassMix },
        { "highpassMix", &highpassMix }, { "compressorMix", &compressorMix }
    }};
    for (const auto& [parameterID, slider] : sliderMap)
    {
        attachSlider (parameterID, *slider);
        addAndMakeVisible (*slider);
    }
    attachSlider ("pan", pan);
    addAndMakeVisible (pan);

    addAndMakeVisible (waveformA);
    addAndMakeVisible (waveformB);
    addAndMakeVisible (waveformTexture);
    addAndMakeVisible (libraryBrowser);
    libraryBrowser.onSoundLoaded = [this] { refreshSelections(); };

    for (auto* button : { &sampleTab, &effectsTab, &libraryTab, &randomOneShotsButton,
                          &randomTextureButton, &randomAllButton, &randomFxButton,
                          &resetFxButton, &loadAButton, &loadBButton, &loadTextureButton,
                          &undoButton, &initButton, &savePresetButton,
                          &oneShotAPreviousButton, &oneShotANextButton,
                          &oneShotBPreviousButton, &oneShotBNextButton,
                          &texturePreviousButton, &textureNextButton,
                          &sceneAButton, &sceneBButton })
    {
        configureButton (*button, button == &randomAllButton);
        addAndMakeVisible (*button);
    }
    randomAllButton.getProperties().set ("dice", true);
    undoButton.getProperties().set ("undo", true);
    undoButton.setEnabled (false);
    sampleTab.getProperties().set ("tab", true);
    effectsTab.getProperties().set ("tab", true);
    libraryTab.getProperties().set ("tab", true);
    oneShotAPreviousButton.getProperties().set ("arrowDirection", -1);
    oneShotANextButton.getProperties().set ("arrowDirection", 1);
    oneShotBPreviousButton.getProperties().set ("arrowDirection", -1);
    oneShotBNextButton.getProperties().set ("arrowDirection", 1);
    texturePreviousButton.getProperties().set ("arrowDirection", -1);
    textureNextButton.getProperties().set ("arrowDirection", 1);
    sampleTab.setRadioGroupId (1001);
    effectsTab.setRadioGroupId (1001);
    libraryTab.setRadioGroupId (1001);
    sampleTab.onClick = [this] { showPage (0); };
    effectsTab.onClick = [this] { showPage (1); };
    libraryTab.onClick = [this] { showPage (2); };
    randomOneShotsButton.onClick = [this]
    {
        processor.randomizeOneShots();
        refreshSelections();
    };
    randomTextureButton.onClick = [this]
    {
        processor.randomizeTexture();
        refreshSelections();
    };
    randomAllButton.onClick = [this]
    {
        processor.randomizePreset();
        refreshSelections();
    };
    undoButton.onClick = [this]
    {
        if (processor.undoLastRandomization())
        {
            refreshSelections();
            repaint();
        }
    };
    sceneAButton.onClick = [this]
    {
        processor.captureScene (0);
        sceneMorph.setValue (0.0, juce::dontSendNotification);
    };
    sceneBButton.onClick = [this]
    {
        processor.captureScene (1);
        sceneMorph.setValue (1.0, juce::dontSendNotification);
    };
    initButton.onClick = [this]
    {
        processor.resetToInitPatch();
        refreshSelections();
    };
    savePresetButton.onClick = [this]
    {
        const auto folder = juce::File (processor.getUserPresetFolderPath());
        if (folder.createDirectory().failed())
            return;
        auto chooser = std::make_shared<juce::FileChooser> (
            "Save SIED user preset", folder.getChildFile ("My Preset.siedpreset"),
            "*.siedpreset");
        chooser->launchAsync (juce::FileBrowserComponent::saveMode
                                  | juce::FileBrowserComponent::canSelectFiles
                                  | juce::FileBrowserComponent::warnAboutOverwriting,
                              [this, chooser] (const juce::FileChooser& result)
                              {
                                  const auto file = result.getResult();
                                  if (file != juce::File {}
                                      && processor.saveUserPreset (
                                          file.getFileNameWithoutExtension()))
                                  {
                                      rebuildPresetMenu();
                                      userPresetBox.setText (
                                          file.getFileNameWithoutExtension(),
                                          juce::dontSendNotification);
                                  }
                              });
    };
    randomFxButton.onClick = [this] { randomizeEffects(); };
    resetFxButton.onClick = [this] { resetEffects(); };
    loadAButton.onClick = [this] { chooseSample (SiedLayer::oneShotA); };
    loadBButton.onClick = [this] { chooseSample (SiedLayer::oneShotB); };
    loadTextureButton.onClick = [this] { chooseSample (SiedLayer::texture); };
    oneShotAPreviousButton.onClick = [this]
    {
        processor.stepOneShot (SiedLayer::oneShotA, -1); refreshSelections();
    };
    oneShotANextButton.onClick = [this]
    {
        processor.stepOneShot (SiedLayer::oneShotA, 1); refreshSelections();
    };
    oneShotBPreviousButton.onClick = [this]
    {
        processor.stepOneShot (SiedLayer::oneShotB, -1); refreshSelections();
    };
    oneShotBNextButton.onClick = [this]
    {
        processor.stepOneShot (SiedLayer::oneShotB, 1); refreshSelections();
    };
    texturePreviousButton.onClick = [this]
    {
        processor.stepTexture (-1); refreshSelections();
    };
    textureNextButton.onClick = [this]
    {
        processor.stepTexture (1); refreshSelections();
    };
    randomAllButton.setTooltip ("Randomize the full patch with restrained Shimmer, Drive, and Crush.");
    undoButton.setTooltip ("Return to the patch before the last randomization");
    sceneAButton.setTooltip ("Capture the current patch as Scene A");
    sceneBButton.setTooltip ("Capture the current patch as Scene B");
    randomFxButton.setTooltip ("Randomize every effect, mode, delay time, and dry/wet mix.");
    initButton.setTooltip ("Restore key - topograph in slot A, turn slot B off, and load Forest.");
    savePresetButton.setTooltip ("Save the complete current patch to Documents/SIED/Presets.");
    oneShotAPreviousButton.setTooltip ("Previous one-shot for slot A");
    oneShotANextButton.setTooltip ("Next one-shot for slot A");
    oneShotBPreviousButton.setTooltip ("Previous one-shot for slot B");
    oneShotBNextButton.setTooltip ("Next one-shot for slot B");
    texturePreviousButton.setTooltip ("Previous texture");
    textureNextButton.setTooltip ("Next texture");
    for (auto* mix : { &shimmerMix, &chorusMix, &delayMix, &crushMix, &reverbMix,
                       &driveMix, &phaserMix, &flangerMix, &tremoloMix, &widthMix,
                       &lowpassMix, &highpassMix, &compressorMix })
        mix->setTooltip ("Dry/wet balance for this effect");

    for (auto* combo : { &userPresetBox, &categoryBox, &oneShotABox, &oneShotBBox,
                          &textureBox, &loopBox,
                          &delayTypeBox, &delayDivisionBox, &reverbTypeBox, &lowpassTypeBox,
                          &highpassTypeBox, &chorusTypeBox, &crushTypeBox, &driveTypeBox,
                          &phaserTypeBox, &flangerTypeBox, &tremoloTypeBox,
                          &compressorTypeBox })
    {
        configureCombo (*combo);
        addAndMakeVisible (*combo);
    }

    userPresetBox.onChange = [this]
    {
        if (userPresetBox.getSelectedId() > 1
            && processor.loadUserPreset (userPresetBox.getText()))
        {
            refreshSelections();
            repaint();
        }
    };

    juce::StringArray categories;
    for (int i = 0; i < processor.getOneShotCount(); ++i)
        categories.addIfNotAlreadyThere (processor.getOneShotCategory (i), true);
    categories.sort (true);
    categoryBox.addItem ("ALL CATEGORIES", 1);
    for (int i = 0; i < categories.size(); ++i)
        categoryBox.addItem (categories[i].toUpperCase(), i + 2);
    categoryBox.setSelectedId (1, juce::dontSendNotification);
    categoryBox.onChange = [this] { rebuildLibraryMenus(); };

    oneShotABox.onChange = [this]
    {
        if (oneShotABox.getSelectedId() > 0)
            processor.loadOneShot (SiedLayer::oneShotA, oneShotABox.getSelectedId() - 1);
    };
    oneShotBBox.onChange = [this]
    {
        if (oneShotBBox.getSelectedId() > 0)
            processor.loadOneShot (SiedLayer::oneShotB, oneShotBBox.getSelectedId() - 1);
    };
    textureBox.onChange = [this]
    {
        if (textureBox.getSelectedId() > 0)
            processor.loadTexture (textureBox.getSelectedId() - 1);
    };

    loopBox.addItemList ({ "AUTO LOOP", "ONE-SHOT", "LOOP REGION" }, 1);
    delayTypeBox.addItemList ({ "STEREO", "PING-PONG", "TAPE", "DIFFUSE" }, 1);
    delayDivisionBox.addItemList ({ "FREE (MS)", "1/1", "1/2", "1/4", "1/8", "1/16",
                                     "1/4 DOTTED", "1/8 DOTTED", "1/8 TRIPLET" }, 1);
    reverbTypeBox.addItemList ({ "ROOM", "HALL", "PLATE", "CLOUD" }, 1);
    lowpassTypeBox.addItemList ({ "CLEAN LP", "WARM LP", "RESONANT LP" }, 1);
    highpassTypeBox.addItemList ({ "CLEAN HP", "WARM HP", "RESONANT HP" }, 1);
    chorusTypeBox.addItemList ({ "GENTLE", "WIDE", "ENSEMBLE" }, 1);
    crushTypeBox.addItemList ({ "DIGITAL", "VINTAGE", "GLITCH" }, 1);
    driveTypeBox.addItemList ({ "SOFT", "TUBE", "FOLD", "CLIP" }, 1);
    phaserTypeBox.addItemList ({ "SMOOTH", "DEEP", "FAST" }, 1);
    flangerTypeBox.addItemList ({ "JET", "TAPE", "METAL" }, 1);
    tremoloTypeBox.addItemList ({ "SINE", "SQUARE", "CHOP" }, 1);
    compressorTypeBox.addItemList ({ "GLUE", "PUNCH", "PUMP" }, 1);

    auto& state = processor.getParameters();
    comboAttachments.push_back (std::make_unique<ComboBoxAttachment> (state, "loopMode", loopBox));
    comboAttachments.push_back (std::make_unique<ComboBoxAttachment> (state, "delayType", delayTypeBox));
    comboAttachments.push_back (std::make_unique<ComboBoxAttachment> (state, "delayDivision", delayDivisionBox));
    comboAttachments.push_back (std::make_unique<ComboBoxAttachment> (state, "reverbType", reverbTypeBox));
    comboAttachments.push_back (std::make_unique<ComboBoxAttachment> (state, "lowpassType", lowpassTypeBox));
    comboAttachments.push_back (std::make_unique<ComboBoxAttachment> (state, "highpassType", highpassTypeBox));
    comboAttachments.push_back (std::make_unique<ComboBoxAttachment> (state, "chorusType", chorusTypeBox));
    comboAttachments.push_back (std::make_unique<ComboBoxAttachment> (state, "crushType", crushTypeBox));
    comboAttachments.push_back (std::make_unique<ComboBoxAttachment> (state, "driveType", driveTypeBox));
    comboAttachments.push_back (std::make_unique<ComboBoxAttachment> (state, "phaserType", phaserTypeBox));
    comboAttachments.push_back (std::make_unique<ComboBoxAttachment> (state, "flangerType", flangerTypeBox));
    comboAttachments.push_back (std::make_unique<ComboBoxAttachment> (state, "tremoloType", tremoloTypeBox));
    comboAttachments.push_back (std::make_unique<ComboBoxAttachment> (state, "compressorType", compressorTypeBox));

    reverseButton.setColour (juce::ToggleButton::textColourId, juce::Colour (text));
    monoButton.setColour (juce::ToggleButton::textColourId, juce::Colour (text));
    addAndMakeVisible (reverseButton);
    addAndMakeVisible (monoButton);
    addAndMakeVisible (oneShotAEnabledButton);
    addAndMakeVisible (oneShotBEnabledButton);
    addAndMakeVisible (textureEnabledButton);
    buttonAttachments.push_back (std::make_unique<ButtonAttachment> (state, "reverse", reverseButton));
    buttonAttachments.push_back (std::make_unique<ButtonAttachment> (state, "mono", monoButton));
    buttonAttachments.push_back (std::make_unique<ButtonAttachment> (state, "oneShotAEnabled",
                                                                       oneShotAEnabledButton));
    buttonAttachments.push_back (std::make_unique<ButtonAttachment> (state, "oneShotBEnabled",
                                                                       oneShotBEnabledButton));
    buttonAttachments.push_back (std::make_unique<ButtonAttachment> (state, "textureEnabled",
                                                                       textureEnabledButton));

    const std::array<std::pair<const char*, const char*>, 18> lockMap
    {{
        { "lockOneShotA", "Lock one-shot A during randomization" },
        { "lockOneShotB", "Lock one-shot B during randomization" },
        { "lockTexture", "Lock texture during randomization" },
        { "lockEnvelope", "Lock envelope and tone during randomization" },
        { "lockShimmer", "Lock Shimmer during randomization" },
        { "lockVoice", "Lock voice and output controls during randomization" },
        { "lockChorus", "Lock Chorus during randomization" },
        { "lockDelay", "Lock Delay during randomization" },
        { "lockReverb", "Lock Reverb during randomization" },
        { "lockDrive", "Lock Drive during randomization" },
        { "lockCrush", "Lock Crush during randomization" },
        { "lockCompressor", "Lock Compressor during randomization" },
        { "lockPhaser", "Lock Phaser during randomization" },
        { "lockFlanger", "Lock Flanger during randomization" },
        { "lockTremolo", "Lock Tremolo during randomization" },
        { "lockWidth", "Lock Width during randomization" },
        { "lockLowpass", "Lock Low-pass during randomization" },
        { "lockHighpass", "Lock High-pass during randomization" }
    }};
    for (size_t index = 0; index < randomLockButtons.size(); ++index)
    {
        auto& lock = randomLockButtons[index];
        lock.getProperties().set ("lockButton", true);
        lock.setTooltip (lockMap[index].second);
        addAndMakeVisible (lock);
        buttonAttachments.push_back (std::make_unique<ButtonAttachment> (
            state, lockMap[index].first, lock));
    }

    sceneMorph.setSliderStyle (juce::Slider::LinearHorizontal);
    sceneMorph.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    sceneMorph.setRange (0.0, 1.0, 0.001);
    sceneMorph.setValue (0.0, juce::dontSendNotification);
    sceneMorph.setColour (juce::Slider::trackColourId, juce::Colour (cyan));
    sceneMorph.setColour (juce::Slider::backgroundColourId, juce::Colour (line));
    sceneMorph.setColour (juce::Slider::thumbColourId, juce::Colour (text));
    sceneMorph.onValueChange = [this]
    {
        processor.applySceneMorph (static_cast<float> (sceneMorph.getValue()));
        refreshSelections();
    };
    sceneMorph.setTooltip ("Morph between captured Scene A and Scene B");
    addAndMakeVisible (sceneMorph);

    rootDetectionLabel.setColour (juce::Label::textColourId, juce::Colour (muted));
    rootDetectionLabel.setFont (uiFont (9.5f, juce::Font::bold));
    rootDetectionLabel.setJustificationType (juce::Justification::centredRight);
    addAndMakeVisible (rootDetectionLabel);

    if (! processor.hasScene (0)) processor.captureScene (0);
    if (! processor.hasScene (1)) processor.captureScene (1);

    libraryStatus.setColour (juce::Label::textColourId, juce::Colour (muted));
    libraryStatus.setFont (uiFont (10.5f));
    libraryStatus.setJustificationType (juce::Justification::centredRight);
    addAndMakeVisible (libraryStatus);

    rebuildLibraryMenus();
    rebuildPresetMenu();
    showPage (0);
    startTimerHz (30);
}

SiedAudioProcessorEditor::~SiedAudioProcessorEditor()
{
    stopTimer();
    setLookAndFeel (nullptr);
}

void SiedAudioProcessorEditor::configureKnob (juce::Slider& slider, const juce::String& name,
                                              juce::Colour accent, const juce::String& suffix,
                                              bool percentage)
{
    slider.setName (name);
    slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setRotaryParameters (juce::MathConstants<float>::pi * 1.23f,
                                juce::MathConstants<float>::pi * 2.77f, true);
    slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 72, 18);
    slider.setTextValueSuffix (percentage ? "%" : suffix);
    slider.setNumDecimalPlacesToDisplay (percentage ? 0 : 2);
    slider.setColour (juce::Slider::thumbColourId, accent);
    slider.setColour (juce::Slider::textBoxTextColourId, juce::Colour (text));
    slider.setColour (juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
    slider.setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    slider.setScrollWheelEnabled (false);
    if (percentage)
    {
        slider.textFromValueFunction = [] (double value)
        {
            return juce::String (std::round (value * 100.0));
        };
        slider.valueFromTextFunction = [] (const juce::String& value)
        {
            return value.getDoubleValue() / 100.0;
        };
    }
}

void SiedAudioProcessorEditor::attachSlider (const juce::String& parameterID, juce::Slider& slider)
{
    if (auto* parameter = processor.getParameters().getParameter (parameterID))
        slider.setDoubleClickReturnValue (true,
                                          parameter->convertFrom0to1 (
                                              parameter->getDefaultValue()));
    sliderAttachments.push_back (std::make_unique<SliderAttachment> (processor.getParameters(),
                                                                      parameterID, slider));
}

void SiedAudioProcessorEditor::configureButton (juce::TextButton& button, bool accent)
{
    button.getProperties().set ("accent", accent);
    button.setColour (juce::TextButton::textColourOffId,
                      accent ? juce::Colour (text) : juce::Colour (text).withAlpha (0.82f));
    button.setColour (juce::TextButton::textColourOnId, juce::Colour (text));
}

void SiedAudioProcessorEditor::configureCombo (juce::ComboBox& box)
{
    box.setColour (juce::ComboBox::backgroundColourId, juce::Colour (panelRaised));
    box.setColour (juce::ComboBox::textColourId, juce::Colour (text));
    box.setColour (juce::ComboBox::outlineColourId, juce::Colour (line));
    box.setColour (juce::ComboBox::arrowColourId, juce::Colour (muted));
}

void SiedAudioProcessorEditor::rebuildLibraryMenus()
{
    const auto selectedFilter = categoryBox.getSelectedId() <= 1
                                    ? juce::String() : categoryBox.getText();
    oneShotABox.clear (juce::dontSendNotification);
    oneShotBBox.clear (juce::dontSendNotification);
    juce::String lastCategory;
    for (int i = 0; i < processor.getOneShotCount(); ++i)
    {
        const auto category = processor.getOneShotCategory (i);
        if (selectedFilter.isNotEmpty() && category.compareIgnoreCase (selectedFilter) != 0)
            continue;
        if (selectedFilter.isEmpty() && category != lastCategory)
        {
            oneShotABox.addSectionHeading (category.toUpperCase());
            oneShotBBox.addSectionHeading (category.toUpperCase());
            lastCategory = category;
        }
        oneShotABox.addItem (processor.getOneShotName (i), i + 1);
        oneShotBBox.addItem (processor.getOneShotName (i), i + 1);
    }

    textureBox.clear (juce::dontSendNotification);
    lastCategory.clear();
    for (int i = 0; i < processor.getTextureCount(); ++i)
    {
        const auto category = processor.getTextureCategory (i);
        if (category != lastCategory)
        {
            textureBox.addSectionHeading (category.toUpperCase());
            lastCategory = category;
        }
        textureBox.addItem (processor.getTextureName (i), i + 1);
    }
    libraryStatus.setText (juce::String (processor.getOneShotCount()) + " one-shots  /  "
                           + juce::String (processor.getTextureCount()) + " textures",
                           juce::dontSendNotification);
    refreshSelections();
}

void SiedAudioProcessorEditor::rebuildPresetMenu()
{
    const auto previous = userPresetBox.getText();
    userPresetBox.clear (juce::dontSendNotification);
    userPresetBox.addItem ("USER PRESETS", 1);
    const auto names = processor.getUserPresetNames();
    for (int i = 0; i < names.size(); ++i)
        userPresetBox.addItem (names[i], i + 2);
    const auto previousIndex = names.indexOf (previous);
    userPresetBox.setSelectedId (previousIndex >= 0 ? previousIndex + 2 : 1,
                                 juce::dontSendNotification);
}

void SiedAudioProcessorEditor::timerCallback()
{
    undoButton.setEnabled (processor.canUndoRandomization());
    rootDetectionLabel.setText (processor.getLastDetectedRootText(),
                                juce::dontSendNotification);
    repaint();
}

void SiedAudioProcessorEditor::refreshSelections()
{
    const auto selectedA = processor.getSelectedOneShot (SiedLayer::oneShotA);
    const auto selectedB = processor.getSelectedOneShot (SiedLayer::oneShotB);
    const auto selectedT = processor.getSelectedTexture();
    oneShotABox.setSelectedId (selectedA + 1, juce::dontSendNotification);
    oneShotBBox.setSelectedId (selectedB + 1, juce::dontSendNotification);
    textureBox.setSelectedId (selectedT + 1, juce::dontSendNotification);
    if (oneShotABox.getSelectedId() == 0)
        oneShotABox.setText (processor.getCurrentSoundName (SiedLayer::oneShotA),
                             juce::dontSendNotification);
    if (oneShotBBox.getSelectedId() == 0)
        oneShotBBox.setText (processor.getCurrentSoundName (SiedLayer::oneShotB),
                             juce::dontSendNotification);
    if (textureBox.getSelectedId() == 0)
        textureBox.setText (processor.getCurrentSoundName (SiedLayer::texture),
                            juce::dontSendNotification);
}

void SiedAudioProcessorEditor::showPage (int page)
{
    currentPage = juce::jlimit (0, 2, page);
    const auto sampleVisible = currentPage == 0;
    const auto effectsVisible = currentPage == 1;
    const auto libraryVisible = currentPage == 2;

    const std::array<juce::Component*, 49> sampleComponents
    {
        &waveformA, &waveformB, &waveformTexture, &categoryBox, &oneShotABox,
        &oneShotBBox, &textureBox, &oneShotAPreviousButton, &oneShotANextButton,
        &oneShotBPreviousButton, &oneShotBNextButton,
        &texturePreviousButton, &textureNextButton,
        &randomOneShotsButton, &randomTextureButton,
        &loadAButton, &loadBButton, &loadTextureButton, &reverseButton, &monoButton,
        &loopBox, &libraryStatus, &shimmer, &shimmerMix,
        &oneShotALevel, &oneShotBLevel,
        &textureLevel, &oneShotATranspose, &oneShotAFine, &oneShotBTranspose,
        &oneShotBFine, &textureTranspose, &textureFine, &textureStart, &textureRandom,
        &oneShotAEnabledButton, &oneShotBEnabledButton, &textureEnabledButton,
        &voices, &glide, &attack, &decay, &sustain, &release, &tone, &transpose,
        &velocity, &output, &rootDetectionLabel
    };
    for (auto* component : sampleComponents)
        component->setVisible (sampleVisible);

    const std::array<juce::Component*, 40> effectComponents
    {
        &chorus, &delay, &delayTimeMs, &crush, &reverb, &drive, &phaser, &flanger, &tremolo,
        &width, &lowpass, &highpass, &compressor, &pan, &delayTypeBox,
        &delayDivisionBox, &reverbTypeBox, &lowpassTypeBox, &highpassTypeBox,
        &chorusTypeBox, &crushTypeBox, &driveTypeBox, &phaserTypeBox,
        &flangerTypeBox, &tremoloTypeBox, &compressorTypeBox,
        &randomFxButton, &resetFxButton, &chorusMix, &delayMix, &crushMix,
        &reverbMix, &driveMix, &phaserMix, &flangerMix, &tremoloMix,
        &widthMix, &lowpassMix, &highpassMix, &compressorMix
    };
    for (auto* component : effectComponents)
        component->setVisible (effectsVisible);

    for (size_t index = 0; index < randomLockButtons.size(); ++index)
        randomLockButtons[index].setVisible (sampleVisible ? index < 6
                                                           : effectsVisible && index >= 6);
    libraryBrowser.setVisible (libraryVisible);
    if (libraryVisible)
        libraryBrowser.refresh();

    sampleTab.setToggleState (sampleVisible, juce::dontSendNotification);
    effectsTab.setToggleState (effectsVisible, juce::dontSendNotification);
    libraryTab.setToggleState (libraryVisible, juce::dontSendNotification);
    resized();
    repaint();
}

void SiedAudioProcessorEditor::paint (juce::Graphics& g)
{
    const auto fullBounds = getLocalBounds().toFloat();
    juce::ColourGradient base (juce::Colour (background).brighter (0.035f), 0.0f, 0.0f,
                               juce::Colour (background).darker (0.10f), 0.0f,
                               static_cast<float> (getHeight()), false);
    base.addColour (0.64, juce::Colour (background));
    g.setGradientFill (base);
    g.fillRect (fullBounds);

    g.setColour (juce::Colour (panel));
    g.fillRect (0, 0, getWidth(), 76);
    g.setColour (juce::Colour (line).withAlpha (0.72f));
    g.drawHorizontalLine (75, 0.0f, static_cast<float> (getWidth()));

    g.setFont (uiFont (24.0f, juce::Font::bold));
    g.setColour (juce::Colour (text));
    g.drawText ("SIED", 24, 16, 78, 28, juce::Justification::centredLeft);
    g.setFont (uiFont (7.8f, juce::Font::bold));
    g.setColour (juce::Colour (muted));
    g.drawText ("ADAMSIED", 25, 42, 78, 12,
                juce::Justification::centredLeft);
    g.setColour (juce::Colour (muted).withAlpha (0.66f));
    g.setFont (uiFont (7.2f, juce::Font::bold));
    g.drawText ("2.6", 76, 18, 28, 10, juce::Justification::centredRight);

    const auto activeTab = currentPage == 0 ? sampleTab.getBounds()
                         : currentPage == 1 ? effectsTab.getBounds()
                                            : libraryTab.getBounds();
    g.setColour (juce::Colour (cyan).withAlpha (0.82f));
    g.fillRoundedRectangle (static_cast<float> (activeTab.getX() + 15),
                            static_cast<float> (activeTab.getBottom() + 5),
                            static_cast<float> (activeTab.getWidth() - 30), 2.0f, 1.0f);

    const auto utilityBar = juce::Rectangle<float> (22.0f, 84.0f,
                                                     static_cast<float> (getWidth() - 44), 40.0f);
    g.setColour (juce::Colour (panel));
    g.fillRoundedRectangle (utilityBar, 8.0f);
    g.setColour (juce::Colour (line).withAlpha (0.64f));
    g.drawRoundedRectangle (utilityBar, 8.0f, 0.8f);
    if (currentPage == 1)
    {
        g.setColour (juce::Colour (muted).withAlpha (0.72f));
        g.setFont (uiFont (8.0f, juce::Font::bold));
        g.drawText ("EFFECTS",
                    getWidth() - 400, 91, 360, 18, juce::Justification::centredRight);
    }

    if (currentPage == 0)
    {
        const auto margin = 22.0f;
        const auto gap = 12.0f;
        const auto cardWidth = (getWidth() - margin * 2.0f - gap * 2.0f) / 3.0f;
        const std::array<juce::Colour, 3> accents
        {{ juce::Colour (cyan), juce::Colour (violet), juce::Colour (coral) }};
        const std::array<juce::ToggleButton*, 3> enableButtons
        {{ &oneShotAEnabledButton, &oneShotBEnabledButton, &textureEnabledButton }};
        const std::array<juce::Slider*, 3> levelSliders
        {{ &oneShotALevel, &oneShotBLevel, &textureLevel }};
        const std::array<SiedLayer, 3> cardLayers
        {{ SiedLayer::oneShotA, SiedLayer::oneShotB, SiedLayer::texture }};
        for (int card = 0; card < 3; ++card)
        {
            const auto bounds = juce::Rectangle<float> (margin + card * (cardWidth + gap),
                                                         132.0f, cardWidth, 276.0f);
            const auto playing = processor.getPlaybackPosition (
                cardLayers[static_cast<size_t> (card)]) >= 0.0f;
            const auto activity = enableButtons[static_cast<size_t> (card)]->getToggleState()
                                      ? 0.08f + normalisedValue (
                                            *levelSliders[static_cast<size_t> (card)]) * 0.10f
                                            + (playing ? 0.10f : 0.0f)
                                      : 0.0f;
            drawPanelSurface (g, bounds, accents[static_cast<size_t> (card)], activity);
            g.setFont (uiFont (9.2f, juce::Font::bold));
            g.setColour (accents[static_cast<size_t> (card)].withAlpha (0.86f));
            const std::array<const char*, 3> titles { "ONE-SHOT A", "ONE-SHOT B", "TEXTURE / LOOP" };
            g.drawText (titles[static_cast<size_t> (card)], bounds.getX() + 14.0f,
                        bounds.getY() + 10.0f, bounds.getWidth() - 130.0f, 16.0f,
                        juce::Justification::centredLeft);
        }

        const auto lowerPanel = juce::Rectangle<float> (
            22.0f, 422.0f, static_cast<float> (getWidth() - 44),
            static_cast<float> (getHeight() - 444));
        const auto shimmerActivity = normalisedValue (shimmer)
                                     * (0.20f + normalisedValue (shimmerMix) * 0.80f);
        drawPanelSurface (g, lowerPanel, juce::Colour (cyan), shimmerActivity * 0.68f);
        const auto heroLeft = static_cast<float> (getWidth() / 2 - 132);
        const auto heroRight = static_cast<float> (getWidth() / 2 + 132);
        g.setColour (juce::Colour (line).withAlpha (0.56f));
        g.drawVerticalLine (static_cast<int> (heroLeft), 446.0f, lowerPanel.getBottom() - 14.0f);
        g.drawVerticalLine (static_cast<int> (heroRight), 446.0f, lowerPanel.getBottom() - 14.0f);
        g.setColour (juce::Colour (muted));
        g.setFont (uiFont (8.5f, juce::Font::bold));
        g.drawText ("ENVELOPE + TONE", 36, 438, 200, 16, juce::Justification::centredLeft);
        g.drawText ("VOICE + OUTPUT", getWidth() - 252, 438, 215, 16,
                    juce::Justification::centredRight);
        g.setColour (juce::Colour (cyan));
        g.setFont (uiFont (10.5f, juce::Font::bold));
        g.drawText ("SHIMMER", getWidth() / 2 - 70, 442, 140, 16,
                    juce::Justification::centred);
        g.setColour (juce::Colour (cyan).withAlpha (0.32f + shimmerActivity * 0.60f));
        g.fillRoundedRectangle (static_cast<float> (getWidth() / 2 - 28), 458.0f,
                                56.0f, 2.0f, 1.0f);
    }
    else if (currentPage == 1)
    {
        constexpr int columns = 6;
        const auto margin = 22;
        const auto top = 134;
        const auto rowHeight = juce::jmax (240, (getHeight() - top - 26) / 2);
        const auto columnWidth = (getWidth() - margin * 2) / columns;
        const std::array<std::array<juce::Slider*, 6>, 2> amountRows
        {{
            {{ &chorus, &delay, &reverb, &drive, &crush, &compressor }},
            {{ &phaser, &flanger, &tremolo, &width, &lowpass, &highpass }}
        }};
        const std::array<std::array<juce::Slider*, 6>, 2> mixRows
        {{
            {{ &chorusMix, &delayMix, &reverbMix, &driveMix, &crushMix, &compressorMix }},
            {{ &phaserMix, &flangerMix, &tremoloMix, &widthMix, &lowpassMix, &highpassMix }}
        }};
        const std::array<std::array<juce::Colour, 6>, 2> accents
        {{
            {{ juce::Colour (cyan), juce::Colour (violet), juce::Colour (cyan),
               juce::Colour (coral), juce::Colour (coral), juce::Colour (cyan) }},
            {{ juce::Colour (violet), juce::Colour (cyan), juce::Colour (coral),
               juce::Colour (cyan), juce::Colour (violet), juce::Colour (violet) }}
        }};
        for (int row = 0; row < 2; ++row)
            for (int column = 0; column < columns; ++column)
            {
                const auto card = juce::Rectangle<float> (
                    static_cast<float> (margin + column * columnWidth + 3),
                    static_cast<float> (top + row * rowHeight + 3),
                    static_cast<float> (columnWidth - 6),
                    static_cast<float> (rowHeight - 6));
                const auto* amount = amountRows[static_cast<size_t> (row)]
                                                [static_cast<size_t> (column)];
                const auto* mix = mixRows[static_cast<size_t> (row)]
                                          [static_cast<size_t> (column)];
                auto amountActivity = normalisedValue (*amount);
                if (amount == &width)
                    amountActivity = static_cast<float> (std::abs (width.getValue() - 1.0));
                const auto activity = std::pow (amountActivity, 0.78f)
                                      * (0.18f + normalisedValue (*mix) * 0.82f);
                const auto accent = accents[static_cast<size_t> (row)]
                                           [static_cast<size_t> (column)];
                drawPanelSurface (g, card, accent, activity);
            }
    }

    g.setFont (uiFont (8.2f, juce::Font::bold));
    const std::array<juce::Slider*, 49> allSliders
    {
        &shimmer, &shimmerMix, &oneShotALevel, &oneShotBLevel, &textureLevel,
        &oneShotATranspose, &oneShotAFine, &oneShotBTranspose, &oneShotBFine,
        &textureTranspose, &textureFine, &textureStart, &textureRandom,
        &voices, &glide, &attack, &decay, &sustain, &release,
        &tone, &transpose, &velocity, &output, &chorus, &delay, &delayTimeMs,
        &crush, &reverb, &drive, &phaser, &flanger, &tremolo, &width,
        &lowpass, &highpass, &compressor, &pan,
        &chorusMix, &delayMix, &crushMix, &reverbMix, &driveMix, &phaserMix,
        &flangerMix, &tremoloMix, &widthMix, &lowpassMix, &highpassMix,
        &compressorMix
    };
    for (auto* slider : allSliders)
        if (slider->isVisible())
        {
            const auto accent = slider->findColour (juce::Slider::thumbColourId);
            auto activity = normalisedValue (*slider);
            if (static_cast<bool> (
                    slider->getProperties().getWithDefault ("bipolarCentre", false)))
                activity = std::abs (activity - 0.5f) * 2.0f;
            const auto reactive = static_cast<bool> (
                slider->getProperties().getWithDefault ("reactiveGlow", false));
            g.setColour (reactive
                             ? juce::Colour (muted).interpolatedWith (
                                   accent, 0.12f + activity * 0.76f)
                             : juce::Colour (muted));
            g.drawText (slider->getName(), slider->getX(), slider->getY() + 1,
                        slider->getWidth(), 15, juce::Justification::centred);
        }
}

void SiedAudioProcessorEditor::resized()
{
    sampleTab.setBounds (116, 20, 76, 38);
    effectsTab.setBounds (194, 20, 54, 38);
    libraryTab.setBounds (250, 20, 82, 38);
    userPresetBox.setBounds (350, 23, 180, 32);
    savePresetButton.setBounds (536, 23, 54, 32);
    sceneAButton.setBounds (getWidth() - 374, 23, 30, 32);
    sceneMorph.setBounds (getWidth() - 340, 24, 96, 30);
    sceneBButton.setBounds (getWidth() - 240, 23, 30, 32);
    undoButton.setBounds (getWidth() - 198, 20, 46, 38);
    initButton.setBounds (getWidth() - 140, 20, 58, 38);
    randomAllButton.setBounds (getWidth() - 70, 14, 54, 50);

    const auto margin = 22;
    if (currentPage == 0)
    {
        categoryBox.setBounds (margin, 88, 160, 32);
        randomOneShotsButton.setBounds (190, 88, 132, 32);
        randomTextureButton.setBounds (330, 88, 128, 32);
        reverseButton.setBounds (466, 88, 78, 32);
        loopBox.setBounds (552, 88, 116, 32);
        libraryStatus.setBounds (676, 88, getWidth() - 820, 32);
        rootDetectionLabel.setBounds (getWidth() - 144, 88, 122, 32);

        const auto gap = 12;
        const auto cardWidth = (getWidth() - margin * 2 - gap * 2) / 3;
        const std::array<int, 3> cardX { margin, margin + cardWidth + gap,
                                         margin + (cardWidth + gap) * 2 };
        constexpr int navigationWidth = 28;
        constexpr int navigationGap = 5;
        const auto comboXOffset = 14 + navigationWidth + navigationGap;
        const auto comboWidth = cardWidth - 14 - 52 - navigationWidth * 2
                                - navigationGap * 3 - 14;
        oneShotAPreviousButton.setBounds (cardX[0] + 14, 164, navigationWidth, 34);
        oneShotBPreviousButton.setBounds (cardX[1] + 14, 164, navigationWidth, 34);
        texturePreviousButton.setBounds (cardX[2] + 14, 164, navigationWidth, 34);
        oneShotABox.setBounds (cardX[0] + comboXOffset, 164, comboWidth, 34);
        oneShotBBox.setBounds (cardX[1] + comboXOffset, 164, comboWidth, 34);
        textureBox.setBounds (cardX[2] + comboXOffset, 164, comboWidth, 34);
        oneShotANextButton.setBounds (oneShotABox.getRight() + navigationGap, 164,
                                      navigationWidth, 34);
        oneShotBNextButton.setBounds (oneShotBBox.getRight() + navigationGap, 164,
                                      navigationWidth, 34);
        textureNextButton.setBounds (textureBox.getRight() + navigationGap, 164,
                                     navigationWidth, 34);
        loadAButton.setBounds (cardX[0] + cardWidth - 66, 164, 52, 34);
        loadBButton.setBounds (cardX[1] + cardWidth - 66, 164, 52, 34);
        loadTextureButton.setBounds (cardX[2] + cardWidth - 66, 164, 52, 34);
        oneShotAEnabledButton.setBounds (cardX[0] + cardWidth - 62, 137, 48, 22);
        oneShotBEnabledButton.setBounds (cardX[1] + cardWidth - 62, 137, 48, 22);
        textureEnabledButton.setBounds (cardX[2] + cardWidth - 62, 137, 48, 22);
        randomLockButtons[0].setBounds (cardX[0] + cardWidth - 90, 137, 22, 22);
        randomLockButtons[1].setBounds (cardX[1] + cardWidth - 90, 137, 22, 22);
        randomLockButtons[2].setBounds (cardX[2] + cardWidth - 90, 137, 22, 22);
        waveformA.setBounds (cardX[0] + 14, 211, cardWidth - 28, 104);
        waveformB.setBounds (cardX[1] + 14, 211, cardWidth - 28, 104);
        waveformTexture.setBounds (cardX[2] + 14, 211, cardWidth - 28, 104);
        const auto oneShotKnobWidth = (cardWidth - 20) / 3;
        const std::array<juce::Slider*, 3> layerAKnobs
        {
            &oneShotALevel, &oneShotATranspose, &oneShotAFine
        };
        const std::array<juce::Slider*, 3> layerBKnobs
        {
            &oneShotBLevel, &oneShotBTranspose, &oneShotBFine
        };
        for (int i = 0; i < 3; ++i)
        {
            layerAKnobs[static_cast<size_t> (i)]->setBounds (
                cardX[0] + 10 + i * oneShotKnobWidth, 318, oneShotKnobWidth, 82);
            layerBKnobs[static_cast<size_t> (i)]->setBounds (
                cardX[1] + 10 + i * oneShotKnobWidth, 318, oneShotKnobWidth, 82);
        }
        const auto textureKnobWidth = (cardWidth - 12) / 5;
        const std::array<juce::Slider*, 5> textureKnobs
        {
            &textureLevel, &textureTranspose, &textureFine, &textureStart, &textureRandom
        };
        for (int i = 0; i < 5; ++i)
            textureKnobs[static_cast<size_t> (i)]->setBounds (
                cardX[2] + 6 + i * textureKnobWidth, 318, textureKnobWidth, 82);

        const auto lowerTop = 462;
        const auto lowerHeight = juce::jmax (150, getHeight() - lowerTop - 26);
        constexpr int heroWidth = 240;
        const auto sideWidth = (getWidth() - margin * 2 - heroWidth - 24) / 2;
        const auto smallWidth = sideWidth / 5;
        const std::array<juce::Slider*, 5> leftKnobs { &attack, &decay, &sustain, &release, &tone };
        for (int i = 0; i < 5; ++i)
            leftKnobs[static_cast<size_t> (i)]->setBounds (margin + i * smallWidth,
                                                           lowerTop, smallWidth, lowerHeight);

        shimmer.setBounds (getWidth() / 2 - heroWidth / 2 - 12, lowerTop - 2,
                           heroWidth - 50, lowerHeight + 4);
        shimmerMix.setBounds (getWidth() / 2 + heroWidth / 2 - 62, lowerTop + 38,
                              62, lowerHeight - 62);

        const auto rightStart = getWidth() - margin - sideWidth;
        const std::array<juce::Slider*, 5> rightKnobs { &voices, &glide, &transpose,
                                                        &velocity, &output };
        for (int i = 0; i < 5; ++i)
            rightKnobs[static_cast<size_t> (i)]->setBounds (rightStart + i * smallWidth,
                                                            lowerTop, smallWidth, lowerHeight);
        monoButton.setBounds (rightStart + 8, getHeight() - 58, 78, 30);
        randomLockButtons[3].setBounds (margin + 174, 434, 22, 22);
        randomLockButtons[4].setBounds (getWidth() / 2 + 76, 434, 22, 22);
        randomLockButtons[5].setBounds (getWidth() - margin - 22, 434, 22, 22);
    }
    else if (currentPage == 1)
    {
        randomFxButton.setBounds (margin, 84, 116, 30);
        resetFxButton.setBounds (146, 84, 96, 30);
        const auto top = 134;
        const auto rowHeight = juce::jmax (240, (getHeight() - top - 26) / 2);
        const auto columnWidth = (getWidth() - margin * 2) / 6;
        const std::array<std::array<juce::Slider*, 6>, 2> amountRows
        {{
            {{ &chorus, &delay, &reverb, &drive, &crush, &compressor }},
            {{ &phaser, &flanger, &tremolo, &width, &lowpass, &highpass }}
        }};
        const std::array<std::array<juce::Slider*, 6>, 2> mixRows
        {{
            {{ &chorusMix, &delayMix, &reverbMix, &driveMix, &crushMix, &compressorMix }},
            {{ &phaserMix, &flangerMix, &tremoloMix, &widthMix, &lowpassMix, &highpassMix }}
        }};

        for (int row = 0; row < 2; ++row)
        {
            const auto y = top + row * rowHeight;
            for (int column = 0; column < 6; ++column)
            {
                const auto x = margin + column * columnWidth;
                amountRows[static_cast<size_t> (row)][static_cast<size_t> (column)]->setBounds (
                    x + 6, y + 4, columnWidth - 12, rowHeight - 120);

                if (row == 0 && column == 1)
                {
                    const auto utilityWidth = (columnWidth - 12) / 2;
                    mixRows[0][1]->setBounds (x + 6, y + rowHeight - 112,
                                               utilityWidth, 72);
                    delayTimeMs.setBounds (x + 6 + utilityWidth, y + rowHeight - 112,
                                           utilityWidth, 72);
                }
                else if (row == 1 && column == 3)
                {
                    const auto utilityWidth = (columnWidth - 12) / 2;
                    mixRows[1][3]->setBounds (x + 6, y + rowHeight - 104,
                                               utilityWidth, 96);
                    pan.setBounds (x + 6 + utilityWidth, y + rowHeight - 104,
                                   utilityWidth, 96);
                }
                else
                {
                    mixRows[static_cast<size_t> (row)][static_cast<size_t> (column)]->setBounds (
                        x + 8, y + rowHeight - 112, columnWidth - 16, 72);
                }
                randomLockButtons[static_cast<size_t> (6 + row * 6 + column)].setBounds (
                    x + columnWidth - 31, y + 10, 22, 22);
            }
        }

        const auto comboWidth = juce::jmin (148, columnWidth - 14);
        const auto placeCombo = [comboWidth] (juce::ComboBox& box, int column, int row,
                                              int xMargin, int topY, int colWidth,
                                              int rowSize)
        {
            const auto x = xMargin + column * colWidth;
            const auto y = topY + (row + 1) * rowSize - 36;
            box.setBounds (x + (colWidth - comboWidth) / 2, y, comboWidth, 28);
        };
        placeCombo (chorusTypeBox, 0, 0, margin, top, columnWidth, rowHeight);
        placeCombo (reverbTypeBox, 2, 0, margin, top, columnWidth, rowHeight);
        placeCombo (driveTypeBox, 3, 0, margin, top, columnWidth, rowHeight);
        placeCombo (crushTypeBox, 4, 0, margin, top, columnWidth, rowHeight);
        placeCombo (compressorTypeBox, 5, 0, margin, top, columnWidth, rowHeight);
        placeCombo (phaserTypeBox, 0, 1, margin, top, columnWidth, rowHeight);
        placeCombo (flangerTypeBox, 1, 1, margin, top, columnWidth, rowHeight);
        placeCombo (tremoloTypeBox, 2, 1, margin, top, columnWidth, rowHeight);
        placeCombo (lowpassTypeBox, 4, 1, margin, top, columnWidth, rowHeight);
        placeCombo (highpassTypeBox, 5, 1, margin, top, columnWidth, rowHeight);

        const auto delayComboWidth = (columnWidth - 16) / 2;
        const auto delayX = margin + columnWidth;
        const auto delayComboY = top + rowHeight - 36;
        delayTypeBox.setBounds (delayX + 5, delayComboY, delayComboWidth, 28);
        delayDivisionBox.setBounds (delayX + 9 + delayComboWidth, delayComboY,
                                    delayComboWidth, 28);
    }
    else
    {
        libraryBrowser.setBounds (margin, 90, getWidth() - margin * 2,
                                  getHeight() - 112);
    }
}

bool SiedAudioProcessorEditor::isInterestedInFileDrag (const juce::StringArray& files)
{
    if (files.isEmpty())
        return false;
    const auto extension = juce::File (files[0]).getFileExtension().toLowerCase();
    return extension == ".wav" || extension == ".aif" || extension == ".aiff"
           || extension == ".flac";
}

void SiedAudioProcessorEditor::filesDropped (const juce::StringArray& files, int x, int)
{
    if (! isInterestedInFileDrag (files))
        return;
    const auto layer = x < getWidth() / 3 ? SiedLayer::oneShotA
                      : x < getWidth() * 2 / 3 ? SiedLayer::oneShotB
                                               : SiedLayer::texture;
    if (processor.loadSampleFile (juce::File (files[0]), layer))
        refreshSelections();
}

void SiedAudioProcessorEditor::chooseSample (SiedLayer layer)
{
    auto chooser = std::make_shared<juce::FileChooser> ("Choose audio for this SIED layer",
                                                         juce::File {},
                                                         "*.wav;*.aif;*.aiff;*.flac");
    chooser->launchAsync (juce::FileBrowserComponent::openMode
                              | juce::FileBrowserComponent::canSelectFiles,
                          [this, chooser, layer] (const juce::FileChooser& result)
                          {
                              const auto file = result.getResult();
                              if (processor.loadSampleFile (file, layer))
                                  refreshSelections();
                          });
}

void SiedAudioProcessorEditor::randomizeEffects()
{
    processor.captureRandomizationUndo();
    auto& state = processor.getParameters();
    auto& random = juce::Random::getSystemRandom();
    const auto soft = [&random] (float minimum, float maximum)
    {
        return minimum + std::pow (random.nextFloat(), 1.55f) * (maximum - minimum);
    };
    const auto effect = [&] (const char* lockID, const char* amountID,
                             const char* mixID, float amountMin, float amountMax,
                             float mixMin, float mixMax)
    {
        if (processor.isRandomizationLocked (lockID))
            return;
        setParameterValue (state, amountID, soft (amountMin, amountMax));
        setParameterValue (state, mixID, mixMin + random.nextFloat() * (mixMax - mixMin));
    };
    effect ("lockShimmer", "halo", "shimmerMix", 0.06f, 0.42f, 0.18f, 0.50f);
    effect ("lockChorus", "drift", "chorusMix", 0.05f, 0.54f, 0.20f, 0.58f);
    effect ("lockDelay", "ghost", "delayMix", 0.04f, 0.46f, 0.16f, 0.48f);
    effect ("lockReverb", "reverb", "reverbMix", 0.04f, 0.40f, 0.15f, 0.46f);
    effect ("lockDrive", "drive", "driveMix", 0.03f, 0.30f, 0.14f, 0.42f);
    effect ("lockCrush", "crush", "crushMix", 0.02f, 0.20f, 0.12f, 0.34f);
    effect ("lockPhaser", "phaser", "phaserMix", 0.04f, 0.46f, 0.16f, 0.48f);
    effect ("lockFlanger", "flanger", "flangerMix", 0.03f, 0.36f, 0.14f, 0.42f);
    effect ("lockTremolo", "tremolo", "tremoloMix", 0.03f, 0.34f, 0.14f, 0.42f);
    effect ("lockLowpass", "lowpass", "lowpassMix", 0.03f, 0.38f, 0.18f, 0.52f);
    effect ("lockHighpass", "highpass", "highpassMix", 0.02f, 0.28f, 0.16f, 0.46f);
    effect ("lockCompressor", "compressor", "compressorMix", 0.04f, 0.46f, 0.20f, 0.58f);
    if (! processor.isRandomizationLocked ("lockWidth"))
    {
        setParameterValue (state, "width", 0.76f + random.nextFloat() * 0.52f);
        setParameterValue (state, "widthMix", 0.24f + random.nextFloat() * 0.48f);
    }
    if (! processor.isRandomizationLocked ("lockDelay"))
    {
        setParameterValue (state, "delayType", static_cast<float> (random.nextInt (4)));
        setParameterValue (state, "delayDivision", static_cast<float> (random.nextInt (9)));
        setParameterValue (state, "delayTimeMs", 60.0f + random.nextFloat() * 900.0f);
    }
    if (! processor.isRandomizationLocked ("lockReverb")) setParameterValue (state, "reverbType", static_cast<float> (random.nextInt (4)));
    if (! processor.isRandomizationLocked ("lockLowpass")) setParameterValue (state, "lowpassType", static_cast<float> (random.nextInt (3)));
    if (! processor.isRandomizationLocked ("lockHighpass")) setParameterValue (state, "highpassType", static_cast<float> (random.nextInt (3)));
    if (! processor.isRandomizationLocked ("lockChorus")) setParameterValue (state, "chorusType", static_cast<float> (random.nextInt (3)));
    if (! processor.isRandomizationLocked ("lockCrush")) setParameterValue (state, "crushType", static_cast<float> (random.nextInt (3)));
    if (! processor.isRandomizationLocked ("lockDrive")) setParameterValue (state, "driveType", static_cast<float> (random.nextInt (4)));
    if (! processor.isRandomizationLocked ("lockPhaser")) setParameterValue (state, "phaserType", static_cast<float> (random.nextInt (3)));
    if (! processor.isRandomizationLocked ("lockFlanger")) setParameterValue (state, "flangerType", static_cast<float> (random.nextInt (3)));
    if (! processor.isRandomizationLocked ("lockTremolo")) setParameterValue (state, "tremoloType", static_cast<float> (random.nextInt (3)));
    if (! processor.isRandomizationLocked ("lockCompressor")) setParameterValue (state, "compressorType", static_cast<float> (random.nextInt (3)));
}

void SiedAudioProcessorEditor::resetEffects()
{
    auto& state = processor.getParameters();
    for (const auto* id : { "halo", "drift", "ghost", "crush", "reverb", "drive", "phaser",
                            "flanger", "tremolo", "lowpass", "highpass", "compressor" })
        setParameterValue (state, id, 0.0f);
    setParameterValue (state, "shimmerMix", 0.68f);
    setParameterValue (state, "chorusMix", 0.50f);
    setParameterValue (state, "delayMix", 0.38f);
    setParameterValue (state, "crushMix", 0.50f);
    setParameterValue (state, "reverbMix", 0.42f);
    setParameterValue (state, "driveMix", 0.55f);
    setParameterValue (state, "phaserMix", 0.50f);
    setParameterValue (state, "flangerMix", 0.50f);
    setParameterValue (state, "tremoloMix", 0.75f);
    for (const auto* id : { "widthMix", "lowpassMix", "highpassMix", "compressorMix" })
        setParameterValue (state, id, 1.0f);
    setParameterValue (state, "width", 1.0f);
    setParameterValue (state, "pan", 0.0f);
    setParameterValue (state, "delayType", 1.0f);
    setParameterValue (state, "delayDivision", 3.0f);
    setParameterValue (state, "delayTimeMs", 350.0f);
    setParameterValue (state, "reverbType", 1.0f);
    setParameterValue (state, "lowpassType", 0.0f);
    setParameterValue (state, "highpassType", 0.0f);
    setParameterValue (state, "chorusType", 0.0f);
    setParameterValue (state, "crushType", 0.0f);
    setParameterValue (state, "driveType", 0.0f);
    setParameterValue (state, "phaserType", 0.0f);
    setParameterValue (state, "flangerType", 0.0f);
    setParameterValue (state, "tremoloType", 0.0f);
    setParameterValue (state, "compressorType", 0.0f);
    processor.requestEffectsReset();
}
