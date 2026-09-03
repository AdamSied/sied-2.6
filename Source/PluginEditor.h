#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class SiedLookAndFeel final : public juce::LookAndFeel_V4
{
public:
    SiedLookAndFeel();
    void drawRotarySlider (juce::Graphics&, int x, int y, int width, int height,
                           float sliderPosition, float rotaryStartAngle, float rotaryEndAngle,
                           juce::Slider&) override;
    void drawLinearSlider (juce::Graphics&, int x, int y, int width, int height,
                           float sliderPos, float minSliderPos, float maxSliderPos,
                           juce::Slider::SliderStyle, juce::Slider&) override;
    void drawButtonBackground (juce::Graphics&, juce::Button&, const juce::Colour&,
                               bool highlighted, bool down) override;
    void drawButtonText (juce::Graphics&, juce::TextButton&,
                         bool highlighted, bool down) override;
    void drawComboBox (juce::Graphics&, int width, int height, bool isButtonDown,
                       int buttonX, int buttonY, int buttonW, int buttonH,
                       juce::ComboBox&) override;
    void drawToggleButton (juce::Graphics&, juce::ToggleButton&,
                           bool highlighted, bool down) override;
    juce::Font getTextButtonFont (juce::TextButton&, int buttonHeight) override;
    juce::Font getComboBoxFont (juce::ComboBox&) override;
    juce::Font getPopupMenuFont() override;
    juce::Font getLabelFont (juce::Label&) override;
    juce::Font getSliderPopupFont (juce::Slider&) override;
    void drawPopupMenuItem (juce::Graphics&, const juce::Rectangle<int>& area,
                            bool isSeparator, bool isActive, bool isHighlighted,
                            bool isTicked, bool hasSubMenu, const juce::String& text,
                            const juce::String& shortcutKeyText, const juce::Drawable* icon,
                            const juce::Colour* textColour) override;
    void drawPopupMenuBackground (juce::Graphics&, int width, int height) override;
    void drawLabel (juce::Graphics&, juce::Label&) override;
    void drawTooltip (juce::Graphics&, const juce::String& text,
                      int width, int height) override;
    int getPopupMenuBorderSize() override { return 1; }

private:
    juce::Font valueFont (float height) const;
    juce::Typeface::Ptr valueTypeface;
};

class SampleWaveformComponent final : public juce::Component,
                                      private juce::Timer
{
public:
    SampleWaveformComponent (SiedAudioProcessor&, SiedLayer,
                             const juce::String& startParameterID,
                             const juce::String& endParameterID = {});
    ~SampleWaveformComponent() override;

    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseDrag (const juce::MouseEvent&) override;
    void mouseUp (const juce::MouseEvent&) override;
    void mouseDoubleClick (const juce::MouseEvent&) override;

private:
    enum class Handle { none, start, end };

    void timerCallback() override;
    void rebuildWaveform();
    void updateHandleFromX (float x);
    juce::Rectangle<float> getWaveformBounds() const;
    float getStart() const;
    float getEnd() const;

    SiedAudioProcessor& processor;
    SiedLayer layer;
    juce::RangedAudioParameter* startParameter = nullptr;
    juce::RangedAudioParameter* endParameter = nullptr;
    std::shared_ptr<const SiedSampleData> displayedSample;
    std::vector<float> waveformPeaks;
    Handle draggedHandle = Handle::none;
    float animatedPlayhead = -1.0f;
    int inactivePlayheadFrames = 0;
};

class LibraryBrowserComponent final : public juce::Component,
                                      private juce::ListBoxModel
{
public:
    explicit LibraryBrowserComponent (SiedAudioProcessor&);
    void paint (juce::Graphics&) override;
    void resized() override;
    void refresh();
    std::function<void()> onSoundLoaded;

private:
    int getNumRows() override;
    void paintListBoxItem (int, juce::Graphics&, int, int, bool) override;
    void listBoxItemClicked (int, const juce::MouseEvent&) override;
    void listBoxItemDoubleClicked (int, const juce::MouseEvent&) override;
    void selectedRowsChanged (int) override;
    void rebuildCategories();
    void rebuildResults();
    void loadSelected();

    SiedAudioProcessor& processor;
    juce::TextEditor searchBox;
    juce::ComboBox sourceBox, targetBox, categoryBox;
    juce::ToggleButton favouritesButton { "FAVORITES" };
    juce::TextButton loadButton { "LOAD" };
    juce::ListBox resultList { "SIED Library", this };
    juce::Label resultCount;
    std::vector<int> filteredIndices;
};

class SiedAudioProcessorEditor final : public juce::AudioProcessorEditor,
                                       public juce::FileDragAndDropTarget,
                                       private juce::Timer
{
public:
    explicit SiedAudioProcessorEditor (SiedAudioProcessor&);
    ~SiedAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    bool isInterestedInFileDrag (const juce::StringArray&) override;
    void filesDropped (const juce::StringArray&, int, int) override;

private:
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;
    using ComboBoxAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    void configureKnob (juce::Slider&, const juce::String& name, juce::Colour accent,
                        const juce::String& suffix = {}, bool percentage = false);
    void attachSlider (const juce::String& parameterID, juce::Slider&);
    void configureButton (juce::TextButton&, bool accent = false);
    void configureCombo (juce::ComboBox&);
    void showPage (int page);
    void chooseSample (SiedLayer);
    void rebuildLibraryMenus();
    void rebuildPresetMenu();
    void refreshSelections();
    void randomizeEffects();
    void resetEffects();
    void timerCallback() override;

    SiedAudioProcessor& processor;
    SiedLookAndFeel lookAndFeel;
    SampleWaveformComponent waveformA;
    SampleWaveformComponent waveformB;
    SampleWaveformComponent waveformTexture;
    LibraryBrowserComponent libraryBrowser;

    juce::Slider shimmer, shimmerMix, oneShotALevel, oneShotBLevel, textureLevel;
    juce::Slider oneShotATranspose, oneShotAFine, oneShotBTranspose, oneShotBFine;
    juce::Slider textureTranspose, textureFine;
    juce::Slider textureStart, textureRandom, voices, glide;
    juce::Slider attack, decay, sustain, release, tone, transpose, velocity, output;
    juce::Slider chorus, delay, delayTimeMs, crush, reverb, drive, phaser, flanger;
    juce::Slider tremolo, width, lowpass, highpass, compressor, pan;
    juce::Slider chorusMix, delayMix, crushMix, reverbMix, driveMix, phaserMix;
    juce::Slider flangerMix, tremoloMix, widthMix, lowpassMix, highpassMix, compressorMix;

    juce::TextButton sampleTab { "SAMPLE" };
    juce::TextButton effectsTab { "FX" };
    juce::TextButton libraryTab { "LIBRARY" };
    juce::TextButton randomOneShotsButton { "RANDOM ONE-SHOTS" };
    juce::TextButton randomTextureButton { "RANDOM TEXTURE" };
    juce::TextButton randomAllButton;
    juce::TextButton undoButton;
    juce::TextButton initButton { "INIT" };
    juce::TextButton savePresetButton { "SAVE" };
    juce::TextButton randomFxButton { "RANDOM FX" };
    juce::TextButton resetFxButton { "RESET FX" };
    juce::TextButton loadAButton { "LOAD" };
    juce::TextButton loadBButton { "LOAD" };
    juce::TextButton loadTextureButton { "LOAD" };
    juce::TextButton oneShotAPreviousButton, oneShotANextButton;
    juce::TextButton oneShotBPreviousButton, oneShotBNextButton;
    juce::TextButton texturePreviousButton, textureNextButton;
    juce::ToggleButton reverseButton { "REVERSE" };
    juce::ToggleButton monoButton { "MONO" };
    juce::ToggleButton oneShotAEnabledButton { "ON" };
    juce::ToggleButton oneShotBEnabledButton { "ON" };
    juce::ToggleButton textureEnabledButton { "ON" };
    std::array<juce::ToggleButton, 18> randomLockButtons;

    juce::TextButton sceneAButton { "A" }, sceneBButton { "B" };
    juce::Slider sceneMorph;
    juce::Label rootDetectionLabel;

    juce::ComboBox userPresetBox, categoryBox, oneShotABox, oneShotBBox, textureBox, loopBox;
    juce::ComboBox delayTypeBox, delayDivisionBox, reverbTypeBox, lowpassTypeBox, highpassTypeBox;
    juce::ComboBox chorusTypeBox, crushTypeBox, driveTypeBox, phaserTypeBox;
    juce::ComboBox flangerTypeBox, tremoloTypeBox, compressorTypeBox;
    juce::Label libraryStatus;
    juce::TooltipWindow tooltipWindow;
    int currentPage = 0;
    std::vector<std::unique_ptr<SliderAttachment>> sliderAttachments;
    std::vector<std::unique_ptr<ButtonAttachment>> buttonAttachments;
    std::vector<std::unique_ptr<ComboBoxAttachment>> comboAttachments;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SiedAudioProcessorEditor)
};
