/*
* \class CJitterSimulator
*
* \brief Implementation of CJitterSimulator class. This class implements time jitter simulation, modeling a potential symptom of 
* age-related sensorineural hearing loss, where cortical speech processing may be limited by age-related decreases
* in the precision of neural synchronization in the midbrain.
*
* \authors 3DI-DIANA Research Group (University of Malaga), in alphabetical order: M. Cuevas-Rodriguez, C. Garre,  D. Gonzalez-Toledo, E.J. de la Rubia-Cuestas, L. Molina-Tanco ||
* Coordinated by , A. Reyes-Lecuona (University of Malaga) and L.Picinali (Imperial College London) ||
* \b Contact: areyes@uma.es and l.picinali@imperial.ac.uk
*
* \b Contributions: (additional authors/contributors can be added here)
*
* \b Project: 3DTI (3D-games for TUNing and lEarnINg about hearing aids) ||
* \b Website: http://3d-tune-in.eu/
*
* \b Copyright: University of Malaga and Imperial College London - 2018
*
* \b Licence: This copy of 3dti_AudioToolkit is licensed to you under the terms described in the 3DTI_AUDIOTOOLKIT_LICENSE file included in this distribution.
*
* \b Acknowledgement: This project has received funding from the European Union's Horizon 2020 research and innovation programme under grant agreement No 644051
*/

#include <HAHLSimulation/TemporalDistortionSimulator.h>

namespace HAHLSimulation {

	/////////////////////////////////////////////////////////////
	//// Default constructor
	//CTemporalDistortionSimulator::CTemporalDistortionSimulator()
	//{
	//	processBufferSize = 0;
	//	sampleRate = 0;
	//	jitterDelayBufferSize = 0;
	//	maxSampleOffset = 0;													
	//	leftRightNoiseSynchronicity = 0.0f;
	//	//CEarPair<CLowHighSplitFilter> splitFilters;			///< Filters splitting low and high frequency contents into separate buffers, for each ear
	//	//CEarPair<Common::CNoiseGenerator> noiseGenerators;	///< Noise generators for jitter source, for each ear
	//	//CEarPair<Common::CDelay> jitterDelayBuffers;		///< Delay buffers for jitter process (in low frequencies), for each ear
	//	//CEarPair<Common::CDelay> highFrequencyDelayBuffers;	///< Delay buffers for high frequencies, for each ear
	//}

	///////////////////////////////////////////////////////////
	// Setup the time jitter simulator
	void CTemporalDistortionSimulator::Setup(int samplingRate, int bufferSize, int bandUpperLimit, float noisePower, float leftRightSynchronicity)
	{
		// Store initial attribute values
		sampleRate = samplingRate;
		processBufferSize = bufferSize;
		leftRightNoiseSynchronicity = leftRightSynchronicity;
		bandUpperLimitLastLeftValue = bandUpperLimit;
		whiteNoisePowerLastLeftValue = noisePower;
		noiseAutocorrelationFilterCutoffFrequencyLastLeftValue = DEFAULT_NOISE_AUTOCORRELATION_CUTOFF;
		power.left = 0.0f;
		autocorrelation.left = 0.0f;
		power.right = 0.0f;
		autocorrelation.right = 0.0f;
		doLeftRightNoiseSynchronicity = false;
		doTemporalDistortionSimulator.left = true;
		doTemporalDistortionSimulator.right = true;
		autocorrelationTimeShift_ms = 1.0f;

		// Setup frequency-split filters
		preLPFFilter.left.Setup(samplingRate, bandUpperLimit, Common::T_filterType::LOWPASS, 4);
		preLPFFilter.right.Setup(samplingRate, bandUpperLimit, Common::T_filterType::LOWPASS, 4);
		preHPFFilter.left.Setup(samplingRate, bandUpperLimit, Common::T_filterType::HIGHPASS, 4);
		preHPFFilter.right.Setup(samplingRate, bandUpperLimit, Common::T_filterType::HIGHPASS, 4);
		postLPFFilter.left.Setup(samplingRate, bandUpperLimit, Common::T_filterType::LOWPASS, 4);
		postLPFFilter.right.Setup(samplingRate, bandUpperLimit, Common::T_filterType::LOWPASS, 4);
		postHPFFilter.left.Setup(samplingRate, bandUpperLimit, Common::T_filterType::HIGHPASS, 4);
		postHPFFilter.right.Setup(samplingRate, bandUpperLimit, Common::T_filterType::HIGHPASS, 4);

		// Setup noise generators
		noiseGenerators.left.Setup(0.0f);
		noiseGenerators.left.EnableAutocorrelationFilter();
		noiseGenerators.left.SetupAutocorrelationFilter(samplingRate, DEFAULT_NOISE_AUTOCORRELATION_CUTOFF, DEFAULT_NOISE_AUTOCORRELATION_Q); // TO DO: do something else with cutoff and Q
		noiseGenerators.right.Setup(0.0f);
		noiseGenerators.right.EnableAutocorrelationFilter();
		noiseGenerators.right.SetupAutocorrelationFilter(samplingRate, DEFAULT_NOISE_AUTOCORRELATION_CUTOFF, DEFAULT_NOISE_AUTOCORRELATION_Q); // TO DO: do something else with cutoff and Q	
		SetWhiteNoisePower(Common::T_ear::BOTH, noisePower);

		// Setup jitter process	
		maxSampleOffset = processBufferSize / 2;	// TO DO: what happens if buffer size is even?
		jitterDelayBufferSize = bufferSize + maxSampleOffset * 2;
		jitterDelayBuffers.left.Setup(maxSampleOffset * 2);
		jitterDelayBuffers.right.Setup(maxSampleOffset * 2);

		// Setup high frequencies delay 
		highFrequencyDelayBuffers.left.Setup(maxSampleOffset);
		highFrequencyDelayBuffers.right.Setup(maxSampleOffset);

		// Setup bypass  
		bypassLowDelays.left.Setup(maxSampleOffset);
		bypassLowDelays.right.Setup(maxSampleOffset);
		bypassHighDelays.left.Setup(maxSampleOffset);
		bypassHighDelays.right.Setup(maxSampleOffset);
		bypassPreLPFFilter.left.Setup(samplingRate, bandUpperLimit, Common::T_filterType::LOWPASS, 4);
		bypassPreLPFFilter.right.Setup(samplingRate, bandUpperLimit, Common::T_filterType::LOWPASS, 4);
		bypassPreHPFFilter.left.Setup(samplingRate, bandUpperLimit, Common::T_filterType::HIGHPASS, 4);
		bypassPreHPFFilter.right.Setup(samplingRate, bandUpperLimit, Common::T_filterType::HIGHPASS, 4);
		bypassPostLPFFilter.left.Setup(samplingRate, bandUpperLimit, Common::T_filterType::LOWPASS, 4);
		bypassPostLPFFilter.right.Setup(samplingRate, bandUpperLimit, Common::T_filterType::LOWPASS, 4);
		bypassPostHPFFilter.left.Setup(samplingRate, bandUpperLimit, Common::T_filterType::HIGHPASS, 4);
		bypassPostHPFFilter.right.Setup(samplingRate, bandUpperLimit, Common::T_filterType::HIGHPASS, 4);
	}
	
	///////////////////////////////////////////////////////////
	// Process an input buffer												 
	//void CTemporalDistortionSimulator::Process(CMonoBuffer<float> & leftInputBuffer, CMonoBuffer<float> & rightInputBuffer, CMonoBuffer<float> & leftOutputBuffer, CMonoBuffer<float> & rightOutputBuffer)
	void CTemporalDistortionSimulator::Process(Common::CEarPair<CMonoBuffer<float>> & inputBuffer, Common::CEarPair<CMonoBuffer<float>> & outputBuffer)
	{
		// Check errors
		ASSERT(inputBuffer.left.size() == processBufferSize, RESULT_ERROR_BADSIZE, "Input buffer size for temporal Distortion simulator (HL) is wrong", "");
		ASSERT(inputBuffer.left.size() == inputBuffer.right.size(), RESULT_ERROR_BADSIZE, "Size of input buffers for temporal Distortion simulator (HL) is wrong", "");
		ASSERT(inputBuffer.left.size() == outputBuffer.left.size(), RESULT_ERROR_BADSIZE, "Size of output buffers for temporal Distortion simulator (HL) is wrong", "");
		ASSERT(outputBuffer.left.size() == outputBuffer.right.size(), RESULT_ERROR_BADSIZE, "Size of output buffers for temporal Distortion simulator (HL) is wrong", "");

		// Full bypass
		if ((!doTemporalDistortionSimulator.left) && (!doTemporalDistortionSimulator.right))
		{
			outputBuffer = inputBuffer;
			return;			
		}

		// Calculate noise sources for each ear
		CMonoBuffer<float> leftNoiseBuffer(outputBuffer.left.size());
		CMonoBuffer<float> rightNoiseBuffer(outputBuffer.right.size());
		noiseGenerators.left.Process(leftNoiseBuffer);
		noiseGenerators.right.Process(rightNoiseBuffer);
		rightNoiseBuffer = leftNoiseBuffer*leftRightNoiseSynchronicity + rightNoiseBuffer*(1.0f - leftRightNoiseSynchronicity);

		// Get number of samples for measuring autocorrelation
		float autocorrelationShift_samples = GetNumberOfSamplesFromTimeInMiliseconds(autocorrelationTimeShift_ms);

		// Process left ear
		if (doTemporalDistortionSimulator.left)
		{
			// 1. Split frequencies
			CMonoBuffer<float> lowBuffer(outputBuffer.left.size());
			CMonoBuffer<float> highBuffer(outputBuffer.left.size());
			preLPFFilter.left.Process(inputBuffer.left, lowBuffer);
			preHPFFilter.left.Process(inputBuffer.left, highBuffer);			

			//// 2. Do a dummy process of the bypass delay buffers, to minimize clicks when enabling/disabling one ear
			//// NOTE: this should be done with the bypass filters...
			//CMonoBuffer<float> dummyBuffer(lowBuffer.size());
			//bypassLowDelays.left.Process(lowBuffer, dummyBuffer);
			//bypassHighDelays.left.Process(highBuffer, dummyBuffer);

			// 3. Compute autocorrelation coefficients for noise sources			
			power.left = leftNoiseBuffer.GetPower();
			autocorrelation.left = leftNoiseBuffer.GetAutocorrelation(autocorrelationShift_samples);

			// 4. Process jitter
			// NOTE: this could be optimized if we iterate only once through both input buffers (left and right)
			CMonoBuffer<float> jitterOutputBuffer(outputBuffer.left.size());
			ProcessJitter(jitterDelayBuffers.left, lowBuffer, leftNoiseBuffer, jitterOutputBuffer);

			// 5. Delay high frequencies
			CMonoBuffer<float> delayedHighBuffer(highBuffer.size());			
			highFrequencyDelayBuffers.left.Process(highBuffer, delayedHighBuffer);						

			// 6. Process post frequency-split filters 
			CMonoBuffer<float> postJitterLowBuffer(jitterOutputBuffer.size());
			CMonoBuffer<float> postJitterHighBuffer(jitterOutputBuffer.size());
			postLPFFilter.left.Process(jitterOutputBuffer, postJitterLowBuffer);
			postHPFFilter.left.Process(delayedHighBuffer, postJitterHighBuffer);
			outputBuffer.left = postJitterLowBuffer + postJitterHighBuffer;
		}
		else // LEFT bypass
		{			
			// 1. Split frequencies
			CMonoBuffer<float> lowBuffer(outputBuffer.left.size());
			CMonoBuffer<float> highBuffer(outputBuffer.left.size());
			bypassPreLPFFilter.left.Process(inputBuffer.left, lowBuffer);
			bypassPreHPFFilter.left.Process(inputBuffer.left, highBuffer);
			
			// 2. Add delay
			CMonoBuffer<float> delayedLowBuffer (lowBuffer.size());
			CMonoBuffer<float> delayedHighBuffer (highBuffer.size());
			bypassLowDelays.left.Process(lowBuffer, delayedLowBuffer);
			bypassHighDelays.left.Process(highBuffer, delayedHighBuffer);

			// 3. Process post frequency-split filters 
			CMonoBuffer<float> postJitterLowBuffer(lowBuffer.size());
			CMonoBuffer<float> postJitterHighBuffer(highBuffer.size());
			bypassPostLPFFilter.left.Process(delayedLowBuffer, postJitterLowBuffer);
			bypassPostHPFFilter.left.Process(delayedHighBuffer, postJitterHighBuffer);
			outputBuffer.left = postJitterLowBuffer + postJitterHighBuffer;
		}

		// Process right ear
		if (doTemporalDistortionSimulator.right)
		{
			// 1. Split frequencies
			CMonoBuffer<float> lowBuffer(outputBuffer.right.size());
			CMonoBuffer<float> highBuffer(outputBuffer.right.size());
			preLPFFilter.right.Process(inputBuffer.right, lowBuffer);
			preHPFFilter.right.Process(inputBuffer.right, highBuffer);			

			//// 2. Do a dummy process of the bypass delay buffers, to minimize clicks when enabling/disabling one ear
			//// NOTE: this should be done with the bypass filters...
			//CMonoBuffer<float> dummyBuffer(lowBuffer.size());
			//bypassLowDelays.left.Process(lowBuffer, dummyBuffer);
			//bypassHighDelays.left.Process(highBuffer, dummyBuffer);

			// 3. Compute autocorrelation coefficients for noise sources
			power.right = rightNoiseBuffer.GetPower();
			autocorrelation.right = rightNoiseBuffer.GetAutocorrelation(autocorrelationShift_samples);

			// 4. Process jitter
			// NOTE: this could be optimized if we iterate only once through both input buffers (left and right)
			CMonoBuffer<float> jitterOutputBuffer(outputBuffer.right.size());
			ProcessJitter(jitterDelayBuffers.right, lowBuffer, rightNoiseBuffer, jitterOutputBuffer);

			// 5. Recompose signals, mixing (jittered) low frequencies with (delayed) high frequencies			
			CMonoBuffer<float> delayedHighBuffer(highBuffer.size());			
			highFrequencyDelayBuffers.right.Process(highBuffer, delayedHighBuffer);			
			//outputBuffer.right = rightDelayedHighBuffer + rightJitterOutputBuffer;

			// 5. Process post frequency-split filters
			CMonoBuffer<float> postJitterLowBuffer(jitterOutputBuffer.size());
			CMonoBuffer<float> postJitterHighBuffer(jitterOutputBuffer.size());
			postLPFFilter.right.Process(jitterOutputBuffer, postJitterLowBuffer);
			postHPFFilter.right.Process(delayedHighBuffer, postJitterHighBuffer);
			outputBuffer.right = postJitterLowBuffer + postJitterHighBuffer;
		}
		else // RIGHT bypass
		{
			// 1. Split frequencies
			CMonoBuffer<float> lowBuffer(outputBuffer.right.size());
			CMonoBuffer<float> highBuffer(outputBuffer.right.size());
			bypassPreLPFFilter.right.Process(inputBuffer.right, lowBuffer);
			bypassPreHPFFilter.right.Process(inputBuffer.right, highBuffer);

			// 2. Add delay
			CMonoBuffer<float> delayedLowBuffer(lowBuffer.size());
			CMonoBuffer<float> delayedHighBuffer(highBuffer.size());
			bypassLowDelays.right.Process(lowBuffer, delayedLowBuffer);
			bypassHighDelays.right.Process(highBuffer, delayedHighBuffer);

			// 3. Process post frequency-split filters 
			CMonoBuffer<float> postJitterLowBuffer(lowBuffer.size());
			CMonoBuffer<float> postJitterHighBuffer(highBuffer.size());
			bypassPostLPFFilter.right.Process(delayedLowBuffer, postJitterLowBuffer);
			bypassPostHPFFilter.right.Process(delayedHighBuffer, postJitterHighBuffer);
			outputBuffer.right = postJitterLowBuffer + postJitterHighBuffer;
		}
	}

	///////////////////////////////////////////////////////////
	// Set the amount of left-right noise synchronicity
	void CTemporalDistortionSimulator::SetLeftRightNoiseSynchronicity(float leftRightSynchronicity)
	{
		leftRightNoiseSynchronicity = leftRightSynchronicity;
	}

	///////////////////////////////////////////////////////////
	// Set the amount of temporal Distortion
	void CTemporalDistortionSimulator::SetWhiteNoisePower(Common::T_ear ear, float noisePower)
	{
		if ((ear == Common::T_ear::BOTH) && (!doLeftRightNoiseSynchronicity))
		{
			SetWhiteNoisePower(Common::T_ear::LEFT, noisePower);
			SetWhiteNoisePower(Common::T_ear::RIGHT, noisePower);
		}
		else
		{
			// Compute noise standard deviation as number of samples from time in ms
			float noiseDeviation = ((float)sampleRate * noisePower) / 1000.0;

			if ((ear == Common::T_ear::LEFT) || (doLeftRightNoiseSynchronicity))
			{
				noiseGenerators.left.SetDeviation(noiseDeviation);
				whiteNoisePowerLastLeftValue = noisePower;
			}
			if ((ear == Common::T_ear::RIGHT) || (doLeftRightNoiseSynchronicity))
				noiseGenerators.right.SetDeviation(noiseDeviation);
		}
	}

	///////////////////////////////////////////////////////////
	// Set the cutoff frequency of jitter noise source autocorrelation filter
	void CTemporalDistortionSimulator::SetNoiseAutocorrelationFilterCutoffFrequency(Common::T_ear ear, float cutoffFrequency)
	{
		if ((ear == Common::T_ear::BOTH) && (!doLeftRightNoiseSynchronicity))
		{
			SetNoiseAutocorrelationFilterCutoffFrequency(Common::T_ear::LEFT, cutoffFrequency);
			SetNoiseAutocorrelationFilterCutoffFrequency(Common::T_ear::RIGHT, cutoffFrequency);
		}
		if ((ear == Common::T_ear::LEFT) || (doLeftRightNoiseSynchronicity))
		{
			noiseGenerators.left.SetAutocorrelationFilterCutoff(cutoffFrequency);
			noiseAutocorrelationFilterCutoffFrequencyLastLeftValue = cutoffFrequency;
		}
		if ((ear == Common::T_ear::RIGHT) || (doLeftRightNoiseSynchronicity))
			noiseGenerators.right.SetAutocorrelationFilterCutoff(cutoffFrequency);
	}

	///////////////////////////////////////////////////////////
	// Set the upper limit for the (low frequency) band where jitter occurs
	void CTemporalDistortionSimulator::SetBandUpperLimit(Common::T_ear ear, int upperLimit)
	{
		if ((ear == Common::T_ear::BOTH) && (!doLeftRightNoiseSynchronicity))
		{
			SetBandUpperLimit(Common::T_ear::LEFT, upperLimit);
			SetBandUpperLimit(Common::T_ear::RIGHT, upperLimit);
		}
		if ((ear == Common::T_ear::LEFT) || (doLeftRightNoiseSynchronicity))
		{
			preLPFFilter.left.SetFilterCoefficients(sampleRate, upperLimit);
			preHPFFilter.left.SetFilterCoefficients(sampleRate, upperLimit);
			postLPFFilter.left.SetFilterCoefficients(sampleRate, upperLimit);
			postHPFFilter.left.SetFilterCoefficients(sampleRate, upperLimit);

			bypassPreLPFFilter.right.SetFilterCoefficients(sampleRate, upperLimit);
			bypassPreHPFFilter.right.SetFilterCoefficients(sampleRate, upperLimit);
			bypassPostLPFFilter.right.SetFilterCoefficients(sampleRate, upperLimit);
			bypassPostHPFFilter.right.SetFilterCoefficients(sampleRate, upperLimit);

			bandUpperLimitLastLeftValue = upperLimit;
		}
		if ((ear == Common::T_ear::RIGHT) || (doLeftRightNoiseSynchronicity))
		{
			preLPFFilter.right.SetFilterCoefficients(sampleRate, upperLimit);
			preHPFFilter.right.SetFilterCoefficients(sampleRate, upperLimit);
			postLPFFilter.right.SetFilterCoefficients(sampleRate, upperLimit);
			postHPFFilter.right.SetFilterCoefficients(sampleRate, upperLimit);

			bypassPreLPFFilter.left.SetFilterCoefficients(sampleRate, upperLimit);
			bypassPreHPFFilter.left.SetFilterCoefficients(sampleRate, upperLimit);
			bypassPostLPFFilter.left.SetFilterCoefficients(sampleRate, upperLimit);
			bypassPostHPFFilter.left.SetFilterCoefficients(sampleRate, upperLimit);
		}
	}

	///////////////////////////////////////////////////////////
	// Internal jitter process...
	void CTemporalDistortionSimulator::ProcessJitter(Common::CDelay& delayBuffer, CMonoBuffer<float> & inputBuffer, CMonoBuffer<float> & noiseSource, CMonoBuffer<float> & outputBuffer)
	{
		// TO DO: check errors...

		// Update delay buffer
		CMonoBuffer<float> delayedInputBuffer(jitterDelayBufferSize);
		delayBuffer.Process(inputBuffer, delayedInputBuffer);

		// Compute output samples

		for (int i = 0; i < outputBuffer.size(); i++)
		{
			// Truncate noise samples to fit in delay buffer size
			float noiseSample = noiseSource[i];
			if (noiseSample <= -maxSampleOffset)
				noiseSample = -maxSampleOffset + 1;
			if (noiseSample >= maxSampleOffset)
				noiseSample = maxSampleOffset - 1;

			// Add sample offset to get sample from delayed input buffer
			float newSample = delayedInputBuffer[i + maxSampleOffset + noiseSample];
			outputBuffer[i] = newSample;
		}
	}

	///////////////////////////////////////////////////////////
	// Get autocorrelation coefficient zero for jitter noise source
	float CTemporalDistortionSimulator::GetPower(Common::T_ear ear)
	{
		if (ear == Common::T_ear::LEFT)
			return GetTimeInMilisecondsFromNumberOfSamples(power.left);
		if (ear == Common::T_ear::RIGHT)
			return GetTimeInMilisecondsFromNumberOfSamples(power.right);

		SET_RESULT(RESULT_ERROR_CASENOTDEFINED, "Autocorrelation coefficients exist only for LEFT and RIGHT ear (not BOTH or NONE)");
		return -1.0f;
	}

	///////////////////////////////////////////////////////////
	// Get autocorrelation coefficient one for jitter noise source
	float CTemporalDistortionSimulator::GetNormalizedAutocorrelation(Common::T_ear ear)
	{
		if (ear == Common::T_ear::LEFT)
			return power.left < 0.0001 ? 0 : autocorrelation.left / power.left;
		if (ear == Common::T_ear::RIGHT)
			return power.right < 0.0001 ? 0 : autocorrelation.right / power.right;

		SET_RESULT(RESULT_ERROR_CASENOTDEFINED, "Autocorrelation coefficients exist only for LEFT and RIGHT ear (not BOTH or NONE)");
		return -1.0f;
	}

	///////////////////////////////////////////////////////////
	// Enable the possibility of changing the synchronicity between the two ears
	void CTemporalDistortionSimulator::EnableLeftRightNoiseSynchronicity()
	{
		doLeftRightNoiseSynchronicity = true;

		// Copy values of internal attributes from left to right
		SetBandUpperLimit(Common::T_ear::RIGHT, bandUpperLimitLastLeftValue);
		SetNoiseAutocorrelationFilterCutoffFrequency(Common::T_ear::RIGHT, noiseAutocorrelationFilterCutoffFrequencyLastLeftValue);
		SetWhiteNoisePower(Common::T_ear::RIGHT, whiteNoisePowerLastLeftValue);
	}

	///////////////////////////////////////////////////////////
	// Disable the possibility of changing the synchronicity between the two ears
	void CTemporalDistortionSimulator::DisableLeftRightNoiseSynchronicity()
	{
		doLeftRightNoiseSynchronicity = false;
	}

	///////////////////////////////////////////////////////////
	// Transform a value in number of samples into a time in miliseconds
	float CTemporalDistortionSimulator::GetTimeInMilisecondsFromNumberOfSamples(float numberOfSamples)
	{
		return (1000.0f * numberOfSamples) / (float)sampleRate;
	}

	///////////////////////////////////////////////////////////
	// Transform a time in miliseconds into number of samples 
	float CTemporalDistortionSimulator::GetNumberOfSamplesFromTimeInMiliseconds(float time_ms)
	{
		return (sampleRate * time_ms) / (float)1000.0f;
	}

	///////////////////////////////////////////////////////////
	// Enable temporal Distortion simulator for one or both ears
	void CTemporalDistortionSimulator::EnableTemporalDistortionSimulator(Common::T_ear ear)
	{
		if (ear == Common::T_ear::BOTH) 
		{
			EnableTemporalDistortionSimulator(Common::T_ear::LEFT);
			EnableTemporalDistortionSimulator(Common::T_ear::RIGHT);
			return;
		}
		if (ear == Common::T_ear::LEFT) 
			doTemporalDistortionSimulator.left = true;
		if (ear == Common::T_ear::RIGHT)
			doTemporalDistortionSimulator.right = true;
	}

	///////////////////////////////////////////////////////////
	// Disable temporal Distortion simulator for one or both ears
	void CTemporalDistortionSimulator::DisableTemporalDistortionSimulator(Common::T_ear ear)
	{
		if (ear == Common::T_ear::BOTH)
		{
			DisableTemporalDistortionSimulator(Common::T_ear::LEFT);
			DisableTemporalDistortionSimulator(Common::T_ear::RIGHT);
			return;
		}
		if (ear == Common::T_ear::LEFT)
		{
			doTemporalDistortionSimulator.left = false;
			//bypassDelays.left.Reset();
		}
		if (ear == Common::T_ear::RIGHT)
		{
			doTemporalDistortionSimulator.right = false;
			//bypassDelays.right.Reset();
		}
	}

}// end namespace HAHLSimulation
