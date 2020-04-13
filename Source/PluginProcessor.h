/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

//==============================================================================
/**
*/
class FftServerAudioProcessor  : public AudioProcessor,
    private Timer
{
public:
    //==============================================================================
    FftServerAudioProcessor();
    ~FftServerAudioProcessor();

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (AudioBuffer<float>&, MidiBuffer&) override;

    void pushNextSampleIntoFifo(float sample) noexcept //circular buffer
    {
        if (fifoIndex == fftSize)    // [11]
        {
            if (!nextFFTBlockReady) // [12]
            {
                zeromem(fftData, sizeof(fftData)); //clears buffer
                memcpy(fftData, fifo, sizeof(fifo));
                nextFFTBlockReady = true;
            }

            fifoIndex = 0;
        }

        fifo[fifoIndex++] = sample;  // [12]
    }


    void timerCallback() override  //
    {
        if (nextFFTBlockReady)
        {
            drawNextFrameOfSpectrum();
            nextFFTBlockReady = false;
            //repaint();
        }
    }


    void drawNextFrameOfSpectrum() //likely conflicts with Paint() from PluginEditor.h somehow
    {
        window.multiplyWithWindowingTable(fftData, fftSize);      // [1]

        forwardFFT.performFrequencyOnlyForwardTransform(fftData); // [2]

        auto mindB = -100.0f;
        auto maxdB = 0.0f;

        for (int i = 0; i < scopeSize; ++i)                        // [3]
        {
            auto skewedProportionX = 1.0f - std::exp(std::log(1.0f - i / (float)scopeSize) * 0.2f);
            auto fftDataIndex = jlimit(0, fftSize / 2, (int)(skewedProportionX * fftSize / 2));
            auto level = jmap(jlimit(mindB, maxdB, Decibels::gainToDecibels(fftData[fftDataIndex])
                - Decibels::gainToDecibels((float)fftSize)),
                mindB, maxdB, 0.0f, 1.0f);
           
            scopeData[i] = level;                                  // [4]
        }
        sendToClients(scopeData);
    }

    //==============================================================================
    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const String getProgramName (int index) override;
    void changeProgramName (int index, const String& newName) override;

    //==============================================================================
    void getStateInformation (MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    enum
    {
        fftOrder = 4, //11 = 2048 points
        fftSize = 1 << fftOrder,
        scopeSize = 16 //changed from 512
    };


//BEGIN COPIED CODE

    void sendToClients(float msg1[]) //COPY THIS OVER sending to any connected client      debug.log() unity //possibly change to float
    {
        float msg[16] = {}; //EQUAL TO SIZE OF FFT ARRAY (THIS SOLVED ISSUE 032420-only first two floats being sent)

        const ScopedLock myScopedLock(sockLock); //protects modification of variables
        MemoryBlock m1(msg1, sizeof(msg)); //CHANGED FROM "sizeof(msg1)" to "sizeof(msg)" + line 198 to fix missing values in unity...
        for (int i = activeConnections.size(); --i >= 0;)
        {
            activeConnections[i]->sendMessage(m1);
        }
    }

    class ArbInterprocessConnection : public InterprocessConnection // COPY THIS OVER owner.sendToClient sendToClient attacehd to main component.
    {
    public:
        ArbInterprocessConnection(FftServerAudioProcessor& owner_)
            : InterprocessConnection(true),
            owner(owner_)
        {
            static int totalConnections = 0;
            ourNumber = ++totalConnections;
        }
        void connectionMade()
        {
            //owner.appendMessage("Connection #" + String(ourNumber) + " - connection started");
        }
        void connectionLost()
        {
            //owner.appendMessage("Connection #" + String(ourNumber) + " - connection lost");
        }
        void messageReceived(const MemoryBlock& message)
        {

        }

    private:
        FftServerAudioProcessor& owner;
        int ourNumber;
    };

    //==============================================================================
    class ArbInterprocessConnectionServer : public InterprocessConnectionServer //COPY THIS OVER
    {
    public:
        ArbInterprocessConnectionServer(FftServerAudioProcessor& owner_)
            : owner(owner_)
        {
        }
        InterprocessConnection* createConnectionObject()
        {
            ArbInterprocessConnection* newConnection = new ArbInterprocessConnection(owner);
            owner.activeConnections.add(newConnection);
            return newConnection;
        }
    private:
        FftServerAudioProcessor& owner;
    };

    OwnedArray <ArbInterprocessConnection, CriticalSection> activeConnections; //DEFINES ACTIVE CONNECTIONS
    CriticalSection sockLock;
    float msgFft[4]; //array to test sending

//END COPIED CODE

    double rawVolume;



private:

    ScopedPointer<ArbInterprocessConnectionServer> server;  //COPY THIS OVER


    dsp::FFT forwardFFT;
    dsp::WindowingFunction<float> window;

    float fifo[fftSize];
    float fftData[2 * fftSize];
    int fifoIndex = 0;
    bool nextFFTBlockReady = false;
    float scopeData[scopeSize];

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FftServerAudioProcessor)
};





//==============================================================================
/*
class AnalyserComponent : public AudioAppComponent,
    private Timer
{
public:

    AnalyserComponent()
        : forwardFFT(fftOrder),
        window(fftSize, dsp::WindowingFunction<float>::hann)
    {
        setOpaque(true);
        setAudioChannels(2, 0);  // we want a couple of input channels but no outputs
        startTimerHz(30);
        setSize(700, 500);

    }

    ~AnalyserComponent() override
    {
        shutdownAudio();
    }
    
    //==============================================================================
    void prepareToPlay(int, double) override {}
    void releaseResources() override {}
    
    void getNextAudioBlock(const AudioSourceChannelInfo& bufferToFill) override
    {
        if (bufferToFill.buffer->getNumChannels() > 0)
        {
            auto* channelData = bufferToFill.buffer->getReadPointer(0, bufferToFill.startSample);

            for (auto i = 0; i < bufferToFill.numSamples; ++i)
                pushNextSampleIntoFifo(channelData[i]);
        }


    }

    
    //==============================================================================
    /*
    void paint(Graphics& g) override
    {
        g.fillAll(Colours::black);

        g.setOpacity(1.0f);
        g.setColour(Colours::white);
        drawFrame(g);
    }
    

    void timerCallback() override
    {
        if (nextFFTBlockReady)
        {
            drawNextFrameOfSpectrum();
            nextFFTBlockReady = false;
            repaint();
        }
    }

    void pushNextSampleIntoFifo(float sample) noexcept //circular buffer
    {
        if (fifoIndex == fftSize)    // [11]
        {
            if (!nextFFTBlockReady) // [12]
            {
                zeromem(fftData, sizeof(fftData)); //clears buffer
                memcpy(fftData, fifo, sizeof(fifo));
                nextFFTBlockReady = true;
            }

            fifoIndex = 0;
        }

        fifo[fifoIndex++] = sample;  // [12]
    }

    
    void drawNextFrameOfSpectrum() //likely conflicts with Paint() from PluginEditor.h somehow
    {
        window.multiplyWithWindowingTable(fftData, fftSize);      // [1]

        forwardFFT.performFrequencyOnlyForwardTransform(fftData); // [2]

        auto mindB = -100.0f;
        auto maxdB = 0.0f;

        for (int i = 0; i < scopeSize; ++i)                        // [3]
        {
            auto skewedProportionX = 1.0f - std::exp(std::log(1.0f - i / (float)scopeSize) * 0.2f);
            auto fftDataIndex = jlimit(0, fftSize / 2, (int)(skewedProportionX * fftSize / 2));
            auto level = jmap(jlimit(mindB, maxdB, Decibels::gainToDecibels(fftData[fftDataIndex])
                - Decibels::gainToDecibels((float)fftSize)),
                mindB, maxdB, 0.0f, 1.0f);

            scopeData[i] = level;                                  // [4]
        }
    }

    /*
    void drawFrame(Graphics& g)
    {
        for (int i = 1; i < scopeSize; ++i)
        {
            auto width = getLocalBounds().getWidth();
            auto height = getLocalBounds().getHeight();

            g.drawLine({ (float)jmap(i - 1, 0, scopeSize - 1, 0, width),
                                  jmap(scopeData[i - 1], 0.0f, 1.0f, (float)height, 0.0f),
                          (float)jmap(i,     0, scopeSize - 1, 0, width),
                                  jmap(scopeData[i],     0.0f, 1.0f, (float)height, 0.0f) });
        }
    }
    

    enum
    {
        fftOrder = 11, //11 = 2048 points
        fftSize = 1 << fftOrder,
        scopeSize = 512
    };
    
private:

    dsp::FFT forwardFFT;
    dsp::WindowingFunction<float> window;

    float fifo[fftSize];
    float fftData[2 * fftSize];
    int fifoIndex = 0;
    bool nextFFTBlockReady = false;
    float scopeData[scopeSize];


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AnalyserComponent)
};
*/