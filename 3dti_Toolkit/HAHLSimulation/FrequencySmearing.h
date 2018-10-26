/**
* \class CFrequencySmearing
*
* \brief  Declaration of CFrequencySmearing interface.
* \date	October 2017
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

#ifndef _CFSMEARING_H_
#define _CFSMEARING_H_

//#define _USE_MATH_DEFINES

#include <vector>
#include <Common/Buffer.h>
#include <Common/Fprocessor.h>

#include <Eigen/QR>

using namespace Eigen;

#ifndef FSMEARING_THRESHOLD
#define FSMEARING_THRESHOLD 0.0000001f
#endif

#define DEFAULT_SMEARING_SECTION_SIZE 1
#define DEFAULT_SMEARING_HZ 0.0f

namespace HAHLSimulation {

	// Bidimensional Float CMonoBuffer type definition
	typedef CMonoBuffer<CMonoBuffer<float>> BidimensionalFloatMonoBuffer;

	/** \details This class implements frequency smearing, which simulates the broadening of auditory filters in sensorineural hearing loss
	*/
	class CFrequencySmearing
	{

	public:

		enum SmearingAlgorithm {
			CLASSIC, SUBFRAME
		};

		/** \brief Default constructor
		*/
		CFrequencySmearing();

		/** \brief Initialize the class and allocate memory.
		*   \details When this method is called, the system initializes variables and allocates memory space for the buffer.
		*	\param [in] _bufferSize size of the input signal buffer (L size)		
		*	\param [in] _samplingRate sampling rate, in Hz
		*   \eh On success, RESULT_OK is reported to the error handler.
		*       On error, an error code is reported to the error handler.
		*/
		void Setup(int _bufferSize, float _samplingRate, SmearingAlgorithm _smearingAlgorithm);

		/** \brief Process one buffer through frequency smearing effect.		
		*	\param [in] inputBuffer input buffer, in frequency domain
		*	\param [out] outputBuffer output buffer, in frequency domain
		*   \eh On error, an error code is reported to the error handler.
		*/
		void Process(const CMonoBuffer<float>& inputBuffer, CMonoBuffer<float>& outputBuffer);

		/** \brief Set the buffer size for the downward section of the smearing window, in number of samples		
		*	\details Frequencies below this size are truncated
		*	\param [in] downwardSize size in number of samples
		*   \eh On error, an error code is reported to the error handler.
		*/
		void SetDownwardSmearingBufferSize(int downwardSize);

		/** \brief Set the buffer size for the upward section of the smearing window, in number of samples
		*	\details Frequencies above this size are truncated
		*	\param [in] upwardSize size in number of samples
		*   \eh On error, an error code is reported to the error handler.
		*/
		void SetUpwardSmearingBufferSize(int upwardSize);

		/** \brief Set the amount of smearing (standard deviation) for the downward section of the smearing window, in Hz				
		*	\param [in] downwardSmearing amount of smearing, in Hz
		*   \eh On error, an error code is reported to the error handler.
		*/
		void SetDownwardSmearing_Hz(float downwardSmearing);

		/** \brief Set the amount of smearing (standard deviation) for the upward section of the smearing window, in Hz
		*	\param [in] upwardSmearing amount of smearing, in Hz
		*   \eh On error, an error code is reported to the error handler.
		*/
		void SetUpwardSmearing_Hz(float upwardSmearing);

		/** \brief Get a (frequency-domain) buffer containing the smearing window
		*	\retval smearingWindow smearing window buffer, in frequency domain
		*   \eh Nothing is reported to the error handler.
		*/
		CMonoBuffer<float>* GetSmearingWindow();


		/** \brief Get frequency smearing algorithm
		*	\retval smearingAlgorithm enum
		*   \eh Nothing is reported to the error handler.
		*/
		SmearingAlgorithm GetSmearingAlgorithm();

		/** \brief Set frequency smearing algorithm
		*	\param [in] _smearingAlgorithm enum
		*   \eh Nothing is reported to the error handler.
		*/
		void SetSmearingAlgorithm(SmearingAlgorithm _smearingAlgorithm);

		

	private:
		
		/////////////
		// METHODS
		/////////////
		// Initialize the hann window buffer according to its size 
		void CalculateHannWindow();

		// Multiply the input buffer by the hann window
		void ProcessHannWindow(const CMonoBuffer<float>& inputBuffer, CMonoBuffer<float>& outputBuffer);
		
		// Overlap ADD method to get the correct output buffer and update the storageBuffer
		void ProcessOutputBuffer_OverlapAddMethod(std::vector<float>& input_ConvResultBuffer, std::vector<float>& outBuffer);		
		
		// This method rounds to zero a value that is very close to zero.
		double CalculateRoundToZero(double number);

		// Do the actual frequency smearing process
		void ProcessSmearing(const CMonoBuffer<float>& inputBuffer, CMonoBuffer<float>& outputBuffer);

		// Generate the smearing window used for convolution
		void CalculateSmearingWindow();

		// Generate the smearing matrix used for convolution
		void CalculateSmearingMatrix();

		// Calculate probability for a single value following a gaussian distribution
		float CalculateGaussianProbability(float mean, float deviation, float value);

		// Configures 
		void SmearingWindowSetup();

		// Tell if a float value is zero or close to zero
		bool IsCloseToZero(float value);

		void InitializePreviousBuffers();

		void ProcessSubframe(const CMonoBuffer<float>& inputBuffer, CMonoBuffer<float>& outputBuffer);

		void ProcessClassic(const CMonoBuffer<float>& inputBuffer, CMonoBuffer<float>& outputBuffer);

		// Process 1D convolution of input buffer with smearing window, with output size equal to input size.
		// To achieve same size, convolution starts from the zero point of the smearing window and ends at the same point.
		void ProcessSmearingConvolution(CMonoBuffer<float> &inputBuffer, CMonoBuffer<float> &outputBuffer);

		// Processes input buffer with different smearing window for each frequency value, with output size equal to input size.
		// To achieve same size, convolution starts from the zero point of the smearing window and ends at the same point.
		void ProcessSmearingComplexConvolution(CMonoBuffer<float> &inputBuffer, CMonoBuffer<float> &outputBuffer);

		// Calculates an auditory filter matrix given frequency's lower and upper sides broadening values
		BidimensionalFloatMonoBuffer CalculateAuditoryFilter(float lowerSideBroadening, float upperSideBroadening);

		// Extends a matrix by adding all-zeros columns, getting an output matrix of size (originalSize, 3*originalSize/2)
		BidimensionalFloatMonoBuffer ExtendMatrix(BidimensionalFloatMonoBuffer& inputMatrix);

		MatrixXf BidimensionalCMonoBufferToEigenMatrix(BidimensionalFloatMonoBuffer& input);

		BidimensionalFloatMonoBuffer EigenMatrixToBidimensionalCMonoBuffer(MatrixXf& input);
		
		// Returns A\B (matrix left division)
		BidimensionalFloatMonoBuffer Solve(BidimensionalFloatMonoBuffer& matrixA, BidimensionalFloatMonoBuffer& matrixB);

		///////////////
		// ATTRIBUTES	
		///////////////
		int bufferSize;										//Size of the inputs buffer		
		float samplingRate;									//Sampling rate, in Hz
		bool setupDone;										//It's true when setup has been called at least once
		float oneSampleBandwidth;							//Precomputed bandwidth for one sample of frequency-domain buffer, in Hz
		CMonoBuffer<float> previousBuffer;					//To store the previous buffer
		CMonoBuffer<float> storageBuffer;					//To store partial results of the convolution from previous buffer
		CMonoBuffer<float> storageLastBuffer[3];			//To store needed partial results of the convolution from last buffer
		CMonoBuffer<float> hannWindowBuffer;				//To store the hann window
		CMonoBuffer<float> smearingWindow;					//To store the smearing window	
		BidimensionalFloatMonoBuffer smearingMatrix;		//To store the smearing matrix

		// Configurable parameters		
		int downwardSmearingBufferSize;		 // Size of downward section of smearing window, in number of samples, for classic algorithm
		int upwardSmearingBufferSize;		 // Size of upward section of smearing window, in number of samples, for classic algorithm
		float downwardSmearing_Hz;			 // Amount of smearing (standard deviation) of downward section of smearing window, in Hz, for classic algorithm
		float upwardSmearing_Hz;			 // Amount of smearing (standard deviation) of upward section of smearing window, in Hz, for classic algorithm
		float downwardBroadeningFactor;		 // Downward broadening factor for subframe algorithm
		float upwardBroadeningFactor;		 // Upward broadening factor for subframe algorithm
		SmearingAlgorithm smearingAlgorithm; // Smearing algorithm, either the classic algorithm or subframe algorithm
	};
}
#endif
