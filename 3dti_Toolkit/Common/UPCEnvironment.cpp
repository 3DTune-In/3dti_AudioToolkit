/**
* \class CUPCEnvironment
*
* \brief  Uniformly Partitioned Convolution Algorithm (UPC algorithm) for reverb path
* \date	January 2017
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


#include <Common/UPCEnvironment.h>
#include <Common/ErrorHandler.h>

namespace Common
{
	CUPCEnvironment::CUPCEnvironment()
	{
		setupDone = false;
	}

	//Initialize the class and allocate memory.
	void CUPCEnvironment::Setup(int _inputSize, int _IR_Frequency_Block_Size, int _IR_Block_Number, bool _IRMemory)
	{
		if (setupDone) {
			//Second time that this method has been called - clear all buffers
			storageInput_buffer.clear();
			storageInputFFT_buffer.clear();
			storageHRIR_buffer.clear();
		}

		inputSize = _inputSize;
		IR_Frequency_Block_Size = _IR_Frequency_Block_Size;
		IR_NumOfSubfilters = _IR_Block_Number;
		IR_Memory = _IRMemory;

		//Prepare the buffer with the space that we are going to need	
		storageInput_buffer.resize(inputSize, 0.0f);

		//Preparing the vector of buffers that is going to store the history of FFTs	
		storageInputFFT_buffer.resize(IR_NumOfSubfilters);
		for (int i = 0; i < IR_NumOfSubfilters; i++) {
			storageInputFFT_buffer[i].resize(IR_Frequency_Block_Size, 0.0f);
		}
		it_storageInputFFT = storageInputFFT_buffer.begin();

		//Preparing the vector of buffers that is going to store the history of the HRIR	
		if (IR_Memory)
		{
			storageHRIR_buffer.resize(IR_NumOfSubfilters);
			for (int i = 0; i < IR_NumOfSubfilters; i++)
			{
				storageHRIR_buffer[i].resize(IR_NumOfSubfilters);
				for (int j = 0; j < IR_NumOfSubfilters; j++)
				{
					storageHRIR_buffer[i][j].resize(IR_Frequency_Block_Size, 0.0f);
				}
			}
			it_storageHRIR = storageHRIR_buffer.begin();
		}

		setupDone = true;
		SET_RESULT(RESULT_OK, "UPC convolver successfully set");
	}//Setup


	void CUPCEnvironment::ProcessUPConvolution(const CMonoBuffer<float>& inBuffer_Time, const TImpulseResponse_Partitioned & IR, CMonoBuffer<float>& outBuffer)
	{
		CMonoBuffer<float> sum;
		sum.resize(IR_Frequency_Block_Size, 0.0f);
		CMonoBuffer<float> temp;

		if (inBuffer_Time.size() == inputSize) {

			//Step 1- extend the input time signal buffer in order to have double length
			std::vector<float> inBuffer_Time_dobleSize;
			inBuffer_Time_dobleSize.reserve(inputSize * 2);
			inBuffer_Time_dobleSize.insert(inBuffer_Time_dobleSize.begin(), storageInput_buffer.begin(), storageInput_buffer.end());
			inBuffer_Time_dobleSize.insert(inBuffer_Time_dobleSize.end(), inBuffer_Time.begin(), inBuffer_Time.end());
			storageInput_buffer = inBuffer_Time;			//Store current input signal

															//Step 2,3 - FFT of the input signal
			CMonoBuffer<float> inBuffer_Frequency;
			Common::CFprocessor::CalculateFFT(inBuffer_Time_dobleSize, inBuffer_Frequency);
			*it_storageInputFFT = inBuffer_Frequency;		//Store the new input FFT into the first FTT history buffers

															//Step 4, 5 - Multiplications and sums
			auto it_product = it_storageInputFFT;

			for (int i = 0; i < IR_NumOfSubfilters; i++) {
				Common::CFprocessor::ProcessComplexMultiplication(*it_product, IR[i], temp);
				sum += temp;
				if (it_product == storageInputFFT_buffer.begin()) {
					it_product = storageInputFFT_buffer.end() - 1;
				}
				else {
					it_product--;
				}
			}
			//Move iterator waiting for the next input block
			if (it_storageInputFFT == storageInputFFT_buffer.end() - 1) {
				it_storageInputFFT = storageInputFFT_buffer.begin();
			}
			else {
				it_storageInputFFT++;
			}
			// Make the IIF
			CMonoBuffer<float> ouputBuffer_temp;
			Common::CFprocessor::CalculateIFFT(sum, ouputBuffer_temp);
			//We are left only with the final half of the result
			int halfsize = (int)(ouputBuffer_temp.size() * 0.5f);
			CMonoBuffer<float> temp_OutputBlock(ouputBuffer_temp.begin() + halfsize, ouputBuffer_temp.end());
			outBuffer = std::move(temp_OutputBlock);			//To use in C++11

		}
		else {
			//TODO: handle size errors
		}
	}

	void CUPCEnvironment::ProcessUPConvolution_withoutIFFT(const CMonoBuffer<float>& inBuffer_Time, const TImpulseResponse_Partitioned & IR, CMonoBuffer<float>& outBuffer, int numberOfSilencedSamples)
	{
		CMonoBuffer<float> sum;
		sum.resize(IR_Frequency_Block_Size, 0.0f);
		CMonoBuffer<float> temp;

		CMonoBuffer<float> zero;
		zero.resize(IR_Frequency_Block_Size, 0.0f);

		ASSERT(inBuffer_Time.size() == inputSize, RESULT_ERROR_BADSIZE, "Bad input size, don't match with the size setting up in the setup method", "");

		if (inBuffer_Time.size() == inputSize && IR.size() != 0 ) 
		{
			//Step 1- extend the input time signal buffer in order to have double length
			std::vector<float> inBuffer_Time_dobleSize;
			inBuffer_Time_dobleSize.reserve(inputSize * 2);
			inBuffer_Time_dobleSize.insert(inBuffer_Time_dobleSize.begin(), storageInput_buffer.begin(), storageInput_buffer.end());
			inBuffer_Time_dobleSize.insert(inBuffer_Time_dobleSize.end(), inBuffer_Time.begin(), inBuffer_Time.end());
			storageInput_buffer = inBuffer_Time;			//Store current input signal

															//Step 2,3 - FFT of the input signal
			CMonoBuffer<float> inBuffer_Frequency;
			Common::CFprocessor::CalculateFFT(inBuffer_Time_dobleSize, inBuffer_Frequency);
			*it_storageInputFFT = inBuffer_Frequency;		//Store the new input FFT into the first FTT history buffers

															//Step 4, 5 - Multiplications and sums
			auto it_product = it_storageInputFFT;
			int numberOfSilencedFrames = floor (numberOfSilencedSamples / inputSize);
			for (int i = 0; i < IR_NumOfSubfilters; i++) {

				if (i >= numberOfSilencedFrames)
					Common::CFprocessor::ProcessComplexMultiplication(*it_product, IR[i], temp);
				else
					Common::CFprocessor::ProcessComplexMultiplication(*it_product, zero, temp);

				sum += temp;
				if (it_product == storageInputFFT_buffer.begin()) {
					it_product = storageInputFFT_buffer.end() - 1;
				}
				else {
					it_product--;
				}
			
			}
			//Move iterator waiting for the next input block
			if (it_storageInputFFT == storageInputFFT_buffer.end() - 1) {
				it_storageInputFFT = storageInputFFT_buffer.begin();
			}
			else {
				it_storageInputFFT++;
			}
			//outBuffer.swap(temp);				//To use in C++03
			outBuffer = std::move(sum);			//To use in C++11			
		}
		else 
		{		
			SET_RESULT(RESULT_ERROR_BADSIZE, "The input buffer size is not correct or there is not a valid BRIR file loaded to render" );
			outBuffer.resize(inBuffer_Time.size(), 0.0f);
		}
	}//UPC_withoutIFFT
}//end namespace Common