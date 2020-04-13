/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
FftServerAudioProcessorEditor::FftServerAudioProcessorEditor (FftServerAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    addAndMakeVisible(volumeSlider);
    volumeSlider.setRange(0.0, 1.0, 0.001); 
    volumeSlider.setSliderStyle(Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    volumeSlider.addListener(this);

    addAndMakeVisible(helloWorld);
    helloWorld.addListener(this);
    

    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (400, 300);
}

FftServerAudioProcessorEditor::~FftServerAudioProcessorEditor()
{
}

//==============================================================================
void FftServerAudioProcessorEditor::paint (Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));

    g.setColour (Colours::white);
    g.setFont (15.0f);
    g.drawFittedText ("Added Gain Control", getLocalBounds(), Justification::centred, 1);
}

void FftServerAudioProcessorEditor::resized()
{

    volumeSlider.setBounds(0, 0, 500, 100);
    helloWorld.setBounds(250, 250, 250, 250);
    //helloWorld.setBounds(Rectangle<int>(Point<int>(250, 250), Point<int>(250, 250)));
    

    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
}
