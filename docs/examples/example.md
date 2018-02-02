# 3dti AUDIOTOOLKIT Example

The 3DTI Audio Toolkit consists of three main components: binaural spatialiser, loudspeakers spatialiser, hearing loss and hearing aid simulator. 

## Binaural Spatialization
The first step is to instantiate and configure it: 
1. Instance the Core Class.
2. Configure the Audio State paremeters: sample rate (by default 44100 Hz) and buffer size (by default 512 samples). The Buffer Size value must be power of two.
3. Configure the HRTF resampling step (by default 5). Allowed integer values between 5 and 90. A low value implies more quality during the audio spatialization but also more memory consumed and more time is needed to load the HRTF. The recommended value for most cases is 45.
4. Instance the listener.

```c++
// TOOLKIT CORE INITIALIZATION
// 1. Create instance(s) of core class (one per listener)
Binaural::CCore   myCore;

// 2. Set values of Sample Rate and Buffer Size into the core. By default are set to 44100 and 512.
sampleRate = 48000; /* Sampling Rate {24000, 44100, 48000, 96000, ...} */
bufferSize = 256; /* Buffer Size {128, 256, 512, 1024, ...} */
myCore.SetAudioState({sampleRate, bufferSize});

// 3. Set HRTF resampling step. By default is set to 5.  
HRTF_resamplingStep = DEFAULT_SAMPLING_RATE /* Resampling Step [5 - 90] */
myCore.SetHRTFResamplingStep(HRTF_resamplingStep);	

// LISTENER INITIALIZATION
// 4. Create instance of listener class.
shared_ptr<Binaural::CListener> listener = myCore.CreateListener();	
```

Once instantiated and configured core class, you must create and configure each audio source to be rendered. This can be done at any time during the simulation.
For each audio source must be done:

5. Instatiate a sound source and ask the core to initialize it.
6. Select the spatialization mode {None, HighPerformance, HighQuality}. HighQuality is set by default. You can change the spatialization mode of each source at any moment.
7. Load auxiliary files, depending the spatialization mode selected. See the resources folder.
    * HighQuality: Load HRTF and ILDNearFieldEffectTable
    * HighPerformance: Load ILDSpatializationTable
    * None: none

```c++
// SOURCE CREATION 
// 5a. Create instance(s) of new source(s) (you can create as many sources as you need)
shared_ptr<Binaural::CSingleSourceDSP> mySource;
// 5b. Ask the core to initialize the source (for every sound source)
mySource = myCore.CreateSingleSourceDSP();

//SELECT SPATIALISATION MODE
// 6. Set the spatialization mode, HIGH QUALITY (by default), HIGH PERFORMANCE or NONE. 
mySource->SetSpatializationMode(Binaural::TSpatializationMode::HighQuality);
or 
mySource->SetSpatializationMode(Binaural::TSpatializationMode::HighPerformance);
or 
mySource->SetSpatializationMode(Binaural::TSpatializationMode::None);

// HIGH QUALITY MODE INTIALIZATION
// 7a. Load HRTF file, from a SOFA or 3DTI file, into the CHRTF head of the listener.
bool result = HRTF::CreateFromSofa( hrtfSofaFile_PATH , listener);
//or
bool result = HRTF::CreateFrom3dti(hrtf3DTIFile_PATH, listener);
if (result) { cout<< "HRTF has been loaded successfully"; }

// 7b. Load ILD for Near Field effect from 3DTI file.
bool result = ILD::CreateFrom3dti_ILDNearFieldEffectTable(fileILDNearFieldEffect.path(), listener);
if (result) { cout<< "ILD Near Field Effect simulation file has been loaded successfully";}

// HIGH PERFORMANCE MODE INTIALIZATION
// 7a. Load a ILD Spatialization Table from 3DTI file. 
bool result = ILD::CreateFrom3dti_ILDSpatializationTable(fileILDSpatialization.path(), listener);
if (result) { cout<< "ILD Spatialization simulation file has been loaded successfully"; }
```
---
Once the core class has been instantiated and configured, and also at least one audio source has been instantiated and configured, you will be able to spatialize sources during your simulation, and hear them. In order to do that, you should: 
* Move sound sources and listener positions in your **main (graphics) loop**.
* Feed the AudioToolkit with audio chunks in your **audio loop**:

**Main (graphics) loop: move the sound sources, move the listener** 
```c++
// 1. You may want to use a mutex to avoid races between mainLoop and audioLoop threads
std::mutex audioMutex;

Mainloop() {
  // 2. Set the transformation (position & orientation) for each source, for instance:
  for(auto & mySource: sources) 
  {
    CTransform newSourceTrf; 
    newSourceTrf.SetPosition(CVector3(x,y,z));	//Move source to absolute position
    { 
        lock_guard < mutex > lock(audioMutex);
        mySource->SetSourceTransform(newSourceTrf);
    }
  }
  // 3. Set the transformation (position & orientation) for the listener, for instance: 
  CTransform listenerTrf; 
  listernerTrf.SetOrientation(CQuaternion(qw, qx, qy, qz));
  {
    lock_guard < mutex > lock(audioMutex);
    listener->SetListenerTransform(listenerTrf);
  }
}
```
***Feed the core with audio chunks in your audio loop***. If you just want anechoic spatialization, then this is all you need to do:  

```c++
AudioLoop() // Assumes output is stereo interlaced
{
  // 1. Declare anechoic output mix, which consists of a pair of mono buffers (one for each channel/ear)  
  Common::CEarPair<CMonoBuffer<float>> bAnechoicOutput;
  bAnechoicOutput.left.resize(bufferSize);
  bAnechoicOutput.right.resize(bufferSize);

  // 2. Iterate through sources
  for(auto & mySource: sources) 
  {
    // 3. Get input chunk for this source 
	// Your audio framework should provide you with the necessary methods to obtain the chunk
    CMonoBuffer< float > bInput(bufferSize);

    // 4. Declare output for this source. Core assumes output is a pair of mono buffers (one for each channel/ear)
    Common::CEarPair<CMonoBuffer<float>> singleSourceAnechoicOut

    // 5. Spatialise this source, updating the input buffer and passing the output buffer
    {
      	lock_guard< mutex > lock(audioMutex);
	mySource->SetBuffer(bInput);
      	mySource->ProcessAnechoic(singleSourceAnechoicOut.left, singleSourceAnechoicOut.right);
    }
    
    // 6. Add this source output to the anechoic output mix
	bAnechoicOutput.left += singleSourceAnechoicOut.left;  
	bAnechoicOutput.right += singleSourceAnechoicOut.right;  
  }

  // 7. Mix with other non-spatialized sounds, apply other effects,...and finally pass result to audio framework. You may probably need to convert your pair of mono buffers into an array of stereo interlaced float samples. Assuming fOutput is the array of floats (float*) used by your audio framework, you could do:
  int stSample = 0;
  for (int i=0; i < bAnechoicOutput.GetNSamples(); i++)
  {
      fOutput[stSample++] = bAnechoicOutput.left[i];
      fOutput[stSample++] = bAnechoicOutput.right[i];
  }
}
```
---
**Reverb**

If you also want to add reverb, mixed with the anechoic output, start by following these steps:
1. Create a instance of the environment.
2. Load BRIR, from a SOFA or 3DTI file, of the room into the environment. See resources folder.

```c++
// 1. Create an instance of environment
std::shared_ptr<Binaural::CEnvironment> environment = myCore.CreateEnvironment();

// 2. Load BRIR (Binaural Room Impulse Response), either from SOFA or from 3DTI format:  
BRIR::CreateFromSofa( brirFile.path() , environment);
//or
BRIR::CreateFrom3dti(brirFile.path(), environment);
```

The following snippet shows a complete example with both anechoic spatialization and environment (reverb) simulation. The code to add with respect to the previous example starts in step 7.

```c++
AudioLoop() // Assumes output is stereo interlaced 
{
  // 1. Declare overall output mix
  Common::CEarPair<CMonoBuffer<float>> bSpatializedOutput;
  bSpatializedOutput.left.resize(bufferSize);
  bSpatializedOutput.right.resize(bufferSize);

  // 2. Iterate through sources, generating anechoic spatialisation
  // This loop includes the call to SetBuffer for each source, which is needed as well for reverb. 
  for(auto & mySource: sources) 
    {
      // 3. Get input chunk for this source 
	  // Your audio framework should provide you with the necessary methods to obtain the chunk
      CMonoBuffer< float > bInput(bufferSize);

      // 4. Declare output for this source. Core assumes output is a pair of mono buffers (one for each channel/ear)
      Common::CEarPair<CMonoBuffer<float>> singleSourceAnechoicOut

	  // 5. Spatialise this source, updating the input buffer and passing the output buffer
      {
        lock_guard< mutex > lock(audioMutex);
	    mySource->SetBuffer(bInput);
        mySource->ProcessAnechoic(singleSourceAnechoicOut.left, singleSourceAnechoicOut.right);
      }
      
      // 6. Add this source output to the overall output mix
      bSpatializedOutput.left += singleSourceAnechoicOut.left;  
	  bSpatializedOutput.right += singleSourceAnechoicOut.right;    
    }
	
  // 7. Process reverb and generate the reverb output
  Common::CEarPair<CMonoBuffer<float>> bReverbOutput;
  bReverbOutput.left.resize(bufferSize);
  bReverbOutput.right.resize(bufferSize);
  environment->ProcessVirtualAmbisonicOutput(bReverbOutput.left, bReverbOuptut.right);
  
  // 8. Mix anechoic and reverb
  bSpatializedOutput.left += bReverbOuptut.left; 
  bSpatializedOutput.right += bReverbOuptut.right; 
  
  // 9. Mix with other non-spatialized sounds, apply other effects,...and finally pass result to audio framework. Assuming fOutput is the array of floats (float*) used by your audio framework, you could do:
  int stSample = 0;
  for (int i=0; i < bSpatializedOutput.GetNSamples(); i++)
  {
      fOutput[stSample++] = bSpatializedOutput.left[i];
      fOutput[stSample++] = bSpatializedOutput.right[i];
  }
  
}
```

## Hearing Aid and Hearing Loss Simulation

You can use the toolkit for hearing aid simulation either with or without binaural spatialization. 
We first need to setup the simulator/s we want to use. 
First, Hearing Aid Simulation:

```c++
// 1. To setup hearing aid simulation, you first need to create a CHearingAidSim object:
HAHLSimulation::CHearingAidSim HAsimulator;

// 2. The Setup method of this class requires many input parameters. An example with typical default values is shown:
#define NUM_LEVELS 3
#define INITIAL_FREQ_HZ 125
#define BANDS_NUMBER 7
#define OCTAVE_BAND_STEP 1
#define CUTOFF_FREQ_LPF_Hz   3000.0f
#define CUTOFF_FREQ_HPF_Hz    500.0f
#define Q_LOW_PASS_FILTER    0.707
#define Q_BAND_PASS_FILTERS 1.4142
#define Q_HIGH_PASS_FILTER   0.707
HAsimulator.Setup(samplingRate, NUM_LEVELS, INITIAL_FREQ_HZ, BANDS_NUMBER, OCTAVE_BAND_STEP,
		              CUTOFF_FREQ_LPF_Hz, CUTOFF_FREQ_HPF_Hz, Q_LOW_PASS_FILTER, 
		              Q_BAND_PASS_FILTERS, Q_HIGH_PASS_FILTER);
					  
// 3. When you are ready with setup, enable hearing aid simulation for any ear/s you need (for both ears in the example):
HAsimulator.EnableHearingAidSimulation(Common::T_ear::BOTH);
```

Now, Hearing Loss Simulation:

```c++
// 1. To setup hearing loss simulation, you first need to create a CHearingLossSim object:
HAHLSimulation::CHearingLossSim HLsimulator;

// 2. The Setup method of this class requires many input parameters. An example with typical default values is shown:
#define DEF_dBs_SPL_for_0_dBs_fs 100.0f
#define HL_INITIAL_FREQ_HZ      62.5
#define HL_BANDS_NUMBER         9
#define HL_FILTERS_PER_BAND		3
HLsimulator.Setup(samplingRate, DEF_dBs_SPL_for_0_dBs_fs, HL_INITIAL_FREQ_HZ, HL_BANDS_NUMBER, HL_FILTERS_PER_BAND, bufferSize);
					  
// 3. When you are ready with setup, enable hearing loss simulation for any ear/s you need (for both ears in the example):
HLsimulator.EnableHearingLossSimulation(Common::T_ear::BOTH);
```

To process an audio buffer through hearing aid and/or hearing loss simulation, you need to add them to your process chain after spatialization:

```c++
// 1. Inside the audio loop, we continue with the previous example in which the spatialized output buffer pair is bSpatializedOutput. 
To process the spatialized audio through hearing aid simulation you first need to create a pair of output buffers for the hearing aid process:
Common::CEarPair<CMonoBuffer<float>> haOutputBuffer;
haOutputBuffer.left.resize(bufferSize);
haOutputBuffer.right.resize(bufferSize);

// 2. Now, apply the hearing aid process to the buffer with spatialized audio: 
HAsimulator.Process(bSpatializedOutput, haOutputBuffer);

// 3. For hearing loss simulation, you first need to create a pair of output buffers for the hearing loss process:
Common::CEarPair<CMonoBuffer<float>> hlOutputBuffer;
hlOutputBuffer.left.resize(bufferSize);
hlOutputBuffer.right.resize(bufferSize);

// 4. And now apply the hearing loss process to the hearing aid output buffer or, if you are using only hearing loss simulation, to the spatialized output buffer:
HLsimulator.Process(haOutputBuffer, hlOutputBuffer);

// 5. The hlOutputBuffer (or the haOutputBuffer if you are not using hearing loss simulation) can now be mixed with other non-spatialized sounds, other effects can be applied,...and finally the result is passed to audio framework. Assuming fOutput is the array of floats (float*) used by your audio framework, you could do:
  int stSample = 0;
  for (int i=0; i < hlOutputBuffer.GetNSamples(); i++)
  {
      fOutput[stSample++] = hlOutputBuffer.left[i];
      fOutput[stSample++] = hlOutputBuffer.right[i];
  }
```
## LoudSpeakers Spatialization
Coming soon
