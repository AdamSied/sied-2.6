#pragma once

#include <JuceHeader.h>
#include <array>
#include <cstdint>
#include <optional>
#include <vector>
#include "MacroEffects.h"
#include "SiedVoice.h"

class SiedAudioProcessor final : public juce::AudioProcessor
{
public:
    SiedAudioProcessor();
    ~SiedAudioProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout&) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 12.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {};
    }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    bool loadSampleFile (const juce::File&, SiedLayer = SiedLayer::oneShotA);
    bool loadOneShot (SiedLayer, int libraryIndex);
    bool loadTexture (int libraryIndex);
    bool stepOneShot (SiedLayer, int direction);
    bool stepTexture (int direction);
    bool loadFactoryPreset (int presetIndex);
    void randomizeOneShots();
    void randomizeTexture();
    void randomizePreset();
    void captureRandomizationUndo();
    bool undoLastRandomization();
    bool canUndoRandomization() const { return ! randomizationUndoHistory.empty(); }
    void captureScene (int sceneIndex);
    void applySceneMorph (float position);
    bool hasScene (int sceneIndex) const;
    void resetToInitPatch();
    bool saveUserPreset (const juce::String& presetName);
    bool loadUserPreset (const juce::String& presetName);
    juce::StringArray getUserPresetNames() const;
    juce::String getUserPresetFolderPath() const;
    void requestEffectsReset()
    {
        resetEffectsRequested.store (true, std::memory_order_release);
    }
    void rescanLibrary();

    int getOneShotCount() const;
    juce::String getOneShotName (int) const;
    juce::String getOneShotCategory (int) const;
    int getTextureCount() const;
    juce::String getTextureName (int) const;
    juce::String getTextureCategory (int) const;
    int getSelectedOneShot (SiedLayer) const;
    int getSelectedTexture() const { return selectedTexture; }
    juce::String getCurrentSoundName (SiedLayer) const;
    juce::String getLibraryFolderPath() const;
    bool isFavourite (SiedLayer, int libraryIndex) const;
    void toggleFavourite (SiedLayer, int libraryIndex);
    juce::String getLastDetectedRootText() const;
    bool isRandomizationLocked (const juce::String& parameterID) const;
    float getPlaybackPosition (SiedLayer layer) const
    {
        return displayedPlayheads[static_cast<size_t> (layer)].load (std::memory_order_relaxed);
    }

    int getCurrentFactoryPreset() const { return selectedOneShots[0]; }
    int getFactoryPresetCount() const;
    juce::String getFactoryPresetName (int presetIndex) const;
    juce::String getFactoryPresetCategory (int presetIndex) const;
    std::shared_ptr<const SiedSampleData> getSampleData (SiedLayer layer) const
    {
        return sampleBank.getSample (layer);
    }
    std::shared_ptr<const SiedSampleData> getCurrentSampleData() const
    {
        return getSampleData (SiedLayer::oneShotA);
    }
    juce::AudioProcessorValueTreeState& getParameters() { return parameters; }

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

private:
    struct LibraryEntry
    {
        juce::String name;
        juce::String category;
        juce::File file;
        int embeddedIndex = -1;
    };

    juce::AudioProcessorValueTreeState parameters;
    juce::Synthesiser synth;
    SiedSampleBank sampleBank;
    MacroEffects macroEffects;
    juce::AudioFormatManager formatManager;
    std::vector<LibraryEntry> oneShotLibrary;
    std::vector<LibraryEntry> textureLibrary;
    std::array<juce::String, 3> loadedSamplePaths;
    std::array<juce::String, 3> currentSoundNames { "Ascent", "Glow", "Vinyl Crackles" };
    std::array<int, 2> selectedOneShots { 0, 10 };
    int selectedTexture = 0;
    juce::StringArray favouriteKeys;
    juce::String lastDetectedRootText { "TUNING —" };
    std::array<juce::ValueTree, 2> scenes;
    int lastSceneSampleSide = -1;
    std::vector<juce::ValueTree> randomizationUndoHistory;
    std::atomic<bool> resetEffectsRequested { false };
    std::array<std::atomic<float>, static_cast<size_t> (SiedLayer::count)> displayedPlayheads
    {{ -1.0f, -1.0f, -1.0f }};

    bool loadSampleFileInternal (const juce::File&, SiedLayer, bool resetPlaybackRegion);
    bool loadFactoryToLayer (int presetIndex, SiedLayer, bool applyPresetParameters);
    bool recallOneShot (SiedLayer, int libraryIndex);
    bool recallTexture (int libraryIndex);
    bool loadEmbeddedTexture();
    int findOneShotByName (const juce::String&, bool exact) const;
    int findTextureByName (const juce::String&, bool exact) const;
    bool installReader (std::unique_ptr<juce::AudioFormatReader>, SiedLayer,
                        const juce::String& name, float gainDb, const juce::String& sourcePath,
                        bool autoNormalise, int loopStart = 0, int loopEnd = 0,
                        bool detectRoot = false);
    void addExternalLibraryFiles (const juce::File&, bool textures);
    void configureVoiceCount();
    void randomizeOneShots (juce::Random&);
    void randomizeTexture (juce::Random&);
    bool isLocked (const char* lockID) const;
    struct RootDetection
    {
        int midiNote = 60;
        float correctionCents = 0.0f;
        float confidence = 0.0f;
    };
    juce::ValueTree captureSceneTree() const;
    void restoreSceneSamples (const juce::ValueTree&);
    static std::optional<RootDetection> detectRootNote (
        const juce::AudioBuffer<float>&, double sampleRate);
    juce::String favouriteKey (SiedLayer, int libraryIndex) const;
    juce::File getFavouritesFile() const;
    void loadFavourites();
    void saveFavourites() const;
    juce::ValueTree createStateTree();
    void restoreStateTree (juce::ValueTree);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SiedAudioProcessor)
};
