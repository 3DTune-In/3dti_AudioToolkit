/**
* \class CBaerMooreFrequencySmearing
*
* \brief  Declaration of CBaerMooreFrequencySmearing interface.
* \date	June 2019
*
* \authors 3DI-DIANA Research Group (University of Malaga), in alphabetical order: M. Cuevas-Rodriguez, C. Garre,  D. Gonzalez-Toledo, �. Rodr�guez-Rivero, E.J. de la Rubia-Cuestas, L. Molina-Tanco ||
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

#include <Eigen/QR>
#include <vector>
#include <Common/Buffer.h>
#include <Common/Fprocessor.h>
#include <HAHLSimulation/FrequencySmearing.h>

#define EIGEN_NO_DEBUG 

namespace HAHLSimulation {
	
	// Bidimensional Float CMonoBuffer type definition
	typedef CMonoBuffer<CMonoBuffer<double>> BidimensionalDoubleMonoBuffer;

	class CBaerMooreFrequencySmearing : public CFrequencySmearing
	{
		public:
		
		// Default constructor
		CBaerMooreFrequencySmearing();

		/** \brief Initialize the class and allocate memory.
		*   \details When this method is called, the system initializes variables and allocates memory space for the buffer.
		*	\param [in] _bufferSize size of the input signal buffer (L size)
		*	\param [in] _samplingRate sampling rate, in Hz
		*   \eh On success, RESULT_OK is reported to the error handler.
		*       On error, an error code is reported to the error handler.
		*/
		void Setup(int _bufferSize, float _samplingRate);

		/** \brief Process one buffer through frequency smearing effect.
		*	\param [in] inputBuffer input buffer, in frequency domain
		*	\param [out] outputBuffer output buffer, in frequency domain
		*   \eh On error, an error code is reported to the error handler.
		*/
		void Process(const CMonoBuffer<float>& inputBuffer, CMonoBuffer<float>& outputBuffer);

		/** \brief Set the amount of smearing (standard deviation) for the downward section of the smearing window, in Hz
		*	\param [in] downwardSmearing amount of smearing, in Hz
		*   \eh On error, an error code is reported to the error handler.
		*/
		void SetDownwardBroadeningFactor(float _downwardBroadeningFactor);

		/** \brief Set the amount of smearing (standard deviation) for the upward section of the smearing window, in Hz
		*	\param [in] upwardSmearing amount of smearing, in Hz
		*   \eh On error, an error code is reported to the error handler.
		*/
		void SetUpwardBroadeningFactor(float _upwardBroadeningFactor);
		
		private:
		
		// Initialize the hann window buffer according to its size 
		void CalculateHannWindow();

		// Multiply the input buffer by the hann window
		void ProcessHannWindow(const CMonoBuffer<float>& inputBuffer, CMonoBuffer<float>& outputBuffer);
		
		// This method rounds to zero a value that is very close to zero.
		double CalculateRoundToZero(double number);

		// Do the actual frequency smearing process
		void ProcessSmearing(const CMonoBuffer<float>& inputBuffer, CMonoBuffer<float>& outputBuffer);

		// Generate the smearing window used for convolution
		void CalculateSmearingMatrix();
		
		// Configures 
		void SmearingFunctionSetup();
		
		void InitializePreviousBuffers();

		// Processes input buffer with different smearing window for each frequency value, with output size equal to input size.
		// To achieve same size, convolution starts from the zero point of the smearing window and ends at the same point.
		void ProcessSmearingComplexConvolution(CMonoBuffer<float> &inputBuffer, CMonoBuffer<float> &outputBuffer);
		
				// Calculates an auditory filter matrix given frequency's lower and upper sides broadening values
		BidimensionalDoubleMonoBuffer CalculateAuditoryFilter(float lowerSideBroadening, float upperSideBroadening);

		// Extends a matrix by adding all-zeros columns, getting an output matrix of size (originalSize, 3*originalSize/2)
		BidimensionalDoubleMonoBuffer ExtendMatrix(BidimensionalDoubleMonoBuffer& inputMatrix);

		Eigen::MatrixXd BidimensionalCMonoBufferToEigenMatrix(BidimensionalDoubleMonoBuffer& input);

		BidimensionalDoubleMonoBuffer EigenMatrixToBidimensionalCMonoBuffer(Eigen::MatrixXd& input);

		// Returns A\B (matrix left division)
		BidimensionalDoubleMonoBuffer Solve(BidimensionalDoubleMonoBuffer& matrixA, BidimensionalDoubleMonoBuffer& matrixB);
		
		int bufferSize;										//Size of the inputs buffer		
		float samplingRate;									//Sampling rate, in Hz
		bool setupDone;										//It's true when setup has been called at least once
		CMonoBuffer<float> previousBuffer;					//To store the previous buffer
		CMonoBuffer<float> storageLastBuffer[3];			//To store needed partial results of the convolution from last buffer
		CMonoBuffer<float> hannWindowBuffer;				//To store the Hann window
		BidimensionalDoubleMonoBuffer smearingMatrix;		//To store the smearing matrix
		
		float downwardBroadeningFactor;		 // Downward broadening factor for subframe algorithm
		float upwardBroadeningFactor;		 // Upward broadening factor for subframe algorithm
	};
}