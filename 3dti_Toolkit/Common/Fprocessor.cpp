/**
* \class CFprocessor
*
* \brief Definition of CFprocessor class.
*
* This class implements the necessary algorithms to do the convolution, in frequency domain, between signal and a impulse response.
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
#include "Fprocessor.h"
#include "ErrorHandler.h"
#include <cmath>

//#define USE_PROFILER_Fprocessor
#ifdef USE_PROFILER_Fprocessor
#include <Common/Profiler.h>
CProfilerDataSet dataSetFFT;
CProfilerDataSet dataSetComplexMult;
CProfilerDataSet dataSetIFFT;
CProfilerDataSet dataSetOthers;
#endif

namespace Common {
	/////////////////////////////
	// CONSTRUCTOR/DESTRUCTOR  //
	/////////////////////////////
	CFprocessor::CFprocessor() : inputSize{ 0 }, IRSize{ 0 }, FFTBufferSize{ 0 }, setupDone{ false }
	{		
	}

	///////////////////////////
	// Static Public Methods //
	///////////////////////////

	//Calculate the FFT of the input signal
	void CFprocessor::CalculateFFT(const std::vector<float>& inputAudioBuffer_time, std::vector<float>& outputAudioBuffer_frequency)
	{
		int inputBufferSize = inputAudioBuffer_time.size();

		ASSERT(inputBufferSize != 0, RESULT_ERROR_BADSIZE, "Bad input size when setting up frequency convolver", "");

		if (inputBufferSize > 0) //Just in case error handler is off
		{
			///////////////////////////////
			// Calculate FFT/output size //
			///////////////////////////////
			int FFTBufferSize = inputBufferSize;			
			//Check if if power of two, if not round up to the next highest power of 2 
			if (!CalculateIsPowerOfTwo(FFTBufferSize)) {
				FFTBufferSize = CalculateNextPowerOfTwo(FFTBufferSize);
			}
			FFTBufferSize *= 2;							//We multiplicate by 2 because we need to store real and imaginary part

			///////////////////////////////////////////////////////////////////////////////
			// Calculate auxiliary arrays size, necessary to use the Takuya OOURA library
			///////////////////////////////////////////////////////////////////////////////
			int ip_size = std::sqrt(FFTBufferSize / 2) + 2;		//Size of the auxiliary array w. This come from lib documentation/examples.
			int w_size = FFTBufferSize * 5 / 4;					//Size of the auxiliary array w. This come from lib documentation/examples.
			std::vector<int> ip(ip_size);						//Define the auxiliary array ip 
			std::vector<double> w(w_size);						//Define the auxiliary array w			
			ip[0] = 0;											//w[],ip[] are initialized if ip[0] == 0.

			//////////////
			// Make FFT //
			//////////////			
			std::vector<double> inputAudioBuffer_frequency(FFTBufferSize, 0.0f);			//Initialize the vector of doubles to store the FFT					
			ProcessAddImaginaryPart(inputAudioBuffer_time, inputAudioBuffer_frequency);			//Copy the input vector into an vector of doubles and insert the imaginary part.											
			cdft(FFTBufferSize, 1, inputAudioBuffer_frequency.data(), ip.data(), w.data());	//Make the FFT
			
			////////////////////
			// Prepare Output //
			////////////////////	
			//Copy to the output float vector			
			if (outputAudioBuffer_frequency.size() != FFTBufferSize) { outputAudioBuffer_frequency.resize(FFTBufferSize); }
			for (int i = 0; i < inputAudioBuffer_frequency.size(); i++) {
				outputAudioBuffer_frequency[i] = static_cast<float>(inputAudioBuffer_frequency[i]);
			}
		}		
	}

	//Calculate the FFT of the input signal in order to convolved it with other signal
	void CFprocessor::CalculateFFT(const std::vector<float>& inputAudioBuffer_time, std::vector<float>& outputAudioBuffer_frequency, int irDataLength)
	{
		int inputBufferSize = inputAudioBuffer_time.size();
		
		ASSERT(inputBufferSize	> 0, RESULT_ERROR_BADSIZE, "Bad input size when setting up frequency convolver", "");
		ASSERT(irDataLength		> 0, RESULT_ERROR_BADSIZE, "Bad ABIR size when setting up frequency convolver", "");
		
		if ((inputBufferSize > 0) && (irDataLength > 0)) //Just in case error handler is off
		{
			///////////////////////////////
			// Calculate FFT/output size //
			///////////////////////////////
			int FFTBufferSize = inputBufferSize + irDataLength;
			//Check if if power of two, if not round up to the next highest power of 2 
			if (!CalculateIsPowerOfTwo(FFTBufferSize)) {
				FFTBufferSize = CalculateNextPowerOfTwo(FFTBufferSize);
			}
			FFTBufferSize *= 2;			//We multiplicate by 2 because we need to store real and imaginary part

			///////////////////////////////////////////////////////////////////////////////
			// Calculate auxiliary arrays size, necessary to use the Takuya OOURA library
			///////////////////////////////////////////////////////////////////////////////
			int ip_size = std::sqrt(FFTBufferSize / 2) + 2;		//Size of the auxiliary array w. This come from lib documentation/examples.
			int w_size = FFTBufferSize * 5 / 4;					//Size of the auxiliary array w. This come from lib documentation/examples.
			std::vector<int> ip(ip_size);						//Define the auxiliary array ip 
			std::vector<double> w(w_size);						//Define the auxiliary array w			
			ip[0] = 0;											//w[],ip[] are initialized if ip[0] == 0.

			//////////////
			// Make FFT //
			//////////////	
			std::vector<double> inputAudioBuffer_frequency(FFTBufferSize, 0.0f);			//Initialize the vector of doubles to store the FFT					
			ProcessAddImaginaryPart(inputAudioBuffer_time, inputAudioBuffer_frequency);			//Copy the input vector into an vector of doubles and insert the imaginary part.
			cdft(FFTBufferSize, 1, inputAudioBuffer_frequency.data(), ip.data(), w.data());	//Make the FFT

			////////////////////
			// Prepare Output //
			////////////////////			
			outputAudioBuffer_frequency.resize(FFTBufferSize);
			for (int i = 0; i < inputAudioBuffer_frequency.size(); i++) {
				outputAudioBuffer_frequency[i] = static_cast<float>(inputAudioBuffer_frequency[i]);
			}
		}
	}
	
	//This method does the complex multiplication between the vector elements. Both vectors have to be the same size
	void CFprocessor::ProcessComplexMultiplication(const std::vector<float>& x, const std::vector<float>& h, std::vector<float>& y)
	{
		ASSERT(x.size() == h.size(), RESULT_ERROR_BADSIZE, "Complex multiplication in frequency convolver requires two vectors of the same size", "");
		
		if (x.size() == h.size())	//Just in case error handler is off
		{
			y.resize(x.size());
			int end = (int)y.size()*0.5f;
			for (int i = 0; i < end; i++)
			{
				float a = x[2 * i];
				float b = x[2 * i + 1];
				float c = (h[2 * i]);
				float d = (h[2 * i + 1]);

				y[2 * i] = a*c - b*d;
				y[2 * i + 1] = a*d + b*c;
			}		
		}
	}//ComplexMultiplicaton

	//Calculate the IFFT of the output signal
	void CFprocessor::CalculateIFFT(const std::vector<float>& inputAudioBuffer_frequency, std::vector<float>& outputAudioBuffer_time)
	{
		int inputBufferSize = inputAudioBuffer_frequency.size();
		ASSERT(inputBufferSize > 0, RESULT_ERROR_BADSIZE, "Bad input size", "");
		
		if (inputBufferSize > 0) //Just in case error handler is off
		{
			//////////////////////////////
			// Calculate output size	//
			//////////////////////////////
			int FFTBufferSize = inputBufferSize;
			
			///////////////////////////////////////////////////////////////////////////////
			// Calculate auxiliary arrays size, necessary to use the Takuya OOURA library
			///////////////////////////////////////////////////////////////////////////////
			int ip_size = std::sqrt(FFTBufferSize / 2) + 2;		//Size of the auxiliary array w. This come from lib documentation/examples.
			int w_size = FFTBufferSize * 5 / 4;					//Size of the auxiliary array w. This come from lib documentation/examples.																				
			std::vector<int> ip(ip_size);						//Define the auxiliary array ip 
			std::vector<double> w(w_size);						//Define the auxiliary array w			
			ip[0] = 0;											//w[],ip[] are initialized if ip[0] == 0.
			
			///////////////
			// Make IFFT //
			///////////////																
			std::vector<double> outBuffer_temp(inputAudioBuffer_frequency.begin(), inputAudioBuffer_frequency.end());	//Convert to double
			cdft(FFTBufferSize, -1, outBuffer_temp.data(), ip.data(), w.data());										//Make the IFFT

			////////////////////
			// Prepare Output //
			////////////////////	
			int outBufferSize = inputAudioBuffer_frequency.size() / 2;	//Locar var to move throught the outbuffer
			if (outputAudioBuffer_time.size() != outBufferSize) { outputAudioBuffer_time.resize(outBufferSize); }
			float normalizeCoef = 2.0f / FFTBufferSize;			//Store the normalize coef for the FFT-1	
			//Fill out the output signal buffer
			for (int i = 0; i < outBufferSize; i++)	{
				outputAudioBuffer_time[i] = static_cast<float>(CalculateRoundToZero(outBuffer_temp[2 * i] * normalizeCoef));
			}
		}		
	}

	void CFprocessor::ProcessToModulePhase(const std::vector<float>& inputBuffer, std::vector<float>& moduleBuffer, std::vector<float>& phaseBuffer)
	{		
		ASSERT(inputBuffer.size() > 0, RESULT_ERROR_BADSIZE, "Bad input size", "");

		if (inputBuffer.size() > 0) {
			
			moduleBuffer.clear();
			phaseBuffer.clear();

			int end = (int)(inputBuffer.size()*0.5);

			for (int i = 0; i < end; i++)
			{
				float real = inputBuffer[2 * i];
				float img = inputBuffer[2 * i + 1];

				moduleBuffer.push_back(std::sqrt(real*real + img*img));
				phaseBuffer.push_back(std::atan2(img, real));
			}
		}		
	}
 
	void CFprocessor::ProcessToPowerPhase(const std::vector<float>& inputBuffer, std::vector<float>& powerBuffer, std::vector<float>& phaseBuffer)
	{
		ASSERT(inputBuffer.size() > 0, RESULT_ERROR_BADSIZE, "Bad input size", "");

		if (inputBuffer.size() > 0) {

			powerBuffer.clear();
			phaseBuffer.clear();

			int end = (int)(inputBuffer.size()*0.5);

			for (int i = 0; i < end; i++)
			{
				float real = inputBuffer[2 * i];
				float img = inputBuffer[2 * i + 1];

				powerBuffer.push_back(real*real + img*img);
				phaseBuffer.push_back(std::atan2(img, real));
			}
		}
	}

	void CFprocessor::ProcessToRealImaginary(const std::vector<float>& moduleBuffer, const std::vector<float>& phaseBuffer, std::vector<float>& outputBuffer)
	{
		ASSERT(moduleBuffer.size() > 0, RESULT_ERROR_BADSIZE, "Bad input size moduleBuffer", "");
		ASSERT(phaseBuffer.size() > 0, RESULT_ERROR_BADSIZE, "Bad input size phaseBuffer", "");
		ASSERT(moduleBuffer.size() == phaseBuffer.size(), RESULT_ERROR_BADSIZE, "Bad input size, moduleBuffer and phaseBuffer should have the same size", "");		

		if ((moduleBuffer.size() == moduleBuffer.size()) && (moduleBuffer.size() > 0)) 
		{
			outputBuffer.clear();
			for (int i = 0; i < moduleBuffer.size(); i++)
			{
				float a = moduleBuffer[i] * std::cos(phaseBuffer[i]);
				float b = moduleBuffer[i] * std::sin(phaseBuffer[i]);

				outputBuffer.push_back(a);
				outputBuffer.push_back(b);
			}
		}
	}
	
	////////////////////
	// Public Methods //
	////////////////////	 

	//Initialize the class and allocate memory.
	void CFprocessor::SetupIFFT_OLA(int _inputSize, int _AIRSize)
	{
		ASSERT(_inputSize > 0, RESULT_ERROR_BADSIZE, "Bad input size when setting up frequency convolver", "");
		ASSERT(_AIRSize > 0, RESULT_ERROR_BADSIZE, "Bad ABIR size when setting up frequency convolver", "");

		if ((_inputSize > 0) && (_AIRSize > 0))		//Just in case error handler is off
		{
			if (setupDone) {
				storageBuffer.clear();		//If is the second time that this method has been called -> clear all buffers
			}

			///////////////////////////////
			// Calculate FFT/output size //
			///////////////////////////////
			inputSize = _inputSize;
			IRSize = _AIRSize;
			FFTBufferSize = inputSize + IRSize;	
			//Check if if power of two, if not round up to the next highest power of 2 
			if (!CalculateIsPowerOfTwo(FFTBufferSize)) {
				FFTBufferSize = CalculateNextPowerOfTwo(FFTBufferSize);
			}
			storageBuffer.resize(FFTBufferSize);		//Prepare the buffer with the space that we are going to needed
			normalizeCoef = 1.0f / FFTBufferSize;		//Store the normalize coef for the FFT-1
			FFTBufferSize *= 2;							//We multiplicate by 2 because we need to store real and imaginary part
			
			///////////////////////////////////////////////////////////////////////////////
			// Calculate auxiliary arrays size, necessary to use the Takuya OOURA library
			///////////////////////////////////////////////////////////////////////////////
			ip_size = std::sqrt(FFTBufferSize / 2) + 2;		//Size of the auxiliary array w. This come from lib documentation/examples.
			w_size = FFTBufferSize * 5 / 4;				//Size of the auxiliary array w. This come from lib documentation/examples.

			setupDone = true;
			SET_RESULT(RESULT_OK, "Frequency convolver succesfully set");		
		}		
	}//SetupIFFT_OLA

	void CFprocessor::CalculateIFFT_OLA(const std::vector<float>& inputBuffer_frequency, std::vector<float>& outputBuffer_time)
	{
		ASSERT(inputBuffer_frequency.size() == FFTBufferSize, RESULT_ERROR_BADSIZE, "Incorrect size of input buffer when computing inverse FFT in frequency convolver", "");
		ASSERT(setupDone, RESULT_ERROR_NOTINITIALIZED, "SetupIFFT_OLA method should be called before call this method", "");

		if ((setupDone) && (inputBuffer_frequency.size() == FFTBufferSize) )	//Just in case error handler is off
		{
			///////////////////////////////////////////////////////////////////////////////
			// Calculate auxiliary arrays size, necessary to use the Takuya OOURA library
			///////////////////////////////////////////////////////////////////////////////
			//Prepare the FFT						
			std::vector<int> ip(ip_size);	//Define the auxiliary array ip 
			std::vector<double> w(w_size);	//Define the auxiliary array w			
			ip[0] = 0;						//w[],ip[] are initialized if ip[0] == 0.

			///////////////
			// Make IFFT //
			///////////////				
			std::vector<double> outBuffer_temp(inputBuffer_frequency.begin(), inputBuffer_frequency.end());		//Convert to double
			cdft(FFTBufferSize, -1, outBuffer_temp.data(), ip.data(), w.data());								//Make the FFT-1
			
			////////////////////
			// Prepare Output //
			////////////////////
			ProcessOutputBuffer_IFFT_OverlapAddMethod(outBuffer_temp, outputBuffer_time);
		}		
	}

	 /////////////////////
	// Private methods //
	/////////////////////

	//This method copies the input vector into an array and insert the imaginary part.
	void CFprocessor::ProcessAddImaginaryPart(const std::vector<float>& input, std::vector<double>& output)
	{
		ASSERT(output.size() >= 2 * input.size(), RESULT_ERROR_BADSIZE, "Output buffer size must be at least twice the input buffer size when adding imaginary part in frequency convolver", "");

		for (int i = 0; i < input.size(); i++)
		{
			output[2 * i] = static_cast<double> (input[i]);
			//output[2 * i + 1] = 0;
		}
	}//ProcessAddImaginaryPart
	
	 //This method copy the FFT-1 output array into the storage vector, remove the imaginary part and normalize the output.	
	void CFprocessor::ProcessOutputBuffer_IFFT_OverlapAddMethod(std::vector<double>& input_ConvResultBuffer, std::vector<float>& outBuffer)
	{
		//Prepare the outbuffer
		if (outBuffer.size() < inputSize)
		{
			outBuffer.resize(inputSize);
		}
		//Check buffer sizes	
		ASSERT(outBuffer.size() == inputSize, RESULT_ERROR_BADSIZE, "OutBuffer size has to be zero or equal to the input size indicated by the setup method", "");

		int outBufferSize = inputSize;	//Locar var to move throught the outbuffer
										
		//Fill out the output signal buffer
		for (int i = 0; i < outBufferSize; i++)
		{
			if (i < storageBuffer.size()) {
				outBuffer[i] = static_cast<float>(storageBuffer[i] + CalculateRoundToZero(input_ConvResultBuffer[2 * i] * normalizeCoef));
			}
			else
			{
				outBuffer[i] = static_cast<float>(CalculateRoundToZero(input_ConvResultBuffer[2 * i] * normalizeCoef));
			}
		}
		//Fill out the storage buffer to be used in the next call
		std::vector<double> temp;
		temp.reserve(0.5f * input_ConvResultBuffer.size() - outBufferSize);
		int inputConvResult_size = 0.5f * input_ConvResultBuffer.size();	//Locar var to move to the end of the input_ConvResultBuffer
		for (int i = outBufferSize; i < inputConvResult_size; i++)
		{
			if (i<storageBuffer.size())
			{
				temp.push_back(storageBuffer[i] + CalculateRoundToZero(input_ConvResultBuffer[2 * i] * normalizeCoef));
			}
			else
			{
				temp.push_back(CalculateRoundToZero(input_ConvResultBuffer[2 * i] * normalizeCoef));
			}
		}
		//storageBuffer.swap(temp);				//To use in C++03
		storageBuffer = std::move(temp);			//To use in C++11
	}//ProcessOutputBuffer_IFFT_OverlapAddMethod
		
	 //This method rounds to zero a value that is very close to zero.
	double CFprocessor::CalculateRoundToZero(double number)
	{
		if (std::abs(number) < THRESHOLD) {	return 0.0f;}
		else { return number; }
	}

	//This method check if a number is a power of 2
	bool CFprocessor::CalculateIsPowerOfTwo(int x)
	{
		return (x != 0) && ((x & (x - 1)) == 0);
	}//CalculateIsPowerOfTwo

	//This method Round up to the next highest power of 2 
	int CFprocessor::CalculateNextPowerOfTwo(int v)
	{
		v--;
		v |= v >> 1;
		v |= v >> 2;
		v |= v >> 4;
		v |= v >> 8;
		v |= v >> 16;
		v++;
		return v;
	}//CalculateNextPowerOfTwo
}//end namespace Common