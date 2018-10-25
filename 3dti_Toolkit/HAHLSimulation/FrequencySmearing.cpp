/**
* \class CFrequencySmearing
*
* \brief Definition of CFrequencySmearing class.
*
* This class implements the necessary algorithms to do the frequency smearing for HL simulation.
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
#include <HAHLSimulation/FrequencySmearing.h>
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
	CFrequencySmearing::CFrequencySmearing() : bufferSize{ 0 }, setupDone{ false }
	{		
	}

	///////////////////
	// Public Methods //
	///////////////////

	//Initialize the class and allocate memory.
	void CFrequencySmearing::Setup(int _bufferSize, float _samplingRate, SmearingAlgorithm _smearingAlgorithm)
	{
		ASSERT(_bufferSize > 0, RESULT_ERROR_BADSIZE, "Bad buffer size when setting up frequency smearing", "");
		
		if (_bufferSize > 0)		//In case the error handler is off
		{

			bufferSize = _bufferSize;					//Store the new buffer size
			samplingRate = _samplingRate;				//Store the new sampling rate
			oneSampleBandwidth = (samplingRate / ((float)bufferSize * 4.0f));
			smearingAlgorithm = _smearingAlgorithm;

			InitializePreviousBuffers();

			downwardSmearingBufferSize = DEFAULT_SMEARING_SECTION_SIZE;
			upwardSmearingBufferSize = DEFAULT_SMEARING_SECTION_SIZE;			
			downwardSmearing_Hz = DEFAULT_SMEARING_HZ;
			upwardSmearing_Hz = DEFAULT_SMEARING_HZ;			

			SmearingWindowSetup();

			setupDone = true;
			SET_RESULT(RESULT_OK, "Smearing frequency succesfully set");
		}				
	}//Setup

	CFrequencySmearing::SmearingAlgorithm CFrequencySmearing::GetSmearingAlgorithm()
	{
		return smearingAlgorithm;
	}

	void CFrequencySmearing::SetSmearingAlgorithm(SmearingAlgorithm _smearingAlgorithm)
	{
		smearingAlgorithm = _smearingAlgorithm;
		InitializePreviousBuffers();
		SmearingWindowSetup();		
	}

	void CFrequencySmearing::Process(const CMonoBuffer<float>& inputBuffer, CMonoBuffer<float>& outputBuffer)
	{
		ASSERT(setupDone, RESULT_ERROR_NOTINITIALIZED, "Setup method should be called before calling Process in Frequency Smearing", "");
		ASSERT(inputBuffer.size() == bufferSize, RESULT_ERROR_BADSIZE, "Bad input size when setting up frequency smearing", "");
		
		if (setupDone && (inputBuffer.size() == bufferSize)) //In case the error handler is off
		{ 			
			switch (smearingAlgorithm) {
			case SmearingAlgorithm::CLASSIC:
				ProcessClassic(inputBuffer, outputBuffer);
				break;
			case SmearingAlgorithm::SUBFRAME:
				ProcessSubframe(inputBuffer, outputBuffer);
				break;
			default:
				break;
			}
		}						
	}
	
	 /////////////////////
	// Private methods //
	/////////////////////

	void CFrequencySmearing::ProcessSubframe(const CMonoBuffer<float>& inputBuffer, CMonoBuffer<float>& outputBuffer)
	{
		//Merge current buffer and last buffer
		CMonoBuffer<float> longInputBuffer;
		longInputBuffer.reserve(previousBuffer.size() + inputBuffer.size());						// preallocate memory
		longInputBuffer.insert(longInputBuffer.end(), previousBuffer.begin(), previousBuffer.end());
		longInputBuffer.insert(longInputBuffer.end(), inputBuffer.begin(), inputBuffer.end());

		CMonoBuffer<float> inputBufferWindow;
		inputBufferWindow.reserve(bufferSize);

		CMonoBuffer<float> storagePrevBuffer[4];
		int shift = bufferSize / 4;

		for (int i = 0; i < 4; i++) {

			//Reserve memory for storaging processed buffer
			storagePrevBuffer[i].reserve(bufferSize);

			//
			inputBufferWindow.clear(); inputBufferWindow.reserve(bufferSize);
			inputBufferWindow.insert(inputBufferWindow.end(),
									 longInputBuffer.begin() + i * shift,
									 longInputBuffer.begin() + i * shift + bufferSize);

			//Enveloped using Hann Window
			CMonoBuffer<float> inputBufferCrossfaded;
			ProcessHannWindow(inputBufferWindow, inputBufferCrossfaded);

			//Make FFT 
			CMonoBuffer<float> inputBufferCrossfaded_FFT;
			Common::CFprocessor::CalculateFFT(inputBufferCrossfaded, inputBufferCrossfaded_FFT);

			//Split in Module and Phase
			CMonoBuffer<float> moduleBufferCrossfaded;
			CMonoBuffer<float> phaseBufferCrossfaded;
			Common::CFprocessor::ProcessToModulePhase(inputBufferCrossfaded_FFT, moduleBufferCrossfaded, phaseBufferCrossfaded);

			// SMEARING
			CMonoBuffer<float> smearedModuleBufferCrossfaded;
			ProcessSmearing(moduleBufferCrossfaded, smearedModuleBufferCrossfaded);

			//Merge Module and Phase
			CMonoBuffer<float> outputBufferCrossfaded_FFT;
			Common::CFprocessor::ProcessToRealImaginary(smearedModuleBufferCrossfaded, phaseBufferCrossfaded, outputBufferCrossfaded_FFT);

			//IFFT
			CMonoBuffer<float> outputBufferCrossfaded;
			Common::CFprocessor::CalculateIFFT(outputBufferCrossfaded_FFT, outputBufferCrossfaded);

			//Enveloped again using Hann Window
			CMonoBuffer<float> outputBufferDoubleCrossfaded;
			ProcessHannWindow(outputBufferCrossfaded, outputBufferDoubleCrossfaded);

			//Save end result
			storagePrevBuffer[i].insert(storagePrevBuffer[i].end(), outputBufferDoubleCrossfaded.begin(), outputBufferDoubleCrossfaded.end());
		}

		// Output buffer sums all contributions for each quarter
		
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < shift; j++) {
				switch (i) {
				case 0: // First quarter
					outputBuffer[j] =	storageLastBuffer[0][shift * 3 + j] +
										storageLastBuffer[1][shift * 2 + j] +
										storageLastBuffer[2][shift + j] +
										storagePrevBuffer[0][j];
					break;
				case 1: // Second quarter
					outputBuffer[shift + j] =	storageLastBuffer[1][shift * 3 + j] +
												storageLastBuffer[2][shift * 2 + j] +
												storagePrevBuffer[0][shift + j] +
												storagePrevBuffer[1][j];
					break;
				case 2: // Third quarter
					outputBuffer[shift * 2 + j] =	storageLastBuffer[2][shift * 3 + j] +
													storagePrevBuffer[0][shift * 2 + j] +
													storagePrevBuffer[1][shift + j] +
													storagePrevBuffer[2][j];
					break;
				case 3: // Fourth quarter
					outputBuffer[shift * 3 + j] =	storagePrevBuffer[0][shift * 3 + j] +
													storagePrevBuffer[1][shift * 2 + j] +
													storagePrevBuffer[2][shift + j] +
													storagePrevBuffer[3][j];
					break;
				}
			}
		}

		//Update new buffer
		previousBuffer.clear();
		previousBuffer.insert(previousBuffer.end(), inputBuffer.begin(), inputBuffer.end());

		//Move storaged processed previous buffer to storaged last buffer
		for (int i = 0; i < 3; i++) {
			storageLastBuffer[i].clear();
			storageLastBuffer[i].insert(storageLastBuffer[i].end(), storagePrevBuffer[i + 1].begin(), storagePrevBuffer[i + 1].end());
		}
	}

	void CFrequencySmearing::ProcessClassic(const CMonoBuffer<float>& inputBuffer, CMonoBuffer<float>& outputBuffer)
	{

		ASSERT(setupDone, RESULT_ERROR_NOTINITIALIZED, "Setup method should be called before calling Process in Frequency Smearing", "");
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
	}

	void CFrequencySmearing::SmearingWindowSetup() {
		
		if (smearingAlgorithm == SmearingAlgorithm::CLASSIC) {
			CalculateSmearingWindow();					//Calculate smearing window with actual parameters			
			hannWindowBuffer.resize(bufferSize * 2);		//Reserve space to store hann window depending on algorithm		
		}
		else {
			smearingMatrix.resize(bufferSize);
			for (int i = 0; i < bufferSize; i++) smearingMatrix[i].resize(bufferSize);
			CalculateSmearingMatrix();
			hannWindowBuffer.resize(bufferSize);		//Reserve space to store hann window depending on algorithm		
		}
		CalculateHannWindow();						//Calculate hann window to this buffer size
	}

	void CFrequencySmearing::CalculateHannWindow()
	{
		float powerFactor = smearingAlgorithm == CFrequencySmearing::SmearingAlgorithm::SUBFRAME ? (1/sqrt(1.5)) : 1;
		int hannWindowSize = hannWindowBuffer.size();
		for (int i = 0; i< hannWindowSize; i++) {
			float temp = (2 * M_PI * i) / (hannWindowSize - 1);
			hannWindowBuffer[i] = CalculateRoundToZero(0.5f * (1 - std::cos(temp))) * powerFactor;
		}
	}

	void CFrequencySmearing::ProcessHannWindow(const CMonoBuffer<float>& inputBuffer, CMonoBuffer<float>& outputBuffer) 
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
	
	void CFrequencySmearing::ProcessSmearing(const CMonoBuffer<float>& inputBuffer, CMonoBuffer<float>& outputBuffer)
	{		
		// TO DO: ASSERT for inputBuffer.size == outputBuffer.size ??

		// TEST
		//CMonoBuffer<float> leftInputBuffer(512);
		//// TONE:
		//leftInputBuffer.Fill(512, 0.0f);
		//leftInputBuffer[64] = 1.0f;
		////// DOWNWARD RAMP:
		////leftInputBuffer.SetFromRamp(false);
		//SetDownwardSmearing_Hz(100.0f);
		//SetUpwardSmearing_Hz(500.0f);
		//SetDownwardSmearingBufferSize(100);
		//SetUpwardSmearingBufferSize(100);
		////

		//// 1. Get right section [-PI, 0) of symmetric input buffer for convolution, and append value at [0]
		//// Note: if input buffer size is even, this might have undefined behavior
		//CMonoBuffer<float> rightInputBuffer(inputBuffer.begin() + inputBuffer.size() / 2, inputBuffer.end());	// Get right side
		//rightInputBuffer.push_back(inputBuffer[0]);	// Append value at frequency 0

		// 1. Get left section [0, PI] of symmetric input buffer for convolution
		// Note: if input buffer size is even, this might have undefined behavior
		CMonoBuffer<float> leftInputBuffer(inputBuffer.begin(), inputBuffer.begin() + inputBuffer.size()/2 + 1);	// Get left side		

		// 2. Process convolution between smearing window or matrix and left section of input
		outputBuffer.clear();
		outputBuffer.assign(leftInputBuffer.size(), 0.0f);
		if (smearingAlgorithm == SmearingAlgorithm::CLASSIC)	ProcessSmearingConvolution(leftInputBuffer, outputBuffer);		
		else													ProcessSmearingComplexConvolution(leftInputBuffer, outputBuffer);

		// 3. Copy inverted convolved buffer to right side of symmetric output (except for last sample)		
		//for (int i = 0; i < leftInputBuffer.size() - 1; i++)
		for (int i = 1; i < leftInputBuffer.size() - 1; i++)
		{
			outputBuffer.push_back(outputBuffer[leftInputBuffer.size() - i - 1]);
		}

		// TEST
		//for (int i = 0; i < outputBuffer.size(); i++)
		//{
		//	WATCH(WV_BUFFER_TEST, outputBuffer[i], float);
		//}
		//		
	}

	void CFrequencySmearing::ProcessOutputBuffer_OverlapAddMethod(std::vector<float>& input_ConvResultBuffer, std::vector<float>& outBuffer)
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

	double CFrequencySmearing::CalculateRoundToZero(double number)
	{
		if (std::abs(number) < FSMEARING_THRESHOLD) {	return 0.0f;}
		else{ return number;}
	}
	
	CMonoBuffer<CMonoBuffer<float>> CFrequencySmearing::CalculateAuditoryFilter(int lowerSideBroadening, int upperSideBroadening)
	{
		CMonoBuffer<CMonoBuffer<float>> auditoryFilter;
		auditoryFilter.resize(bufferSize);

		// Initializing all-zeros (bufferSize, bufferSize) matrix
		for (int i = 0; i < bufferSize; i++)
		{
			auditoryFilter[i].reserve(bufferSize);
			auditoryFilter[i].insert(auditoryFilter[i].end(), bufferSize, 0.0f);
		}

		// First row is all-zeros except first number, calculated independently to avoid division by zero
		auditoryFilter[0][0] = 1.0f / (((float)lowerSideBroadening + (float)upperSideBroadening) / 2.0f);
		
		// Remaining rows are calculated element-by-element
		float fhz, erbhz, pl, pu, g, erbNorm;
		for (int i = 1; i < bufferSize; i++)
		{
			// Filter constants calculation
			fhz = ((float)i)*(float)samplingRate / (2.0f * (float)bufferSize);
			erbhz = 24.7f * ((fhz * 0.00437f) + 1.0f);
			pl = 4.0f * fhz / (erbhz * (float)lowerSideBroadening);
			pu = 4.0f * fhz / (erbhz * (float)upperSideBroadening);
			erbNorm = erbhz * ((float)lowerSideBroadening + (float)upperSideBroadening) / (49.4f);
			
			// Filling row i 
			for (int j = 0; j < bufferSize; j++)
			{
				g = abs((float) (i - j)) / (float) i;

				if (j < i)	auditoryFilter[i][j] = ((1.0f + pl*g)*exp(-pl*g)) / erbNorm;	// Lower side
				else		auditoryFilter[i][j] = ((1.0f + pu*g)*exp(-pu*g)) / erbNorm;	// Upper side
			}

		}

		return auditoryFilter;
		
	}

	CMonoBuffer<CMonoBuffer<float>> CFrequencySmearing::ExtendMatrix(CMonoBuffer<CMonoBuffer<float>>& inputMatrix)
	{
		// Output matrix will be a copy of the input matrix with extra zeros at the end of each row
		CMonoBuffer<CMonoBuffer<float>> outputMatrix = inputMatrix;
		int size = outputMatrix.size();
		
		// Adding size/2 zeros at the end of each row
		for (int i = 0; i < size; i++) {
			outputMatrix[i].resize(3 * size / 2);		// First the row must be resized
			for (int j = size; j < 3 * size / 2; j++) 
				outputMatrix[i][j] = 0.0f;				// Initialized with zeros
		}

		return outputMatrix;
	}

	CMonoBuffer<CMonoBuffer<float>> CFrequencySmearing::Solve(CMonoBuffer<CMonoBuffer<float>>& matrixA, CMonoBuffer<CMonoBuffer<float>>& matrixB)
	{
		//TODO: return A\B 
		return matrixB;
	}

	void CFrequencySmearing::CalculateSmearingMatrix()
	{
		CMonoBuffer<CMonoBuffer<float>> normalMatrix = CalculateAuditoryFilter(1, 1);
		CMonoBuffer<CMonoBuffer<float>> widenMatrix = CalculateAuditoryFilter(downwardSmearing_Hz+200, upwardSmearing_Hz+200);
		CMonoBuffer<CMonoBuffer<float>> normalMatrixExtended = ExtendMatrix(normalMatrix);
		
		// Completing right part of the auditory filters in extended part of the matrix
		for (int i = bufferSize / 2; i < bufferSize; i++)
		{
			for (int j = 0; j < min(2 * i - 1, bufferSize / 2); j++)
			{
				normalMatrixExtended[i][bufferSize + j] = normalMatrix[i][i-j-1];
			}
		}

		// smearingMatrix = normalMatrixExtended \ widenMatrix; 
		smearingMatrix = Solve(normalMatrixExtended, widenMatrix);
		
		// Discarding extended part of the matrix
		for (int i = 0; i < bufferSize; i++) smearingMatrix[i].resize(bufferSize);
		
	}

	void CFrequencySmearing::CalculateSmearingWindow()
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
		for (int i = downwardSmearingBufferSize-1; i >= 0; i--)
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

	float CFrequencySmearing::CalculateGaussianProbability(float mean, float deviation, float value)
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

	void CFrequencySmearing::SetDownwardSmearingBufferSize(int downwardSize)
	{
		ASSERT(downwardSize > 0, RESULT_ERROR_OUTOFRANGE, "Smearing window size must be a positive value", "");
		downwardSmearingBufferSize = downwardSize;		
		CalculateSmearingWindow();
	}

	void CFrequencySmearing::SetUpwardSmearingBufferSize(int upwardSize)
	{
		ASSERT(upwardSize > 0, RESULT_ERROR_OUTOFRANGE, "Smearing window size must be a positive value", "");
		upwardSmearingBufferSize = upwardSize;		
		CalculateSmearingWindow();
	}

	void CFrequencySmearing::SetDownwardSmearing_Hz(float downwardSmearing)
	{
		ASSERT(downwardSmearing >= 0.0f, RESULT_ERROR_OUTOFRANGE, "Smearing amount must be a positive (or zero) value in Hz", "");
		downwardSmearing_Hz = downwardSmearing;
		CalculateSmearingWindow();
	}

	void CFrequencySmearing::SetUpwardSmearing_Hz(float upwardSmearing)
	{
		ASSERT(upwardSmearing >= 0.0f, RESULT_ERROR_OUTOFRANGE, "Smearing amount must be a positive (or zero) value in Hz", "");
		upwardSmearing_Hz = upwardSmearing;
		CalculateSmearingWindow();
	}

	CMonoBuffer<float>* CFrequencySmearing::GetSmearingWindow()
	{
		return &smearingWindow;
	}

	bool CFrequencySmearing::IsCloseToZero(float value)
	{
		if (std::abs(value) < FSMEARING_THRESHOLD)
			return true;
		else
			return false;
	}

	void CFrequencySmearing::InitializePreviousBuffers()
	{
		if (setupDone) {
			previousBuffer.clear();
			storageBuffer.clear();
			hannWindowBuffer.clear();
			smearingWindow.clear();
		}
		for (int i = 0; i < 3; i++) {
			storageLastBuffer[i].reserve(bufferSize);
			storageLastBuffer[i].insert(storageLastBuffer[i].begin(), bufferSize, 0.0f);
		}
		previousBuffer.insert(previousBuffer.begin(), bufferSize, 0.0f);

		previousBuffer.resize(bufferSize);
		storageBuffer.resize(bufferSize);
	}

	void CFrequencySmearing::ProcessSmearingComplexConvolution(CMonoBuffer<float> &inputBuffer, CMonoBuffer<float> &outputBuffer)
	{
		ASSERT(inputBuffer.size() == outputBuffer.size(), RESULT_ERROR_BADSIZE, "Smearing convolution process requires output buffer to be of the same size of input source signal", "");

		// Each frequency is convolved with a different smearing window
		for (int n = 0; n < inputBuffer.size(); n++)
		{
			// Initialize sample in zero
			outputBuffer[n] = 0.0f;

			// Adding to sample the contributions of the corresponding row
			for (int m = 0; m < inputBuffer.size(); m++) outputBuffer[n] += inputBuffer[m] * smearingMatrix[n][m];
			
		}
	}
	
	void CFrequencySmearing::ProcessSmearingConvolution(CMonoBuffer<float> &inputBuffer, CMonoBuffer<float> &outputBuffer)
	{
		ASSERT(inputBuffer.size() == outputBuffer.size(), RESULT_ERROR_BADSIZE, "Smearing convolution process requires output buffer to be of the same size of input source signal", "");

		//for (int outputIndex = 0; outputIndex < inputBuffer.size(); outputIndex++)				
		//{			
		//	outputBuffer[outputIndex] = 0.0f;						
		//	for (int smearingIndex = 0; smearingIndex < smearingWindow.size(); smearingIndex++)			
		//	{		
		//		int inputIndex = inputBuffer.size() - 1 - outputIndex + smearingIndex - downwardSmearingBufferSize;
		//		if (( inputIndex >= 0) && (inputIndex < outputBuffer.size()))
		//			outputBuffer[outputIndex] += inputBuffer[inputIndex] * smearingWindow[smearingIndex];
		//	}			
		//}

		for (int n = 0; n < inputBuffer.size(); n++)
		{
			outputBuffer[n] = 0.0f;
			for (int m = 0; m < inputBuffer.size(); m++)
			{
				int smearingWindowIndex = n - m + downwardSmearingBufferSize;
				if ( (smearingWindowIndex >= 0) && (smearingWindowIndex < smearingWindow.size()) )
					outputBuffer[n] += inputBuffer[m] * smearingWindow[smearingWindowIndex];
			}
		}
	}
	
}//end namespace