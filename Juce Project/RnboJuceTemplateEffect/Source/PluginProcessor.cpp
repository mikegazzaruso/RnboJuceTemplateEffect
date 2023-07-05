/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "ParamIds.h"

static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    auto msFormat = [](float value, int)
    {
        if (value < 100.0f)
            return juce::String (value, 1) + " ms";
        else
            return juce::String (std::roundf (value)) + " ms";
    };

    layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { ParamIDs::delaytimeL, 1 },
                                                             "delaytimeL",
                                                             juce::NormalisableRange<float> (1.0f, 2000.0f, 0.01f, 1.0f),
                                                             20.0f,
                                                             juce::String(),
                                                             juce::AudioProcessorParameter::genericParameter,
                                                             msFormat,
                                                             nullptr));

    layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { ParamIDs::delaytimeR, 1 },
                                                             "delaytimeR",
                                                             juce::NormalisableRange<float> (1.0f, 2000.0f, 0.01f, 1.0f),
                                                             20.0f,
                                                             juce::String(),
                                                             juce::AudioProcessorParameter::genericParameter,
                                                             msFormat,
                                                             nullptr));
    
    layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { ParamIDs::feedbackL, 1 },
                                                             "feedbackL",
                                                             juce::NormalisableRange<float> (0.0f, 0.999f, 0.01f, 1.0f),
                                                             0.2f,
                                                             juce::String(),
                                                             juce::AudioProcessorParameter::genericParameter,
                                                             nullptr,
                                                             nullptr));
    
    layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { ParamIDs::feedbackR, 1 },
                                                             "feedbackR",
                                                             juce::NormalisableRange<float> (0.0f, 0.999f, 0.01f, 1.0f),
                                                             0.2f,
                                                             juce::String(),
                                                             juce::AudioProcessorParameter::genericParameter,
                                                             nullptr,
                                                             nullptr));
    
    layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { ParamIDs::dryWetMix, 1 },
                                                             "dryWetMix",
                                                             juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f, 1.0f),
                                                             0.2f,
                                                             juce::String(),
                                                             juce::AudioProcessorParameter::genericParameter,
                                                             nullptr,
                                                             nullptr));

    return layout;
}


//==============================================================================
RnboJuceTemplateEffectAudioProcessor::RnboJuceTemplateEffectAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
#endif
    apvts (*this, &undoManager, "Parameters", createParameterLayout())
{
    for (RNBO::ParameterIndex i = 0; i < rnboObject.getNumParameters(); ++i)
    {
        RNBO::ParameterInfo info;
        rnboObject.getParameterInfo (i, &info);

        if (info.visible)
        {
            auto paramID = juce::String (rnboObject.getParameterId (i));

            // Each apvts parameter id and range must be the same as the rnbo param object's.
            // If you hit this assertion then you need to fix the incorrect id in ParamIDs.h.
            jassert (apvts.getParameter (paramID) != nullptr);

            // If you hit these assertions then you need to fix the incorrect apvts
            // parameter range in createParameterLayout().
            // jassert (info.min == apvts.getParameterRange (paramID).start);
            // jassert (info.max == apvts.getParameterRange (paramID).end);

            apvtsParamIdToRnboParamIndex[paramID] = i;

            apvts.addParameterListener (paramID, this);
            rnboObject.setParameterValue (i, apvts.getRawParameterValue (paramID)->load());
        }
    }
}

RnboJuceTemplateEffectAudioProcessor::~RnboJuceTemplateEffectAudioProcessor()
{
}

//==============================================================================
const juce::String RnboJuceTemplateEffectAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool RnboJuceTemplateEffectAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool RnboJuceTemplateEffectAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool RnboJuceTemplateEffectAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double RnboJuceTemplateEffectAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int RnboJuceTemplateEffectAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int RnboJuceTemplateEffectAudioProcessor::getCurrentProgram()
{
    return 0;
}

void RnboJuceTemplateEffectAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String RnboJuceTemplateEffectAudioProcessor::getProgramName (int index)
{
    return {};
}

void RnboJuceTemplateEffectAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void RnboJuceTemplateEffectAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    rnboObject.prepareToProcess (sampleRate, static_cast<size_t> (samplesPerBlock));
}

void RnboJuceTemplateEffectAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool RnboJuceTemplateEffectAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void RnboJuceTemplateEffectAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused (midiMessages);
    auto bufferSize = buffer.getNumSamples();
    
    rnboObject.prepareToProcess (getSampleRate(),
                                 static_cast<size_t> (bufferSize));
    
    rnboObject.process (buffer.getArrayOfWritePointers(),
                        static_cast<RNBO::Index> (buffer.getNumChannels()),
                        buffer.getArrayOfWritePointers(),
                        static_cast<RNBO::Index> (buffer.getNumChannels()),
                        static_cast<RNBO::Index> (bufferSize));
}

//==============================================================================
bool RnboJuceTemplateEffectAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* RnboJuceTemplateEffectAudioProcessor::createEditor()
{
    return new GenericAudioProcessorEditor (*this);
}

//==============================================================================
void RnboJuceTemplateEffectAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    juce::MemoryOutputStream mos (destData, true);
    apvts.state.writeToStream (mos);
}

void RnboJuceTemplateEffectAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    auto tree = juce::ValueTree::readFromData (data, static_cast<size_t> (sizeInBytes));

    if (tree.isValid())
        apvts.replaceState (tree);
}

void RnboJuceTemplateEffectAudioProcessor::parameterChanged (const juce::String& parameterID, float newValue)
{
    rnboObject.setParameterValue (apvtsParamIdToRnboParamIndex[parameterID], newValue);
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new RnboJuceTemplateEffectAudioProcessor();
}
