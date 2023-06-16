/**
* \class CHAIR
*
* \brief Declaration of CHAIR interface.
* \date	July 2016
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

#ifndef _CNAHRIR_H_
#define _CNAHRIR_H_

#include <unordered_map>
#include <cmath>  
#include <Common/Buffer.h>
#include <Common/Fprocessor.h>
#include <Common/ErrorHandler.h>
#include <Common/CommonDefinitions.h>
#include <Common/AIR.h>
#include <BinauralSpatializer/HRTF.h>

/*! \file */

/** \brief Type definition for the data of one ambisonics channel. Contains the impulse responses of all virtual speakers
*/
template<class TVirtualSpeakerID>
using TBHFormatChannelData = std::unordered_map<TVirtualSpeakerID, oneEarHRIR_struct>;

/** \brief Type definition for the Ambisonics. Contains the channel data of all ambisonics channels
*/
template<class TVirtualSpeakerID>
using TBHFormat = std::unordered_map<int, TBHFormatChannelData<TVirtualSpeakerID>>;

/** \brief Type definition for the data of one ambisonics channel. Contains the impulse responses of all virtual speakers
*/
template<class TVirtualSpeakerID>
using TBHFormatChannelData_Partitioned = std::unordered_map<TVirtualSpeakerID, TOneEarHRIRPartitionedStruct>;

/** \brief Type definition for the Ambisonics. Contains the channel data of all ambisonics channels
*/
template<class TVirtualSpeakerID>
using TBHFormat_Partitioned = std::unordered_map<int, TBHFormatChannelData_Partitioned<TVirtualSpeakerID>>;


// Hash function to access one bformat channel
//namespace std
//{
//	template<>
//	struct hash<int>
//	{
//		size_t operator()(const int & key) const
//		{
//			return std::hash<int>()(static_cast<int>(key));
//		}
//	};
//}

//////////////////////////////////////////////////////
//////////////////////////////////////////////////////

namespace Common
{
	/** \details This is a template class for storing the impulse responses needed for processing reverb using Virtual Ambisonics. */
	template <
		unsigned int nVirtualSpeakers,
		typename TVirtualSpeakerID
	>
		class CAHRIR
	{
		// METHODS
	public:

		/** \brief Default constructor.
		*/
		CAHRIR()
		{
			inputSourceLength = 0;
			impulseResponseLength = 0;
			setupDone = false;
		}

		/** \brief Setup input and AIR lengths
		*	\param [in] _inputSourceLength length of input buffer
		*	\param [in] _irLength length of impulse response
		*   \eh On error, an error code is reported to the error handler.
		*/
		void Setup(int _inputSourceLength, int _irLength)
		{
			ASSERT(nVirtualSpeakers > 0, RESULT_ERROR_BADSIZE, "Attempt to setup AIR for 0 virtual speakers", "");
			ASSERT(((_inputSourceLength > 0) & (_irLength > 0)), RESULT_ERROR_BADSIZE, "AIR and input source length must be greater than 0", "AIR setup successfull");

			inputSourceLength = _inputSourceLength;
			impulseResponseLength = _irLength;
			impulseResponseBlockLength_time = 2 * _inputSourceLength;
			impulseResponseBlockLength_freq = 2 * impulseResponseBlockLength_time;
			float temp_impulseResponseNumberOfBlocks = (float)impulseResponseLength / (float)inputSourceLength;
			impulseResponseNumberOfBlocks = static_cast<int>(std::ceil(temp_impulseResponseNumberOfBlocks));

#ifdef USE_FREQUENCY_COVOLUTION_WITHOUT_PARTITIONS_REVERB 
			ACNFormat.clear();
#else
			ACNFormat_Partitioned.clear();
#endif
			setupDone = true;				//Indicate that the setup has been done
		}

		//////////////////////////////////////////////////////

		/** \brief Add impulse response for one BFormat channel on one virtual speaker
		*	\param [in] channel channel where data will be added 
		*	\param [in] speaker virtual speaker where data will be added 
		*	\param [in] newData impulse response data to be added
		*   \eh On error, an error code is reported to the error handler.
		*/
		/*void AddImpulseResponse(int channel, TVirtualSpeakerID speaker, const oneEarHRIR_struct& newData)
		{
			ASSERT(setupDone, RESULT_ERROR_NOTSET, "The necessary parameters have not been set; you must call Setup before", "");
			ASSERT(newData.size() == impulseResponseLength, RESULT_ERROR_BADSIZE, "Size of impulse response does not agree with the one specified in the AIR setup", "");

			TOneEarHRIRPartitionedStruct dataFFT_Partitioned;
			dataFFT_Partitioned = Calculate_ARIRFFT_partitioned(newData);
			AddImpulseResponse(channel, speaker, std::move(dataFFT_Partitioned));

		}*/

		/** \brief Add partitioned impulse response for one BFormat channel on one virtual speaker
		*	\param [in] channel channel where data will be added 
		*	\param [in] speaker virtual speaker where data will be added 
		*	\param [in] dataFFT_Partitioned partitioned impulse response data to be added
		*   \eh On error, an error code is reported to the error handler.
		*/
		void AddImpulseResponse(int channel, TVirtualSpeakerID speaker, const TOneEarHRIRPartitionedStruct&& dataFFT_Partitioned)
		{
			// Emplace new channel data in current bFormat_Partitioned data
			auto it = ACNFormat_Partitioned.find(channel);
			if (it != ACNFormat_Partitioned.end())
			{
				TBHFormatChannelData_Partitioned<TVirtualSpeakerID> existingChannelData = it->second;
				existingChannelData.emplace(speaker, dataFFT_Partitioned);
				ACNFormat_Partitioned.erase(channel);
				ACNFormat_Partitioned.emplace(channel, std::move(existingChannelData));
				//SET_RESULT(RESULT_OK, "Adding AIR Partitioned to existing ambisonics channel succesfull");
			}
			else {
				TBHFormatChannelData_Partitioned<TVirtualSpeakerID> channelData_Partitioned;
				channelData_Partitioned.emplace(speaker, dataFFT_Partitioned);
				ACNFormat_Partitioned.emplace(channel, std::move(channelData_Partitioned));
				//SET_RESULT(RESULT_OK, "Adding first AIR partitioned to a new ambisonics channel succesfull");
			}
		}

		//////////////////////////////////////////////////////

		/** \brief Get data from one BFormat channel
		*	\details In most cases, GetImpulseResponse should be used rather than this method.
		*	\param [in] channel channel from which data will be obtained 
		*	\retval channelData data from channel
		*	\pre Channel must be filled with data before calling this method 
		*   \eh On success, RESULT_OK is reported to the error handler.
		*       On error, an error code is reported to the error handler.
		*	\sa GetImpulseResponse, AddImpulseResponse
		*/
		//const TBHFormatChannelData<TVirtualSpeakerID>& GetChannelData(int channel) const
		//{
		//	// TO DO: check if data was added for this channel?

		//	auto it = ACNFormat.find(channel);
		//	if (it != ACNFormat.end())
		//	{
		//		SET_RESULT(RESULT_OK, "AIR returned correct channel data");
		//		return it->second; // return a const reference so the caller sees changes.		
		//	}
		//	else
		//	{
		//		ASSERT(false, RESULT_ERROR_OUTOFRANGE, "Trying to get AIR data from a bFormat channel of a higher order Ambisonic", "");
		//		return emptyChannelData; // returning an empty channel.
		//	}
		//}

		//////////////////////////////////////////////////////

		/** \brief Get data from one BFormat_Partitioned channel
		*	\details In most cases, GetImpulseResponse_Partitioned should be used rather than this method.
		*	\param [in] channel channel from which data will be obtained.
		*	\retval channelData data from channel
		*	\pre Channel must be filled with data before calling this method
		*   \eh On error, an error code is reported to the error handler.
		*	\sa GetImpulseResponse_Partitioned, AddImpulseResponse
		*/
		const TBHFormatChannelData_Partitioned <TVirtualSpeakerID>& GetChannelData_Partitioned(int channel) const
		{
			// TO DO: check if data was added for this channel?

			auto it = ACNFormat_Partitioned.find(channel);
			if (it != ACNFormat_Partitioned.end())
			{
				//SET_RESULT(RESULT_OK, "AIR returned correct channel data");
				return it->second; // return a const reference so the caller sees changes.		
			}
			else
			{
				ASSERT( false, RESULT_ERROR_OUTOFRANGE, "Trying to get AIR data from a bFormat_Partitioned channel of a higher order Ambisonic", "");
				return emptyChannelData_Partitioned; // returning an empty channel.
			}
		}

		//////////////////////////////////////////////////////

		/** \brief Get impulse response from one BFormat channel for one virtual speaker
		*	\param [in] channel channel from which data will be obtained 
		*	\param [in] speaker virtual speaker from which data will be obtained 
		*	\retval impulseResponse Impulse response data from speaker in channel
		*	\pre Data must be filled before calling this method 
		*   \eh On error, an error code is reported to the error handler.
		*	\sa AddImpulseResponse
		//*/
		//const oneEarHRIR_struct& GetImpulseResponse(int channel, TVirtualSpeakerID speaker) const
		//{
		//	auto it = GetChannelData(channel).find(speaker);
		//	if (it != GetChannelData(channel).end())
		//	{
		//		SET_RESULT(RESULT_OK, "AIR returned correct impulse response for virtual speaker");
		//		return it->second;
		//	}
		//	else
		//	{
		//		ASSERT(false, RESULT_ERROR_OUTOFRANGE, "Trying to get Impulse Response data from an unknown virtual speaker", "");
		//		return emptyImpulseResponse;
		//	}
		//}

		//////////////////////////////////////////////////////

		/** \brief Get impulse response partitioned (uniformely partitioned with bufferSize length) from one BFormat channel for one virtual speaker
		*	\param [in] channel channel from which data will be obtained 
		*	\param [in] speaker virtual speaker from which data will be obtained 
		*	\retval impulseResponse partitioned Impulse response data from speaker in channel
		*	\pre Data must be filled before calling this method 
		*   \eh On error, an error code is reported to the error handler.
		*	\sa AddImpulseResponse
		*/
		const TOneEarHRIRPartitionedStruct& GetImpulseResponse_Partitioned(int channel, TVirtualSpeakerID speaker) const
		{
			auto it = GetChannelData_Partitioned(channel).find(speaker);
			if (it != GetChannelData_Partitioned(channel).end())
			{
				//SET_RESULT(RESULT_OK, "AIR returned correct impulse response for virtual speaker");
				return it->second;
			}
			else
			{
				ASSERT(false, RESULT_ERROR_OUTOFRANGE, "Trying to get Impulse Response data from an unknown virtual speaker", "");
				return emptyImpulseResponse_Partitioned;
			}
		}

		//////////////////////////////////////////////////////


		/** \brief Get data length of stored impulse responses
		*	\retval dataLength Impulse response buffer size
		*	\pre Impulse response length must be setup
		*	\sa SetupIFFT_OLA
		*   \eh Nothing is reported to the error handler.
		*/
		const int GetDataLength() const
		{
			//ASSERT(impulseResponseLength > 0, RESULT_ERROR_NOTINITIALIZED, "Data length of AIR has not been defined; have you loaded any AIR data?", "AIR length returned succesfully");
			return impulseResponseLength;
		}

		/** \brief Get data length of a stored subfilter impulse response in time domain (the length of one partition, which is the same for every partition)
		*	\retval dataLength Impulse response partition buffer size (time domain)
		*	\pre Impulse response length must be setup 
		*	\sa SetupIFFT_OLA
		*   \eh Nothing is reported to the error handler.
		*/
		const int GetDataBlockLength() const {
			return impulseResponseBlockLength_time;
		}

		/** \brief Get data length of stored impulse responses in frequency domain (the length of one partition, which is the same for every partition)
		*	\retval dataLength Impulse response FFT partition buffer size (frequency domain)
		*	\pre Impulse response length must be setup 
		*	\sa SetupIFFT_OLA
		*   \eh Nothing is reported to the error handler.
		*/
		const int GetDataBlockLength_freq() const {
			return impulseResponseBlockLength_freq;
		}

		/** \brief Get number of sub-filters (blocks) fo the AIR partition
		*	\retval dataLength Number of sub filters
		*	\pre Impulse response length must be setup 
		*	\sa SetupIFFT_OLA
		*   \eh Nothing is reported to the error handler.
		*/
		const int GetDataNumberOfBlocks() const {
			return impulseResponseNumberOfBlocks;
		}

		/** \brief Set to initial state
		*   \eh Nothing is reported to the error handler.*/
		void Reset() {
			inputSourceLength = 0;
			impulseResponseLength = 0;
			setupDone = false;
#ifdef USE_FREQUENCY_COVOLUTION_WITHOUT_PARTITIONS_REVERB 
			ACNFormat.clear();
#else
			ACNFormat_Partitioned.clear();
#endif
		}

		/** \brief Returns true when the data is loaded and ready to be used
		*   \eh Nothing is reported to the error handler.*/
		bool IsInitialized() {
#ifdef USE_FREQUENCY_COVOLUTION_WITHOUT_PARTITIONS_REVERB 
			return impulseResponseLength != 0 && impulseResponseLength != 0 && setupDone && ACNFormat.size() > 0;
#else
			return impulseResponseLength != 0 && impulseResponseLength != 0 && setupDone && ACNFormat_Partitioned.size() > 0;
#endif
		}

		//////////////////////////////////////////////////////

	private:
		bool setupDone;							//To know if the setup has been done
		int impulseResponseLength;				// Data length of one channel for one virtual speaker
		int impulseResponseBlockLength_freq;	// Data length of one block of the FFT partitioned impulse response in time domain
		int impulseResponseBlockLength_time;	// Data length of one block of the FFT partitioned impulse response in frequency domain
		int inputSourceLength;					// Input source length		
		int impulseResponseNumberOfBlocks;		// Number of blocks of the partitioned IR 

		//TBHFormat<TVirtualSpeakerID> ACNFormat;							// The actual data
		TBHFormat_Partitioned<TVirtualSpeakerID> ACNFormat_Partitioned;	// The actual data partitioned and transformed (FFT)

		//TBHFormatChannelData<TVirtualSpeakerID> emptyChannelData;
		//oneEarHRIR_struct emptyImpulseResponse;
		TBHFormatChannelData_Partitioned<TVirtualSpeakerID> emptyChannelData_Partitioned;
		TOneEarHRIRPartitionedStruct emptyImpulseResponse_Partitioned;

		//METHODS

		//TOneEarHRIRPartitionedStruct Calculate_ARIRFFT_partitioned(const oneEarHRIR_struct& newData_time)
		//{
		//	int blockSize = inputSourceLength;
		//	int numberOfBlocks = GetDataNumberOfBlocks();

		//	TOneEarHRIRPartitionedStruct new_DataFFT_Partitioned;
		//	new_DataFFT_Partitioned.HRIR_Partitioned.reserve(numberOfBlocks);
		//	//Index to go throught the AIR values in time domain
		//	int index;
		//	for (int i = 0; i < newData_time.size(); i = i + blockSize)
		//	{
		//		CMonoBuffer<float> data_FFT_doubleSize;
		//		//Resize with double size and zeros to make the zero-padded demanded by the algorithm
		//		data_FFT_doubleSize.resize(blockSize * 2);
		//		//Fill each AIR block
		//		for (int j = 0; j < blockSize; j++) {
		//			index = i + j;
		//			if (index < newData_time.size()) {
		//				data_FFT_doubleSize[j] = newData_time[index];
		//			}
		//		}
		//		//FFT
		//		CMonoBuffer<float> data_FFT;
		//		Common::CFprocessor::CalculateFFT(data_FFT_doubleSize, data_FFT);
		//		//Prepare struct to return the value
		//		new_DataFFT_Partitioned.push_back(data_FFT);
		//	}
		//	return new_DataFFT_Partitioned;
		//}
	};
}

//////////////////////////////////////////////////////
// INSTANCES: 
//////////////////////////////////////////////////////

//////////////////////////////////////////////////////
// ABIR //////////////////////////////////////////////

//namespace std
//{
//	template<>
//	struct hash<Common::T_ear>
//	{
//		size_t operator()(const Common::T_ear & key) const
//		{
//			return std::hash<int>()(static_cast<int>(key));
//		}
//	};
//}

/** \brief
* Class storing Ambisonics Binaural Impulse Response data, for Binaural spatialization reverb
*	\details This is an instance of CAIR template
*/
using CAHRBIR = Common::CAHRIR<2, Common::T_ear>;

/** \brief
* Instance of TBFormatChannelData for use with ABIR
*/
using THABIRChannelData = TBHFormatChannelData<Common::T_ear>;

//////////////////////////////////////////////////////
// ARIR //////////////////////////////////////////////

/** \brief ID of binaural virtual speakers, for use with ARIR
*/
//struct TLoudspeakersSpeakerID
//{
//	int azimuth;		///< Azimuth position of virtual speaker
//	int elevation;		///< Elevation position of virtual speaker
//	bool operator==(const TLoudspeakersSpeakerID& rhs) const
//	{
//		return ((azimuth == rhs.azimuth) && (elevation == rhs.elevation));
//	}
//	/*TLoudspeakersSpeakerID& operator=(const TLoudspeakersSpeakerID& a)
//	{
//		azimuth = a.azimuth;
//		elevation = a.elevation;
//		return *this;
//	}*/
//};

//namespace std
//{
//	template<>
//	struct hash<TLoudspeakersSpeakerID>
//	{
//		size_t operator()(const TLoudspeakersSpeakerID & key) const
//		{
//			//return std::hash<int>()(static_cast<int>(key.azimuth*1000 + key.elevation));
//			return std::hash<int>()(static_cast<int>(key.azimuth)) ^ std::hash<int>()(static_cast<int>(key.elevation));
//		}
//	};
//}

/** \brief
* Instance of TBFormatChannelData for use with ARIR
*/
using THARIRChannelData = TBHFormatChannelData<TLoudspeakersSpeakerID>;

#endif

