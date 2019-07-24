/**
* \class CBaerMooreFrequencySmearing
*
* \brief Definition of CBaerMooreFrequencySmearing class.
*
* This class implements the necessary algorithms to do the Baer-Moore frequency smearing approach for HL simulation.
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
#include <HAHLSimulation/BaerMooreFrequencySmearing.h>
#include <Common/ErrorHandler.h>
#include <cmath>

using namespace Eigen;

// 1 / (sqrt(2*PI))
#ifndef INVERSE_SQRT_2PI
#define INVERSE_SQRT_2PI 0.39894228f
#endif

namespace HAHLSimulation {
	/////////////////////////////
	// CONSTRUCTOR/DESTRUCTOR  //
	/////////////////////////////
	CBaerMooreFrequencySmearing::CBaerMooreFrequencySmearing() : bufferSize{ 0 }, setupDone{ false }
	{
	}

	///////////////////
	// Public Methods //
	///////////////////

	//Initialize the class and allocate memory.
	void CBaerMooreFrequencySmearing::Setup(int _bufferSize, float _samplingRate)
	{
		ASSERT(_bufferSize > 0, RESULT_ERROR_BADSIZE, "Bad buffer size when setting up frequency smearing", "");

		if (_bufferSize > 0)		//In case the error handler is off
		{

			bufferSize = _bufferSize;					//Store the new buffer size
			samplingRate = _samplingRate;				//Store the new sampling rate

			InitializePreviousBuffers();

			downwardBroadeningFactor = MIN_SMEARING_BROADENING_FACTOR;
			upwardBroadeningFactor = MIN_SMEARING_BROADENING_FACTOR;

			SmearingFunctionSetup();

			setupDone = true;
			SET_RESULT(RESULT_OK, "Smearing frequency succesfully set");
		}
	}//Setup


	void CBaerMooreFrequencySmearing::Process(const CMonoBuffer<float>& inputBuffer, CMonoBuffer<float>& outputBuffer)
	{
		ASSERT(inputBuffer.size() == bufferSize, RESULT_ERROR_BADSIZE, "Bad input size when setting up frequency smearing", "");

		if (setupDone && inputBuffer.size() == outputBuffer.size()) {
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

				//Split in Power and Phase
				CMonoBuffer<float> powerBufferCrossfaded;
				CMonoBuffer<float> phaseBufferCrossfaded;
				Common::CFprocessor::ProcessToPowerPhase(inputBufferCrossfaded_FFT, powerBufferCrossfaded, phaseBufferCrossfaded);

				// SMEARING
				CMonoBuffer<float> smearedModuleBufferCrossfaded;
				ProcessSmearing(powerBufferCrossfaded, smearedModuleBufferCrossfaded);

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
						outputBuffer[j] = storageLastBuffer[0][shift * 3 + j] +
							storageLastBuffer[1][shift * 2 + j] +
							storageLastBuffer[2][shift + j] +
							storagePrevBuffer[0][j];
						break;
					case 1: // Second quarter
						outputBuffer[shift + j] = storageLastBuffer[1][shift * 3 + j] +
							storageLastBuffer[2][shift * 2 + j] +
							storagePrevBuffer[0][shift + j] +
							storagePrevBuffer[1][j];
						break;
					case 2: // Third quarter
						outputBuffer[shift * 2 + j] = storageLastBuffer[2][shift * 3 + j] +
							storagePrevBuffer[0][shift * 2 + j] +
							storagePrevBuffer[1][shift + j] +
							storagePrevBuffer[2][j];
						break;
					case 3: // Fourth quarter
						outputBuffer[shift * 3 + j] = storagePrevBuffer[0][shift * 3 + j] +
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
		else
		{
			outputBuffer.clear();
			outputBuffer.insert(outputBuffer.end(), bufferSize, 0.0f);
		}
	}


	void CBaerMooreFrequencySmearing::SmearingFunctionSetup() {

		//Reserve space to store hann window depending on algorithm		
		hannWindowBuffer.resize(bufferSize);

		//Calculate Smearing Function depending on algorithm
		CalculateSmearingMatrix();

		//Calculate Hann window to this buffer size
		CalculateHannWindow();

	}

	void CBaerMooreFrequencySmearing::CalculateHannWindow()
	{
		int hannWindowSize = hannWindowBuffer.size();
		for (int i = 0; i< hannWindowSize; i++) {
			double temp = (2.0 * M_PI * (double)i) / ((double)hannWindowSize - 1.0);
			hannWindowBuffer[i] = CalculateRoundToZero(0.5 * (1.0 - std::cos(temp))) / sqrt(1.5);
		}
	}

	void CBaerMooreFrequencySmearing::ProcessHannWindow(const CMonoBuffer<float>& inputBuffer, CMonoBuffer<float>& outputBuffer)
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

	void CBaerMooreFrequencySmearing::ProcessSmearing(const CMonoBuffer<float>& inputBuffer, CMonoBuffer<float>& outputBuffer)
	{
		// 1. Get left section [0, PI] of symmetric input buffer for convolution
		// Note: if input buffer size is even, this might have undefined behavior
		CMonoBuffer<float> leftInputBuffer(inputBuffer.begin(), inputBuffer.begin() + inputBuffer.size() / 2 + 1);	// Get left side		

		// 2. Process convolution between smearing window or matrix and left section of input
		outputBuffer.clear();
		outputBuffer.assign(leftInputBuffer.size(), 0.0f);
		
		ProcessSmearingComplexConvolution(leftInputBuffer, outputBuffer);

		// 3. Copy inverted convolved buffer to right side of symmetric output (except for last sample)		
		//for (int i = 0; i < leftInputBuffer.size() - 1; i++)
		for (int i = 1; i < leftInputBuffer.size() - 1; i++)
		{
			outputBuffer.push_back(outputBuffer[leftInputBuffer.size() - i - 1]);
		}

	}

	double CBaerMooreFrequencySmearing::CalculateRoundToZero(double number)
	{
		if (std::abs(number) < FSMEARING_THRESHOLD) { return 0.0f; }
		else { return number; }
	}

	BidimensionalDoubleMonoBuffer CBaerMooreFrequencySmearing::CalculateAuditoryFilter(float lowerSideBroadening, float upperSideBroadening)
	{
		BidimensionalDoubleMonoBuffer auditoryFilter;
		auditoryFilter.resize(bufferSize);

		// Initializing all-zeros (bufferSize, bufferSize) matrix
		for (int i = 0; i < bufferSize; i++)
		{
			auditoryFilter[i].reserve(bufferSize);
			auditoryFilter[i].insert(auditoryFilter[i].end(), bufferSize, 0.0f);
		}

		// First row is all-zeros except first number, calculated independently to avoid division by zero
		auditoryFilter[0][0] = 1.0f / ((lowerSideBroadening + upperSideBroadening) / 2.0f);

		// Remaining rows are calculated element-by-element
		double fhz, erbhz, pl, pu, g, erbNorm;
		for (int i = 1; i < bufferSize; i++)
		{
			// Filter constants calculation
			fhz = ((double)i)*(double)samplingRate / (2.0 * (double)bufferSize);
			erbhz = 24.7 * ((fhz * 0.00437) + 1.0);
			pl = 4.0 * fhz / (erbhz * lowerSideBroadening);
			pu = 4.0 * fhz / (erbhz * upperSideBroadening);
			erbNorm = erbhz * (lowerSideBroadening + upperSideBroadening) / (49.4);

			// Filling row i 
			for (int j = 0; j < bufferSize; j++)
			{
				g = abs((double)(i - j)) / (double)i;

				if (j < i)	auditoryFilter[i][j] = ((1.0 + pl*g)*exp(-pl*g)) / erbNorm;	// Lower side
				else		auditoryFilter[i][j] = ((1.0 + pu*g)*exp(-pu*g)) / erbNorm;	// Upper side
			}

		}

		return auditoryFilter;

	}

	BidimensionalDoubleMonoBuffer CBaerMooreFrequencySmearing::ExtendMatrix(BidimensionalDoubleMonoBuffer& inputMatrix)
	{
		// Output matrix will be a copy of the input matrix with extra zeros at the end of each row
		BidimensionalDoubleMonoBuffer outputMatrix = inputMatrix;
		int size = outputMatrix.size();

		// Adding size/2 zeros at the end of each row
		for (int i = 0; i < size; i++) {
			outputMatrix[i].resize(3 * size / 2);		// First the row must be resized
			for (int j = size; j < 3 * size / 2; j++)
				outputMatrix[i][j] = 0.0f;				// Initialized with zeros
		}

		return outputMatrix;
	}

	MatrixXd CBaerMooreFrequencySmearing::BidimensionalCMonoBufferToEigenMatrix(BidimensionalDoubleMonoBuffer& input)
	{
		MatrixXd output(input.size(), input[0].size());

		for (int i = 0; i < input.size(); i++)
		{
			for (int j = 0; j < input[i].size(); j++)
			{
				output(i, j) = input[i][j];
			}
		}

		return output;
	}

	BidimensionalDoubleMonoBuffer CBaerMooreFrequencySmearing::EigenMatrixToBidimensionalCMonoBuffer(MatrixXd& input)
	{
		BidimensionalDoubleMonoBuffer output;
		output.resize(input.rows());

		for (int i = 0; i < input.rows(); i++)
		{
			output[i].resize(input.cols());
			for (int j = 0; j < input.cols(); j++)
			{
				output[i][j] = input(i, j);
			}
		}

		return output;
	}

	BidimensionalDoubleMonoBuffer CBaerMooreFrequencySmearing::Solve(BidimensionalDoubleMonoBuffer& matrixA, BidimensionalDoubleMonoBuffer& matrixB)
	{
		// Declaration of Eigen's dynamic float matrices A, B and X, where X = A\B
		MatrixXd a(matrixA.size(), matrixA[0].size());
		MatrixXd b(matrixB.size(), matrixB[0].size());
		MatrixXd x(max(matrixA.size(), matrixB.size()), max(matrixA[0].size(), matrixB[0].size()));

		// Conversion of matrices A and B to Eigen's dynamic float matrix format
		a = BidimensionalCMonoBufferToEigenMatrix(matrixA);
		b = BidimensionalCMonoBufferToEigenMatrix(matrixB);

		// Calculation of X = A\B
		x = a.colPivHouseholderQr().solve(b).eval();

		// Return X converted to bidimensional float CMonoBuffer
		return EigenMatrixToBidimensionalCMonoBuffer(x);
	}

	void CBaerMooreFrequencySmearing::CalculateSmearingMatrix()
	{
		smearingMatrix.resize(bufferSize);
		for (int i = 0; i < bufferSize; i++) smearingMatrix[i].resize(bufferSize);

		BidimensionalDoubleMonoBuffer normalMatrix = CalculateAuditoryFilter(1, 1);
		BidimensionalDoubleMonoBuffer widenMatrix = CalculateAuditoryFilter(downwardBroadeningFactor, upwardBroadeningFactor);
		BidimensionalDoubleMonoBuffer normalMatrixExtended = ExtendMatrix(normalMatrix);

		// Completing right part of the auditory filters in extended part of the matrix
		for (int i = bufferSize / 2; i < bufferSize; i++)
		{
			for (int j = 0; j < min(2 * i - bufferSize + 1, bufferSize / 2); j++)
			{
				normalMatrixExtended[i][bufferSize + j] = normalMatrix[i][2 * i - bufferSize - j];
			}
		}

		// smearingMatrix = normalMatrixExtended \ widenMatrix; 
		smearingMatrix = Solve(normalMatrixExtended, widenMatrix);

		// Resizing output matrix
		smearingMatrix.resize(bufferSize);
		for (int i = 0; i < bufferSize; i++) smearingMatrix[i].resize(bufferSize);

	}

	void CBaerMooreFrequencySmearing::SetDownwardBroadeningFactor(float _downwardBroadeningFactor)
	{

		if (_downwardBroadeningFactor > MIN_SMEARING_BROADENING_FACTOR)
			downwardBroadeningFactor = _downwardBroadeningFactor;
		else
			downwardBroadeningFactor = MIN_SMEARING_BROADENING_FACTOR;

		setupDone = false;
		SmearingFunctionSetup();
		setupDone = true;
	}

	void CBaerMooreFrequencySmearing::SetUpwardBroadeningFactor(float _upwardBroadeningFactor)
	{
		if (_upwardBroadeningFactor > MIN_SMEARING_BROADENING_FACTOR)
			upwardBroadeningFactor = _upwardBroadeningFactor;
		else
			upwardBroadeningFactor = MIN_SMEARING_BROADENING_FACTOR;

		setupDone = false;
		SmearingFunctionSetup();
		setupDone = true;

	}

	void CBaerMooreFrequencySmearing::InitializePreviousBuffers()
	{
		
			previousBuffer.clear();
			hannWindowBuffer.clear();
			smearingMatrix.clear();
		
		for (int i = 0; i < 3; i++) {
			storageLastBuffer[i].reserve(bufferSize);
			storageLastBuffer[i].insert(storageLastBuffer[i].begin(), bufferSize, 0.0f);
		}
		previousBuffer.insert(previousBuffer.end(), bufferSize, 0.0f);
		hannWindowBuffer.insert(hannWindowBuffer.end(), bufferSize, 0.0f);
	}

	void CBaerMooreFrequencySmearing::ProcessSmearingComplexConvolution(CMonoBuffer<float> &inputBuffer, CMonoBuffer<float> &outputBuffer)
	{
		ASSERT(inputBuffer.size() == outputBuffer.size(), RESULT_ERROR_BADSIZE, "Smearing convolution process requires output buffer to be of the same size of input source signal", "");

		// Each frequency is convolved with a different smearing window
		for (int n = 0; n < inputBuffer.size(); n++)
		{
			// Initialize sample in zero
			outputBuffer[n] = 0.0f;

			// Adding to sample the contributions of the corresponding row
			for (int m = 0; m < inputBuffer.size(); m++) outputBuffer[n] += inputBuffer[m] * smearingMatrix[n][m];

			// Power to module
			double moduleSample = std::sqrt(outputBuffer[n]);
			isnormal(moduleSample) ? outputBuffer[n] = moduleSample : outputBuffer[n] = 0.0f;
			
		}
	}

}//end namespace