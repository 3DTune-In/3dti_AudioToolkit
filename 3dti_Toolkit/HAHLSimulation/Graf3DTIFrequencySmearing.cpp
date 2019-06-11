/**
* \class CGraf3DTIFrequencySmearing
*
* \brief Definition of CGraf3DTIFrequencySmearing class.
*
* This class implements the necessary algorithms to do the 3DTI frequency smearing approach for HL simulation.
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
#include <HAHLSimulation/Graf3DTIFrequencySmearing.h>
#include <Common/ErrorHandler.h>
#include <cmath>


// 1 / (sqrt(2*PI))
#ifndef INVERSE_SQRT_2PI
#define INVERSE_SQRT_2PI 0.39894228f
#endif

namespace HAHLSimulation {
	/////////////////////////////
	// CONSTRUCTOR/DESTRUCTOR  //
	/////////////////////////////
	CGraf3DTIFrequencySmearing::CGraf3DTIFrequencySmearing() : bufferSize{ 0 }, setupDone{ false }
	{
	}

	///////////////////
	// Public Methods //
	///////////////////

	//Initialize the class and allocate memory.
	void CGraf3DTIFrequencySmearing::Setup(int _bufferSize, float _samplingRate)
	{
		ASSERT(_bufferSize > 0, RESULT_ERROR_BADSIZE, "Bad buffer size when setting up frequency smearing", "");

		if (_bufferSize > 0)		//In case the error handler is off
		{

			bufferSize = _bufferSize;					//Store the new buffer size
			samplingRate = _samplingRate;				//Store the new sampling rate
			oneSampleBandwidth = (samplingRate / ((float)bufferSize * 4.0f));

			InitializePreviousBuffers();

			downwardSmearingBufferSize = DEFAULT_SMEARING_SECTION_SIZE;
			upwardSmearingBufferSize = DEFAULT_SMEARING_SECTION_SIZE;
			downwardSmearing_Hz = DEFAULT_SMEARING_HZ;
			upwardSmearing_Hz = DEFAULT_SMEARING_HZ;

			SmearingFunctionSetup();

			setupDone = true;
			SET_RESULT(RESULT_OK, "Smearing frequency succesfully set");
		}
	}//Setup

	void CGraf3DTIFrequencySmearing::Process(const CMonoBuffer<float>& inputBuffer, CMonoBuffer<float>& outputBuffer)
	{
		ASSERT(inputBuffer.size() == bufferSize, RESULT_ERROR_BADSIZE, "Bad input size when setting up frequency smearing", "");

		if (setupDone && (inputBuffer.size() == bufferSize)) //In case the error handler is off
		{
			//Merge current buffer and last buffer
			CMonoBuffer<float> longInputBuffer;
			longInputBuffer.reserve(previousBuffer.size() + inputBuffer.size());						// preallocate memory
			longInputBuffer.insert(longInputBuffer.end(), previousBuffer.begin(), previousBuffer.end());
			longInputBuffer.insert(longInputBuffer.end(), inputBuffer.begin(), inputBuffer.end());

			//Enveloped using Hann Window
			CMonoBuffer<float> longInputBufferWindowed;
			ProcessHannWindow(longInputBuffer, longInputBufferWindowed);

			//Make FFT
			CMonoBuffer<float> longInputBufferWindowed_FFT;
			Common::CFprocessor::CalculateFFT(longInputBufferWindowed, longInputBufferWindowed_FFT);

			//Split in Module and Phase
			CMonoBuffer<float> moduleBuffer;
			CMonoBuffer<float> phaseBuffer;
			Common::CFprocessor::ProcessToModulePhase(longInputBufferWindowed_FFT, moduleBuffer, phaseBuffer);

			// SMEARING
			CMonoBuffer<float> smearingModuleBuffer;
			ProcessSmearing(moduleBuffer, smearingModuleBuffer);

			//Merge Module and Phase
			CMonoBuffer<float> longOutputBuffer_FFT;
			Common::CFprocessor::ProcessToRealImaginary(smearingModuleBuffer, phaseBuffer, longOutputBuffer_FFT);

			//IFFT
			CMonoBuffer<float> longOutputBuffer;
			Common::CFprocessor::CalculateIFFT(longOutputBuffer_FFT, longOutputBuffer);

			//Overlap ADD method to get the correct output buffer and update the storageBuffer
			ProcessOutputBuffer_OverlapAddMethod(longOutputBuffer, outputBuffer);

			//Update new buffer
			previousBuffer = inputBuffer;
		}
		else
		{
			outputBuffer.clear();
			outputBuffer.insert(outputBuffer.end(), bufferSize, 0.0f);
		}
	}

	/////////////////////
	// Private methods //
	/////////////////////

	void CGraf3DTIFrequencySmearing::SmearingFunctionSetup() {

		//Reserve space to store hann window 
		hannWindowBuffer.resize(bufferSize * 2);

		//Calculate Smearing Window
		CalculateSmearingWindow();

		//Calculate Hann window to this buffer size
		CalculateHannWindow();

	}

	void CGraf3DTIFrequencySmearing::CalculateHannWindow()
	{
		int hannWindowSize = hannWindowBuffer.size();
		for (int i = 0; i< hannWindowSize; i++) {
			double temp = (2.0 * M_PI * (double)i) / ((double)hannWindowSize - 1.0);
			hannWindowBuffer[i] = CalculateRoundToZero(0.5 * (1.0 - std::cos(temp)));
		}
	}

	void CGraf3DTIFrequencySmearing::ProcessHannWindow(const CMonoBuffer<float>& inputBuffer, CMonoBuffer<float>& outputBuffer)
	{
		ASSERT(inputBuffer.size() == hannWindowBuffer.size(), RESULT_ERROR_BADSIZE, "The input buffer size has to be equal to hann window buffer size", "");

		if (inputBuffer.size() == hannWindowBuffer.size())		//Just in case the error handler is off
		{
			outputBuffer.resize(inputBuffer.size());	//resize output buffer
														//Process hann window
			for (int i = 0; i< inputBuffer.size(); i++) {
				outputBuffer[i] = inputBuffer[i] * hannWindowBuffer[i];
			}
		}
	}

	void CGraf3DTIFrequencySmearing::ProcessSmearing(const CMonoBuffer<float>& inputBuffer, CMonoBuffer<float>& outputBuffer)
	{
		// 1. Get left section [0, PI] of symmetric input buffer for convolution
		// Note: if input buffer size is even, this might have undefined behavior
		CMonoBuffer<float> leftInputBuffer(inputBuffer.begin(), inputBuffer.begin() + inputBuffer.size() / 2 + 1);	// Get left side		

		// 2. Process convolution between smearing window or matrix and left section of input
		outputBuffer.clear();
		outputBuffer.assign(leftInputBuffer.size(), 0.0f);
		
		ProcessSmearingConvolution(leftInputBuffer, outputBuffer);

		// 3. Copy inverted convolved buffer to right side of symmetric output (except for last sample)		
		//for (int i = 0; i < leftInputBuffer.size() - 1; i++)
		for (int i = 1; i < leftInputBuffer.size() - 1; i++)
		{
			outputBuffer.push_back(outputBuffer[leftInputBuffer.size() - i - 1]);
		}
	
	}

	void CGraf3DTIFrequencySmearing::ProcessOutputBuffer_OverlapAddMethod(std::vector<float>& input_ConvResultBuffer, std::vector<float>& outBuffer)
	{
		if (outBuffer.size() < bufferSize) { outBuffer.resize(bufferSize); }	//Prepare the outbuffer		
																				//Check buffer sizes	
		ASSERT(outBuffer.size() == bufferSize, RESULT_ERROR_BADSIZE, "OutBuffer size has to be zero or equal to the input size indicated by the setup method", "");

		if (outBuffer.size() == bufferSize) //Just in case error handler is off
		{
			int outBufferSize = bufferSize;	//Locar var to move throught the outbuffer

											//Fill out the output signal buffer
			for (int i = 0; i < outBufferSize; i++)
			{
				if (i < storageBuffer.size()) {
					outBuffer[i] = static_cast<float>(storageBuffer[i] + CalculateRoundToZero(input_ConvResultBuffer[i]));
				}
				else
				{
					outBuffer[i] = static_cast<float>(CalculateRoundToZero(input_ConvResultBuffer[i]));
				}
			}
			//Fill out the storage buffer to be used in the next call
			std::vector<float> temp;
			temp.reserve(input_ConvResultBuffer.size() - outBufferSize);
			int inputConvResult_size = input_ConvResultBuffer.size();	//Locar var to move to the end of the input_ConvResultBuffer
			for (int i = outBufferSize; i < inputConvResult_size; i++)
			{
				if (i<storageBuffer.size())
				{
					temp.push_back(storageBuffer[i] + CalculateRoundToZero(input_ConvResultBuffer[i]));
				}
				else
				{
					temp.push_back(CalculateRoundToZero(input_ConvResultBuffer[i]));
				}
			}
			storageBuffer.swap(temp);				//To use in C++03
													//storageBuffer = std::move(temp);			//To use in C++11
		}
	}

	double CGraf3DTIFrequencySmearing::CalculateRoundToZero(double number)
	{
		if (std::abs(number) < FSMEARING_THRESHOLD) { return 0.0f; }
		else { return number; }
	}


	void CGraf3DTIFrequencySmearing::CalculateSmearingWindow()
	{
		//// Compute left and right buffer size 
		//int leftSize = leftSmearingWindowBandwidth / oneSampleBandwidth;
		//int rightSize = rightSmearingWindowBandwidth / oneSampleBandwidth;				

		// Init values
		float totalArea = 0.0f;
		float valueAtMean = 0.0f;

		// Special case: both smearing amounts are 0 -> single impulse
		if (IsCloseToZero(downwardSmearing_Hz) && IsCloseToZero(upwardSmearing_Hz))
		{
			smearingWindow.clear();
			smearingWindow.assign(downwardSmearingBufferSize + upwardSmearingBufferSize, 0.0f);
			smearingWindow[downwardSmearingBufferSize] = 1.0f;	// This is the impulse
			return;
		}

		// Build downward section
		smearingWindow.clear();
		for (int i = downwardSmearingBufferSize - 1; i >= 0; i--)
		{
			float scaledValue = (float)i * oneSampleBandwidth;	// Value in Hz
			float gaussianValue = CalculateGaussianProbability(0.0f, downwardSmearing_Hz, scaledValue);
			smearingWindow.push_back(gaussianValue);
			totalArea += gaussianValue; // Add new value to total area									
		}
		valueAtMean = smearingWindow[downwardSmearingBufferSize - 1];

		// Build upward section
		CMonoBuffer<float> upwardSmearingWindow(upwardSmearingBufferSize);
		upwardSmearingWindow.clear();
		float upwardArea = 0.0f;
		for (int i = 0; i < upwardSmearingBufferSize; i++)
		{
			float scaledValue = (float)i * oneSampleBandwidth;	// Value in Hz
			float gaussianValue = CalculateGaussianProbability(0.0f, upwardSmearing_Hz, scaledValue);
			upwardSmearingWindow.push_back(gaussianValue);
			upwardArea += gaussianValue; // Add new value to total area						
		}

		// Normalize upward section wrt value at mean for left side
		upwardArea *= valueAtMean / upwardSmearingWindow[0];
		totalArea += upwardArea;
		upwardSmearingWindow.ApplyGain(valueAtMean / upwardSmearingWindow[0]);

		// Concatenate downward and upward buffers and normalize wrt total area		
		smearingWindow.insert(smearingWindow.end(), upwardSmearingWindow.begin(), upwardSmearingWindow.end());
		smearingWindow.ApplyGain(1.0f / totalArea);
	}

	float CGraf3DTIFrequencySmearing::CalculateGaussianProbability(float mean, float deviation, float value)
	{
		if (IsCloseToZero(deviation))
		{
			if (value == 0.0f)
				return 1.0f;
			else
				return 0.0f;
		}

		float standarizedValue = ((float)value - mean) / deviation;	// Value in Hz, standarized to gaussian distribution
		return deviation * INVERSE_SQRT_2PI * std::exp(-0.5f * standarizedValue * standarizedValue); // Gaussian probability
	}

	void CGraf3DTIFrequencySmearing::SetDownwardSmearingBufferSize(int downwardSize)
	{
		ASSERT(downwardSize > 0, RESULT_ERROR_OUTOFRANGE, "Smearing window size must be a positive value", "");
		downwardSmearingBufferSize = downwardSize;
		setupDone = false;
		SmearingFunctionSetup();
		setupDone = true;
	}

	void CGraf3DTIFrequencySmearing::SetUpwardSmearingBufferSize(int upwardSize)
	{
		ASSERT(upwardSize > 0, RESULT_ERROR_OUTOFRANGE, "Smearing window size must be a positive value", "");
		upwardSmearingBufferSize = upwardSize;
		setupDone = false;
		SmearingFunctionSetup();
		setupDone = true;
	}

	void CGraf3DTIFrequencySmearing::SetDownwardSmearing_Hz(float downwardSmearing)
	{
		ASSERT(downwardSmearing >= 0.0f, RESULT_ERROR_OUTOFRANGE, "Smearing amount must be a positive (or zero) value in Hz", "");
		downwardSmearing_Hz = downwardSmearing;
		setupDone = false;
		SmearingFunctionSetup();
		setupDone = true;
	}

	void CGraf3DTIFrequencySmearing::SetUpwardSmearing_Hz(float upwardSmearing)
	{
		ASSERT(upwardSmearing >= 0.0f, RESULT_ERROR_OUTOFRANGE, "Smearing amount must be a positive (or zero) value in Hz", "");
		upwardSmearing_Hz = upwardSmearing;
		setupDone = false;
		SmearingFunctionSetup();
		setupDone = true;
	}

	CMonoBuffer<float>* CGraf3DTIFrequencySmearing::GetSmearingWindow()
	{
		return &smearingWindow;
	}

	bool CGraf3DTIFrequencySmearing::IsCloseToZero(float value)
	{
		if (std::abs(value) < FSMEARING_THRESHOLD)
			return true;
		else
			return false;
	}

	void CGraf3DTIFrequencySmearing::InitializePreviousBuffers()
	{
		
			previousBuffer.clear();
			storageBuffer.clear();
			hannWindowBuffer.clear();
			smearingWindow.clear();
		

		previousBuffer.insert(previousBuffer.end(), bufferSize, 0.0f);
		storageBuffer.insert(storageBuffer.end(), bufferSize, 0.0f);
		hannWindowBuffer.insert(hannWindowBuffer.end(), bufferSize * 2, 0.0f);
	}

	void CGraf3DTIFrequencySmearing::ProcessSmearingConvolution(CMonoBuffer<float> &inputBuffer, CMonoBuffer<float> &outputBuffer)
	{
		ASSERT(inputBuffer.size() == outputBuffer.size(), RESULT_ERROR_BADSIZE, "Smearing convolution process requires output buffer to be of the same size of input source signal", "");

		for (int n = 0; n < inputBuffer.size(); n++)
		{
			outputBuffer[n] = 0.0f;
			for (int m = 0; m < inputBuffer.size(); m++)
			{
				int smearingWindowIndex = n - m + downwardSmearingBufferSize;
				if ((smearingWindowIndex >= 0) && (smearingWindowIndex < smearingWindow.size()))
					outputBuffer[n] += inputBuffer[m] * smearingWindow[smearingWindowIndex];
			}
		}
	}

}//end namespace