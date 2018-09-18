/**
* \class CBRIR
*
* \brief Declaration of CBRIR class interface
* \details 
* \version Alpha 1.0
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

#include <BinauralSpatializer/BRIR.h>
namespace Binaural {



	//PUBLIC METHODS
	void CBRIR::BeginSetup(int32_t _BRIRLength) 
	{		
		if ((ownerEnvironment!=nullptr) && (ownerEnvironment->ownerCore != nullptr))
		{			
			BRIRLength = _BRIRLength;
			BRIRLength_frequency = _BRIRLength * 2.0f;
			bufferSize = ownerEnvironment->GetCoreAudioState().bufferSize;

			//Clear every table	
			t_BRIR_DataBase.clear();
			t_BRIR_partitioned.clear();

			//UPC algorithm init variables
			BRIRsubfilterLength_time = 2 * bufferSize;
			BRIRsubfilterLength_frequency = 2 * BRIRsubfilterLength_time;
			float temp_impulseResponseNumberOfBlocks = (float)BRIRLength / (float)bufferSize;
			BRIRNumOfSubfilters = static_cast<int>(std::ceil(temp_impulseResponseNumberOfBlocks));

			setupInProgress = true;
			BRIR_ready = false;
		}
		else
		{
			SET_RESULT(RESULT_ERROR_NULLPOINTER, "Error in BRIR Begin Setup: OwnerCore or ownerEnvironment are nullPtr");
		}
	}
	
	void CBRIR::AddBRIRTable(TBRIRTable && newTable)
	{
		t_BRIR_DataBase = newTable;
	}

	const TBRIRTable & CBRIR::GetRawBRIRTable() const
	{
		return t_BRIR_DataBase;
	}
			
	bool CBRIR::AddBRIR(VirtualSpeakerPosition vsPosition, Common::T_ear vsChannel, TImpulseResponse && newBRIR)
	{
		if (setupInProgress) 
		{
			auto returnValue = t_BRIR_DataBase.emplace(TVirtualSpeaker(vsPosition, vsChannel), std::forward<TImpulseResponse>(newBRIR));
			//Error handler
			if (returnValue.second) { /*SET_RESULT(RESULT_OK, "BRIR emplaced into t_BRIR_DataBase succesfully"); */
				return true;
			}
			else { SET_RESULT(RESULT_WARNING, "Error emplacing BRIR in t_BRIR_DataBase map");
			return false;
			}
		}
		else {
			SET_RESULT(RESULT_ERROR_NOTSET, "AddBRIR: It is not possible to Add a BRIR. Try to call BeginSetUp before addind a BRIR");
			return false;
		}
	}

	bool CBRIR::EndSetup()
	{
		if (!t_BRIR_DataBase.empty())
		{
			setupInProgress = false;
			BRIR_ready = true;

			//Setup values
#ifdef USE_FREQUENCY_COVOLUTION_WITHOUT_PARTITIONS_ANECHOIC
			t_BRIRFFT = CalculateBRIRFFT_Table();
#else
			t_BRIR_partitioned = CalculateBRIRFFT_Table_partitioned();
#endif 
			
			//Calculate ARIR table and set the convolution buffers
			if (!ownerEnvironment->SetABIR()) {
				return false;
			}

			//Free up memory
			//t_BRIR_DataBase.clear();

			SET_RESULT(RESULT_OK, "BRIR Matrix completed succesfully");
			return true;
		}
		else
		{
			// TO DO: Should be ASSERT?
			SET_RESULT(RESULT_ERROR_NOTSET, "The t_BRIR_DataBase map has not been set");
			return false;
		}		
	}
	
	void CBRIR::CalculateNewBRIRTable() {
		if (!t_BRIR_DataBase.empty()) {
			bufferSize = ownerEnvironment->GetCoreAudioState().bufferSize;;

			//Clear every table	
			t_BRIR_partitioned.clear();

			//UPC algorithm init variables
			BRIRsubfilterLength_time = 2 * bufferSize;
			BRIRsubfilterLength_frequency = 2 * BRIRsubfilterLength_time;
			float temp_impulseResponseNumberOfBlocks = (float)BRIRLength / (float)bufferSize;
			BRIRNumOfSubfilters = static_cast<int>(std::ceil(temp_impulseResponseNumberOfBlocks));

			setupInProgress = true;
			BRIR_ready = false;

			EndSetup();
		}
	}

	void CBRIR::Reset()
	{
		setupInProgress = false;
		BRIR_ready = false;

		//Clear every table	
		t_BRIR_DataBase.clear();
		t_BRIR_partitioned.clear();

		//Update parameters	
		BRIRLength = 0;
		BRIRLength_frequency = 0;
		bufferSize = 0;
	}
	bool CBRIR::IsIREmpty(const TImpulseResponse_Partitioned& in) {
		
		return in == emptyBRIR_partitioned;
	}

	const TImpulseResponse_Partitioned & CBRIR::GetBRIR_Partitioned(VirtualSpeakerPosition vsPos, Common::T_ear vsChannel) const
	{
		if (!setupInProgress) {
			auto it = t_BRIR_partitioned.find(TVirtualSpeaker(vsPos, vsChannel));
			if (it != t_BRIR_partitioned.end())
			{
				return it->second;
			}
			else
			{
				SET_RESULT(RESULT_ERROR_NOTSET, "GetBRIR_Partitioned: BRIR "+ std::to_string(vsPos) +" not found. Returning an empty BRIR");
				return emptyBRIR_partitioned;
			}
		}
		else {
			SET_RESULT(RESULT_WARNING, "GetBRIR_Partitioned return empty. Setup in progress");
			return emptyBRIR_partitioned;
		}
	}

	const TImpulseResponse & CBRIR::GetBRIR(VirtualSpeakerPosition vsPos, Common::T_ear vsChannel) const
	{
		if (!setupInProgress) {
			auto it = t_BRIR_DataBase.find(TVirtualSpeaker(vsPos, vsChannel));
			if (it != t_BRIR_DataBase.end())
			{
				return it->second;
			}
			else
			{
				SET_RESULT(RESULT_ERROR_NOTSET, "GetBRIR: BRIR not found. Returning an empty BRIR");
				return emptyBRIR;
			}
		}
		else {
			SET_RESULT(RESULT_WARNING, "GetBRIR return empty. Setup in progress");
			return emptyBRIR;
		}
	}

	int CBRIR::GetBRIRLength() {
		return BRIRLength;
	}

	int CBRIR::GetBRIRLength_frequency() {
		return BRIRLength_frequency;
	}

	int CBRIR::GetBRIROneSubfilterLength() {
		return BRIRsubfilterLength_frequency;
	}

	int CBRIR::GetBRIRNumberOfSubfilters() {
		return BRIRNumOfSubfilters;
	}

	bool CBRIR::IsBRIRready() {
		return BRIR_ready;
	}

	// PRIVATE METHODS
	

	TBRIRTable CBRIR::CalculateBRIRFFT_Table() {

		TBRIRTable newBRIR_Table;

		for (auto it = t_BRIR_DataBase.begin(); it != t_BRIR_DataBase.end(); it++)
		{
			auto returnValue = newBRIR_Table.emplace(it->first, std::forward<TImpulseResponse>(CalculateBRIRFFT(it->second)));
			//Error handler
			if (returnValue.second) { /*SET_RESULT(RESULT_OK, "BRIR emplaced into newBRIR_Table succesfully"); */ }
			else { SET_RESULT(RESULT_WARNING, "Error emplacing BRIR in newBRIR_Table map"); }
		}

		return newBRIR_Table;
	}

	TBRIRTablePartitioned CBRIR::CalculateBRIRFFT_Table_partitioned() {

		TBRIRTablePartitioned		newBRIR_Table_Partitioned;

		for (auto it = t_BRIR_DataBase.begin(); it != t_BRIR_DataBase.end(); it++) 
		{
			auto returnValue = newBRIR_Table_Partitioned.emplace(it->first, std::forward<TImpulseResponse_Partitioned>(CalculateBRIRFFT_partitioned(it->second)));
			//Error handler
			if (returnValue.second) { /*SET_RESULT(RESULT_OK, "BRIR emplaced into t_BRIR_partitioned succesfully"); */ }
			else { SET_RESULT(RESULT_WARNING, "Error emplacing BRIR in newBRIR_Table_Partitioned map"); }
		}

		return newBRIR_Table_Partitioned;	
	}

	TImpulseResponse_Partitioned CBRIR::CalculateBRIRFFT_partitioned(const TImpulseResponse & newData_time)
	{
		int blockSize = bufferSize;
		int numberOfBlocks = GetBRIRNumberOfSubfilters();

		TImpulseResponse_Partitioned new_DataFFT_Partitioned;
		new_DataFFT_Partitioned.reserve(numberOfBlocks);
		//Index to go throught the AIR values in time domain
		int index;
		for (int i = 0; i < newData_time.size(); i = i + blockSize)
		{
			CMonoBuffer<float> data_FFT_doubleSize;
			//Resize with double size and zeros to make the zero-padded demanded by the algorithm
			data_FFT_doubleSize.resize(blockSize * 2);
			//Fill each AIR block
			for (int j = 0; j < blockSize; j++) {
				index = i + j;
				if (index < newData_time.size()) {
					data_FFT_doubleSize[j] = newData_time[index];
				}
			}
			//FFT
			CMonoBuffer<float> data_FFT;
			Common::CFprocessor::CalculateFFT(data_FFT_doubleSize, data_FFT);
			//Prepare struct to return the value
			new_DataFFT_Partitioned.push_back(data_FFT);
		}
		return new_DataFFT_Partitioned;
	}

	TImpulseResponse CBRIR::CalculateBRIRFFT(const TImpulseResponse & newData_time)
	{
		CMonoBuffer<float> data_FFT_doubleSize;
		//Resize with double size and zeros to make the zero-padded demanded by the algorithm
		data_FFT_doubleSize.resize(BRIRLength * 2, 0.0f);
		//Fill each AIR block
		//for (int j = 0; j < BRIRLength; j++)
		//{
		//	data_FFT_doubleSize[j] = newData_time[j];
		//}
		data_FFT_doubleSize.SetFromCopy((CMonoBuffer<float>)newData_time);
		//FFT
		CMonoBuffer<float> data_FFT;
		Common::CFprocessor::CalculateFFT(data_FFT_doubleSize, data_FFT);
		return data_FFT_doubleSize;
	}

}//end namespace Binaural