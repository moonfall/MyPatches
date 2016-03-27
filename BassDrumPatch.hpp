#ifndef __BassDrumPatch_hpp__
#define __BassDrumPatch_hpp__

#include "StompBox.h"
#include "SmoothValue.h"
#include "BiquadFilter.h"
#include "Envelope.h"
#include "Oscillator.h"
#include "NoiseOscillator.h"
#include "Oscillators.hpp"

class ExponentialDecayEnvelope {
private:
  const float fs;
  float value;
  float incr;
public:
  ExponentialDecayEnvelope(float sr)
    : fs(sr){}
  void setRate(float r){
    if(r < 0)
      incr = 1.0f - 100*(1/(1-r))/fs;
    else
      incr = 1.0f + 100*r/fs;
  }
  void setDecay(float d){
    setRate(-(d+1/fs));
  }
  void trigger(){
    value = 1.0;
  }
  float getNextSample(){
    float sample = value;
    value *= incr;
    return sample;
  }
};
  
class DrumVoice : public Oscillator {
private:
  // AdsrEnvelope* env;
  // FloatArray env;
  SineOscillator* sine;
  ChirpOscillator* chirp;
  ImpulseOscillator* impulse;
  ExponentialDecayEnvelope* env1;
  ExponentialDecayEnvelope* env2;
  Oscillator* noise;
  BiquadFilter* filter;
  float freq;
  float decay;
  SmoothFloat snare;
  float balance;
  const float fs;
public:
  DrumVoice(float sr) : fs(sr) {
    // env = new AdsrEnvelope(sr);
    // env = FloatArray::create(1024);
    // for(int i=0; i<env.getSize(); ++i)
    //   env[i] = expf(-M_E*i/env.getSize());
    snare = 0;
    balance = 0.2;
    sine = new SineOscillator(sr);
    chirp = new ChirpOscillator(sr);
    impulse = new ImpulseOscillator();
    env1 = new ExponentialDecayEnvelope(sr);
    env2 = new ExponentialDecayEnvelope(sr);
    noise = new PinkNoiseOscillator();
    filter = BiquadFilter::create(1);
    filter->setLowPass(0.6, FilterStage::BUTTERWORTH_Q);
  }  
  void setFrequency(float f){
    freq = f;
    // filter->setLowPass(f*2/(fs*2), FilterStage::BUTTERWORTH_Q);
  }
  void setDecay(float d){
    decay = d;
  }
  void setSnap(float s){
    snare = s;
    balance = s*0.5;
    filter->setLowPass(0.25+balance, FilterStage::BUTTERWORTH_Q);
  }
  void trigger(){
    sine->setFrequency(freq);
    chirp->setFrequency(freq*2);
    env1->setDecay(decay);
    env2->setDecay(decay*snare*0.5);
    chirp->setDecay(decay);
    env1->trigger();
    env2->trigger();
    chirp->trigger();
    impulse->trigger();
  }
  float getNextSample(){
    float vca1 = sine->getNextSample();
    vca1 += chirp->getNextSample();
    vca1 *= env1->getNextSample();

    float vca2 = 0.0f;
    vca2 += impulse->getNextSample();
    // vca2 += filter->process(noise->getNextSample());
    // vca2 *= env2->getNextSample();
    vca2 += noise->getNextSample();
    vca2 = filter->process(vca2);
    vca2 *= env2->getNextSample();
    
    float sample = vca1*(1.0-balance) + vca2*balance;
    return sample;
  }
};

class BassDrumPatch : public Patch {
private:
  DrumVoice* kick;
  bool buttonstate = false;
public:
  BassDrumPatch(){
    registerParameter(PARAMETER_A, "Tone");
    registerParameter(PARAMETER_B, "Decay");
    registerParameter(PARAMETER_C, "Snare");
    registerParameter(PARAMETER_D, "Level");
    kick = new DrumVoice(getSampleRate());
  }
  ~BassDrumPatch(){
  }
  void processAudio(AudioBuffer& buffer){
    float tone = 20*powf(2, getParameterValue(PARAMETER_A)*4);
    float decay = getParameterValue(PARAMETER_B)*100;
    float c = getParameterValue(PARAMETER_C);
    float level = getParameterValue(PARAMETER_D)*2;
    kick->setDecay(decay);
    kick->setFrequency(tone);
    FloatArray left = buffer.getSamples(LEFT_CHANNEL);
    FloatArray right = buffer.getSamples(RIGHT_CHANNEL);
    kick->setSnap(c);
    if(isButtonPressed(PUSHBUTTON) != buttonstate){
      buttonstate = isButtonPressed(PUSHBUTTON);
      if(buttonstate)
	kick->trigger();
    }
    kick->getSamples(left);
    left.multiply(level);
    right.copyFrom(left);
  }
};

#endif   // __BassDrumPatch_hpp__
