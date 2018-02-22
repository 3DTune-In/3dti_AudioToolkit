/**
* \class UPCAnechoic
*
* \brief Declaration of CUPCAnechoic class interface.
* \date	November 2016
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

#ifndef _CUPCANECHOIC_H_
#define _CUPCANECHOIC_H_

#include <iostream>
#include <vector>
#include <Common/Fprocessor.h>
#include <Common/Buffer.h>
#include <BinauralSpatializer/HRTF.h>

/** \brief Type definition for partitioned HRIR table
*/
typedef std::vector<CMonoBuffer<float>> THRIR_partitioned;

namespace Binaural {

	/** \details This class implements the necessary algorithms to do the convolution, in frequency domain, between signal and a impulse response using the	Uniformly Partitioned Convolution Algorithm (UPC algorithm)
	*/
	class CUPCAnechoic
	{

	public:

		/** \brief Default constructor
		*   \eh Nothing is reported to the error handler.
		*/
		CUPCAnechoic();

		/** \brief Initialize the class and allocate memory.
		*   \details When this method is called, the system initializes variables and allocates memory space for the buffer.
		*	\param [in] _inputSize size of the input signal buffer (B size)
		*	\param [in] _HRIR_Frequency_Block_Size size of the FTT Impulse Response blocks, this number is (2*B + k) = 2^n
		*	\param [in] _HRIR_Block_Number number of blocks in which is divided the the impluse response
		*	\param [in] _IRMemory if true, the method with IR memory will be used (otherwise, the method without memory will be used instead)
		*   \eh On error, an error code is reported to the error handler.
		*/
		void Setup(int _inputSize, int _HRIR_Frequency_Block_Size, int _HRIR_Block_Number, bool _IRMemory);
		
		/** \brief Process the Uniformed Partitioned Convolution of the input signal with one impulse response
		*   \details This method performs the convolution between the input signal and the partitioned HRIR using the UPC* method, returning the FFT of the output.
		*   \details *Wefers, F. (2015). Partitioned convolution algorithms for real-time auralization (Vol. 20). Logos Verlag Berlin GmbH.
		*	\param [in] inBuffer_Time input signal buffer of B size
		*	\param [in] IR buffer structure that contains the HRIR divided in subfilters. Each subfilter with a size of HRIR_Frequency_Block_Size size  = 2*B
		*	\param [out] outBuffer FFT of the output signal of 2*B size (complex numbers). After the IIFT is done, only the last B samples are significant
		*   \eh Nothing is reported to the error handler.
		*/
		void ProcessUPConvolution(const CMonoBuffer<float>& inBuffer_Time, const TOneEarHRIRPartitionedStruct & IR, CMonoBuffer<float>& outBuffer);

		/** \brief Make the Uniformed Partitioned Convolution of the input signal using also last input signal buffers (method with memory)
		*   \details This method performs the convolution between the input signal and the partitioned HRIR (the HRIR has been stored for each input signal block) using the UPC* method, returning the FFT of the output.
		*   \details *Wefers, F. (2015). Partitioned convolution algorithms for real-time auralization (Vol. 20). Logos Verlag Berlin GmbH.
		*	\param [in] inBuffer_Time input signal buffer of B size
		*	\param [in] IR buffer structure that contains the HRIR divided in subfilters. Each subfilter with a size of HRIR_Frequency_Block_Size size  = 2*B
		*	\param [out] outBuffer FFT of the output signal of 2*B size (complex numbers). After the IIFT is done, only the last B samples are significant
		*   \eh Nothing is reported to the error handler.
		*/
		void ProcessUPConvolutionWithMemory(const CMonoBuffer<float>& inBuffer_Time, const TOneEarHRIRPartitionedStruct & IR, CMonoBuffer<float>& outBuffer);

	private:
		// ATTRIBUTES	
		int inputSize;								//Size of the inputs buffer				
		int impulseResponse_Frequency_Block_Size;	//Size of the HRIR buffer
		int impulseResponseNumberOfSubfilters;		//Number of blocks in which is divided the HRIR
		bool impulseResponseMemory;					//Indicate if HRTF storage buffer has to be prepared to do UPC with memory
		bool setupDone;								//It's true when setup has been called at least once
				
		std::vector<float> storageInput_buffer;						//To store the last input signal
		std::vector<vector<float>> storageInputFFT_buffer;			//To store the history of input signals FFTs 
		std::vector<vector<float>>::iterator it_storageInputFFT;	//Declare a general iterator to keep the head of the FTTs buffer
		std::vector<THRIR_partitioned> storageHRIR_buffer;			//To store the HRIR of the orientation of the previous frames
		std::vector<THRIR_partitioned>::iterator it_storageHRIR;		//Declare a general iterator to keep the head of the storageHRIR_buffer		
	};
}
#endif
