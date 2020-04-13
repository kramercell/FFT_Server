/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class FftServerAudioProcessorEditor  : public AudioProcessorEditor, Slider::Listener, TextButton::Listener
{
public:
    FftServerAudioProcessorEditor (FftServerAudioProcessor&);
    ~FftServerAudioProcessorEditor();

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;

    void sliderValueChanged(Slider* slider) override //overrides virtual function of slider class
    {
        if (slider == &volumeSlider)
        {
            //do stuff here
            //Logger::writeToLog(String(volumeSlider.getValue()));
            //PluginTest040120AudioProcessor
        }
    }
    
    void buttonClicked(Button* button) override
    {
        if (button == &helloWorld)
        {
            Logger::writeToLog("Hello World");
        }
    }


private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    FftServerAudioProcessor& processor;

    Slider volumeSlider; //{Slider::RotaryHorizontalVerticalDrag};
    TextButton helloWorld{ "vst" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FftServerAudioProcessorEditor)
};
