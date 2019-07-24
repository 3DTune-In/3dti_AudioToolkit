/**
* \class CFprocessor
*
* \brief  Declaration of CFprocessor class interface.
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

#ifndef _CFPROCESSOR_H_
#define _CFPROCESSOR_H_

//#include "Buffer.h"
//#include <math.h>
#include <iostream>
#include <vector>
#include "fftsg.h"
#include "Buffer.h"

#ifndef THRESHOLD
#define THRESHOLD 0.0000001f
#endif

namespace Common {

	/** \details This class implements the necessary algorithms to do the convolution, in frequency domain, between signal and a impulse response.
	*/
	class CFprocessor
	{

	public:

		/** \brief Default constructor
		*/
		CFprocessor();
		
		/** \brief Calculate the FFT of B points the input signal. Where B = 2^n = (N + k).
		*   \details This method will extend the input buffer with zeros (k) until be power of 2 and then made the FFT.
		*	\param [in] inputAudioBuffer_time vector containing the samples of input signal in time-domain. N is this buffer size.
		*	\param [out] outputAudioBuffer_frequency FFT of the input signal. Have a size of B * 2, because contains the real and imaginary parts of each B point.
		*/
		static void CalculateFFT(const std::vector<float>& inputAudioBuffer_time, std::vector<float>& outputAudioBuffer_frequency);

		/** \brief Calculate the FFT of B points of the input signal in order to make a convolution with other vector. Where B = (N + P + k).
		*   \details This method will extend the input buffer with zeros (k) until reaching the size of B = 2^n = (N + P + k) and then make the FFT.
		*	\param [in] inputAudioBuffer_time vector containing the samples of input signal in time-domain. N is this buffer size.
		*	\param [out] outputAudioBuffer_frequency FFT of the input signal. Have a size of B * 2 = (N + P + k) * 2, because contains the real and imaginary parts of each B point.
		*	\param [in] irDataLength is P, the size in the time domain of the other vector which is going to do the convolved with this one in the frequency domain (multiplication).
		*/
		static void CalculateFFT(const std::vector<float>& inputAudioBuffer_time, std::vector<float>& outputAudioBuffer_frequency, int irDataLength);
					
		/** \brief Get the IFFT of K points of the input signal buffer. 
		*   \details This method makes the IFFT of the input buffer. This method doesn't implement OLA or OLS algothim, it doesn't resolve the inverse convolution.
		*   \param [in] inputAudioBuffer_frequency Vector of samples storing the output signal in frecuency domain. This buffers has to be size of K
		*   \param [out] outputAudioBuffer_time Vector of samples where the IFFT of the output signal will be returned in time domain. This vector will have a size of K/2.
		*	\pre inputAudioBuffer_frequency has to have the same size that the one returned by any of the CalculateFFT_ methods.
		*   \throws May throw exceptions and errors to debugger
		*/
		static void CalculateIFFT(const std::vector<float>& inputAudioBuffer_frequency, std::vector<float>& outputAudioBuffer_time);

		/** \brief Process complex multiplication between the elements of two vectors.
		*   \details This method makes the complex multiplication of vector samples: (a+bi)(c+di) = (ac-bd)+i(ad+bc)
		*   \param [in] x Vector of samples that has real and imaginary parts interlaced. x[i] = Re[Xj], x[i+1] = Img[Xj]
		*   \param [in] h Vector of samples that has real and imaginary parts interlaced. h[i] = Re[Hj], h[i+1] = Img[Hj]
		*	\param [out] y Complex multiplication of x and h vectors
		*	\pre Both vectors (x and h) have to be the same size
		*   \throws May throw exceptions and errors to debugger
		*/
		static void ProcessComplexMultiplication(const std::vector<float>& x, const std::vector<float>& h, std::vector<float>& y);

		/** \brief Process a buffer with complex numbers to get two separated vectors one with the modules and other with the phases.
		*   \details This method return two vectors with the module and phase of the vector introduced.
		*   \param [in] inputBuffer Vector of samples that has real and imaginary parts interlaced. inputBuffer[i] = Re[Xj], x[i+1] = Img[Xj]
		*	\param [out] moduleBuffer Vector of real numbers that are the module of the complex numbers. moduleBuffer[i] = sqrt(inputBuffer[2 * i]^2 * inputBuffer[2 * i + 1]^2)
		*	\param [out] phaseBuffer  Vector of real numbers that are the argument of the complex numbers.	phaseBuffer [i] = atan(inputBuffer[2 * i + 1] / inputBuffer[2 * i]^2)		
		*   \throws May throw exceptions and errors to debugger
		*/
		static void ProcessToModulePhase(const std::vector<float>& inputBuffer, std::vector<float>& moduleBuffer, std::vector<float>& phaseBuffer);

		/** \brief Process a buffer with complex numbers to get two separated vectors one with the powers and other with the phases.
		*   \details This method return two vectors with the power and phase of the vector introduced.
		*   \param [in] inputBuffer Vector of samples that has real and imaginary parts interlaced. inputBuffer[i] = Re[Xj], x[i+1] = Img[Xj]
		*	\param [out] powerBuffer Vector of real numbers that are the power of the complex numbers. moduleBuffer[i] = inputBuffer[2 * i]^2 * inputBuffer[2 * i + 1]^2
		*	\param [out] phaseBuffer  Vector of real numbers that are the argument of the complex numbers.	phaseBuffer [i] = atan(inputBuffer[2 * i + 1] / inputBuffer[2 * i]^2)
		*   \throws May throw exceptions and errors to debugger
		*/
		static void ProcessToPowerPhase(const std::vector<float>& inputBuffer, std::vector<float>& powerBuffer, std::vector<float>& phaseBuffer);

		/** \brief Process two buffers with module and phase of complex numbers, in order to get a vector with complex numbers in binomial way
		*   \details This method return one vectors that has real and imaginary parts interlaced. inputBuffer[i] = Re[Xj], x[i+1] = Img[Xj]
		*   \param [in] moduleBuffer Vector of samples that represents the module of complex numbers.
		*   \param [in] phaseBuffer Vector of samples that represents the argument of complex numbers.
			\param [out] outputBuffer Vector of samples that has real and imaginary parts interlaced. outputBuffer[i] = Re[Xi] = moduleBuffer[i] * cos(phaseBuffer[i]), outputBuffer[i+1] = Img[Xi] = moduleBuffer[i] * sin(phaseBuffer[i])
		*   \throws May throw exceptions and errors to debugger
		*/
		static void ProcessToRealImaginary(const std::vector<float>& moduleBuffer, const std::vector<float>& phaseBuffer, std::vector<float>& outputBuffer);

		/** \brief Initialize the class and allocate memory in other to use the CalculateIFFT_OLA method.
		*   \details When this method is called, the system initializes variables and allocates memory space for the buffer.
		*	\param [in] _inputSize size of the input signal buffer (L size)
		*	\param [in] _impulseResponseSize size of the Impulse Response, which is the size of the buffer that contains the AIR or HRIR signal (P size)
		*   \throws May throw exceptions and errors to debugger
		*/
		void SetupIFFT_OLA(int _inputSize, int _impulseResponseSize);

		/** \brief Calculate the IFFT of the output signal using OLA (Overlap-Add) algorithm.
		*   \details This method makes the IFFT of the signal using OLA (Overlap-Add) algorithm. Do the IFFT, adds the samples obtained with buffer samples in order to get output signal, and updates the buffer.
		*   \param [in] signal_frequency Vector of samples storing the output signal in frecuency domain.
		*   \param [out] signal_time Vector of samples where the IFFT of the output signal will be returned in time domain. This vector will have the size indicated in \link SetupIFFT_OLA \endlink method.
		*	\pre signal_frequency has to have the same size that the one returned by any of the CalculateFFT_ methods.
		*   \throws May throw exceptions and errors to debugger
		*	\sa CalculateFFT_Input, CalculateFFT_IR
		*/
		void CalculateIFFT_OLA(const std::vector<float>& signal_frequency, std::vector<float>& signal_time);

		/** \brief This method check if a number is a power of 2
		*	\param [in] integer to check
		*	\param [out] return true if the number is power of two
		*/
		static bool CalculateIsPowerOfTwo(int x);

	private:
		// ATTRIBUTES	
		int inputSize;			//Size of the inputs buffer		
		int IRSize;				//Size of the AmbiIR buffer
		int FFTBufferSize;		//Size of the outputbuffer and zeropadding buffers	
		double normalizeCoef;		//Coef to normalize the Inverse FFT
		int ip_size;			//Size of the auxiliary array ip;
		int w_size;				//Size of the auxiliary array w;	
		bool setupDone;			//It's true when setup has been called at least once
		std::vector<double> storageBuffer;		//To store the results of the convolution


		// METHODS 	

		// brief This method copies the input vector into an array and insert the imaginary part.
		static void ProcessAddImaginaryPart(const std::vector<float>& input, std::vector<double>& output);
		
		//This method copy the FFT-1 output array into the storage vector, remove the imaginary part and normalize the output.	
		void ProcessOutputBuffer_IFFT_OverlapAddMethod(std::vector<double>& input, std::vector<float>& outBuffer);

		//This method rounds to zero a value that is very close to zero.
		static double CalculateRoundToZero(double number);

		//This method Round up to the next highest power of 2 
		static int CalculateNextPowerOfTwo(int v);
	};
}//end namespace Common
#endif
