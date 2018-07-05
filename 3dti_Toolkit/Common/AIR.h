/**
* \class CAIR
*
* \brief Declaration of CAIR interface.
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

#ifndef _CAIR_H_
#define _CAIR_H_

#include <unordered_map>
#include <cmath>  
#include <Common/Buffer.h>
#include <Common/Fprocessor.h>
#include <Common/ErrorHandler.h>
#include <Common/CommonDefinitions.h>

/*! \file */

// Type definitions

/** \brief Type definition for Impulse response 
*/
typedef CMonoBuffer<float> TImpulseResponse;

/** \brief Type definition for partitioned Impulse response 
*/
typedef CMonoBuffer<TImpulseResponse> TImpulseResponse_Partitioned;

/** \brief Type definition for bFormat channel specification
*/
enum TBFormatChannel {
	W = 0,				///<	Channel W
	X,					///<	Channel X
	Y,					///<	Channel Y
	Z,					///<	Channel Z
	NOMORECHANNELS
};

/** \brief Type definition for the data of one ambisonics channel. Contains the impulse responses of all virtual speakers
*/
template<class TVirtualSpeakerID>
using TBFormatChannelData = std::unordered_map<TVirtualSpeakerID, TImpulseResponse>;

/** \brief Type definition for the Ambisonics bFormat. Contains the channel data of all ambisonics channels
*/
template<class TVirtualSpeakerID>
using TBFormat = std::unordered_map<TBFormatChannel, TBFormatChannelData<TVirtualSpeakerID>>;

/** \brief Type definition for the data of one ambisonics channel. Contains the impulse responses of all virtual speakers
*/
template<class TVirtualSpeakerID>
using TBFormatChannelData_Partitioned = std::unordered_map<TVirtualSpeakerID, TImpulseResponse_Partitioned>;

/** \brief Type definition for the Ambisonics bFormat. Contains the channel data of all ambisonics channels
*/
template<class TVirtualSpeakerID>
using TBFormat_Partitioned = std::unordered_map<TBFormatChannel, TBFormatChannelData_Partitioned<TVirtualSpeakerID>>;


// Hash function to access one bformat channel
namespace std
{
	template<>
	struct hash<TBFormatChannel>
	{
		size_t operator()(const TBFormatChannel & key) const
		{
			return std::hash<int>()(static_cast<int>(key));
		}
	};
}

//////////////////////////////////////////////////////
//////////////////////////////////////////////////////

namespace Common
{
	/** \details This is a template class for storing the impulse responses needed for processing reverb using Virtual Ambisonics. */
	template <
		unsigned int nVirtualSpeakers,
		typename TVirtualSpeakerID
	>
		class CAIR
	{
		// METHODS
	public:

		/** \brief Default constructor.
		*/
		CAIR()
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
			bFormat.clear();
#else
			bFormat_Partitioned.clear();
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
		void AddImpulseResponse(TBFormatChannel channel, TVirtualSpeakerID speaker, const TImpulseResponse & newData)
		{
			ASSERT(setupDone, RESULT_ERROR_NOTSET, "The necessary parameters have not been set; you must call Setup before", "");
			ASSERT(channel < NOMORECHANNELS, RESULT_ERROR_OUTOFRANGE, "Trying to load AIR data for a bFormat channel of a higher order Ambisonic", "");
			ASSERT(newData.size() == impulseResponseLength, RESULT_ERROR_BADSIZE, "Size of impulse response does not agree with the one specified in the AIR setup", "");
			// TO DO: ASSERT for speaker ID
#ifdef USE_FREQUENCY_COVOLUTION_WITHOUT_PARTITIONS_REVERB 		
		// Make the FFT
			TImpulseResponse dataFFT;
			Common::CFprocessor::CalculateFFT(newData, dataFFT, inputSourceLength);

			// Emplace new channel data in current bFormat data
			auto it = bFormat.find(channel);
			if (it != bFormat.end())
			{
				TBFormatChannelData<TVirtualSpeakerID> existingChannelData = it->second;
				existingChannelData.emplace(speaker, std::move(dataFFT));
				bFormat.erase(channel);
				bFormat.emplace(channel, std::move(existingChannelData));
				//SET_RESULT(RESULT_OK, "Adding AIR to existing ambisonics channel succesfull");
			}
			else
			{
				TBFormatChannelData<TVirtualSpeakerID> channelData;
				channelData.emplace(speaker, std::move(dataFFT));
				bFormat.emplace(channel, std::move(channelData));
				//SET_RESULT(RESULT_OK, "Adding first AIR to a new ambisonics channel succesfull");
			}
#else
			TImpulseResponse_Partitioned dataFFT_Partitioned;
			dataFFT_Partitioned = Calculate_ARIRFFT_partitioned(newData);
			AddImpulseResponse(channel, speaker, std::move(dataFFT_Partitioned));
#endif
		}

		/** \brief Add partitioned impulse response for one BFormat channel on one virtual speaker
		*	\param [in] channel channel where data will be added 
		*	\param [in] speaker virtual speaker where data will be added 
		*	\param [in] dataFFT_Partitioned partitioned impulse response data to be added
		*   \eh On error, an error code is reported to the error handler.
		*/
		void AddImpulseResponse(TBFormatChannel channel, TVirtualSpeakerID speaker, const TImpulseResponse_Partitioned && dataFFT_Partitioned)
		{
			ASSERT(channel < NOMORECHANNELS, RESULT_ERROR_OUTOFRANGE, "Trying to load AIR data for a bFormat channel of a higher order Ambisonic", "");

			// Emplace new channel data in current bFormat_Partitioned data
			auto it = bFormat_Partitioned.find(channel);
			if (it != bFormat_Partitioned.end())
			{
				TBFormatChannelData_Partitioned<TVirtualSpeakerID> existingChannelData = it->second;
				existingChannelData.emplace(speaker, dataFFT_Partitioned);
				bFormat_Partitioned.erase(channel);
				bFormat_Partitioned.emplace(channel, std::move(existingChannelData));
				//SET_RESULT(RESULT_OK, "Adding AIR Partitioned to existing ambisonics channel succesfull");
			}
			else {
				TBFormatChannelData_Partitioned<TVirtualSpeakerID> channelData_Partitioned;
				channelData_Partitioned.emplace(speaker, dataFFT_Partitioned);
				bFormat_Partitioned.emplace(channel, std::move(channelData_Partitioned));
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
		const TBFormatChannelData<TVirtualSpeakerID>& GetChannelData(TBFormatChannel channel) const
		{
			// TO DO: check if data was added for this channel?

			auto it = bFormat.find(channel);
			if (it != bFormat.end())
			{
				SET_RESULT(RESULT_OK, "AIR returned correct channel data");
				return it->second; // return a const reference so the caller sees changes.		
			}
			else
			{
				ASSERT(false, RESULT_ERROR_OUTOFRANGE, "Trying to get AIR data from a bFormat channel of a higher order Ambisonic", "");
				return emptyChannelData; // returning an empty channel.
			}
		}

		//////////////////////////////////////////////////////

		/** \brief Get data from one BFormat_Partitioned channel
		*	\details In most cases, GetImpulseResponse_Partitioned should be used rather than this method.
		*	\param [in] channel channel from which data will be obtained.
		*	\retval channelData data from channel
		*	\pre Channel must be filled with data before calling this method
		*   \eh On error, an error code is reported to the error handler.
		*	\sa GetImpulseResponse_Partitioned, AddImpulseResponse
		*/
		const TBFormatChannelData_Partitioned <TVirtualSpeakerID>& GetChannelData_Partitioned(TBFormatChannel channel) const
		{
			// TO DO: check if data was added for this channel?

			auto it = bFormat_Partitioned.find(channel);
			if (it != bFormat_Partitioned.end())
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
		*/
		const TImpulseResponse& GetImpulseResponse(TBFormatChannel channel, TVirtualSpeakerID speaker) const
		{
			auto it = GetChannelData(channel).find(speaker);
			if (it != GetChannelData(channel).end())
			{
				SET_RESULT(RESULT_OK, "AIR returned correct impulse response for virtual speaker");
				return it->second;
			}
			else
			{
				ASSERT(false, RESULT_ERROR_OUTOFRANGE, "Trying to get Impulse Response data from an unknown virtual speaker", "");
				return emptyImpulseResponse;
			}
		}

		//////////////////////////////////////////////////////

		/** \brief Get impulse response partitioned (uniformely partitioned with bufferSize length) from one BFormat channel for one virtual speaker
		*	\param [in] channel channel from which data will be obtained 
		*	\param [in] speaker virtual speaker from which data will be obtained 
		*	\retval impulseResponse partitioned Impulse response data from speaker in channel
		*	\pre Data must be filled before calling this method 
		*   \eh On error, an error code is reported to the error handler.
		*	\sa AddImpulseResponse
		*/
		const TImpulseResponse_Partitioned & GetImpulseResponse_Partitioned(TBFormatChannel channel, TVirtualSpeakerID speaker) const
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
			bFormat.clear();
#else
			bFormat_Partitioned.clear();
#endif
		}

		/** \brief Returns true when the data is loaded and ready to be used
		*   \eh Nothing is reported to the error handler.*/
		bool IsInitialized() {
#ifdef USE_FREQUENCY_COVOLUTION_WITHOUT_PARTITIONS_REVERB 
			return impulseResponseLength != 0 && impulseResponseLength != 0 && setupDone && bFormat.size() > 0;
#else
			return impulseResponseLength != 0 && impulseResponseLength != 0 && setupDone && bFormat_Partitioned.size() > 0;
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

		TBFormat<TVirtualSpeakerID> bFormat;							// The actual data
		TBFormat_Partitioned<TVirtualSpeakerID> bFormat_Partitioned;	// The actual data partitioned and transformed (FFT)

		TBFormatChannelData<TVirtualSpeakerID> emptyChannelData;
		TImpulseResponse emptyImpulseResponse;
		TBFormatChannelData_Partitioned<TVirtualSpeakerID> emptyChannelData_Partitioned;
		TImpulseResponse_Partitioned emptyImpulseResponse_Partitioned;

		//METHODS

		TImpulseResponse_Partitioned Calculate_ARIRFFT_partitioned(const TImpulseResponse & newData_time)
		{
			int blockSize = inputSourceLength;
			int numberOfBlocks = GetDataNumberOfBlocks();

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
	};
}

//////////////////////////////////////////////////////
// INSTANCES: 
//////////////////////////////////////////////////////

//////////////////////////////////////////////////////
// ABIR //////////////////////////////////////////////

namespace std
{
	template<>
	struct hash<Common::T_ear>
	{
		size_t operator()(const Common::T_ear & key) const
		{
			return std::hash<int>()(static_cast<int>(key));
		}
	};
}

/** \brief
* Class storing Ambisonics Binaural Impulse Response data, for Binaural spatialization reverb
*	\details This is an instance of CAIR template
*/
using CABIR = Common::CAIR<2, Common::T_ear>;

/** \brief
* Instance of TBFormatChannelData for use with ABIR
*/
using TABIRChannelData = TBFormatChannelData<Common::T_ear>;

//////////////////////////////////////////////////////
// ARIR //////////////////////////////////////////////

/** \brief ID of binaural virtual speakers, for use with ARIR
*/
struct TLoudspeakersSpeakerID
{
	int azimuth;		///< Azimuth position of virtual speaker
	int elevation;		///< Elevation position of virtual speaker
	bool operator==(const TLoudspeakersSpeakerID& rhs) const
	{
		return ((azimuth == rhs.azimuth) && (elevation == rhs.elevation));
	}
	/*TLoudspeakersSpeakerID& operator=(const TLoudspeakersSpeakerID& a)
	{
		azimuth = a.azimuth;
		elevation = a.elevation;
		return *this;
	}*/
};

namespace std
{
	template<>
	struct hash<TLoudspeakersSpeakerID>
	{
		size_t operator()(const TLoudspeakersSpeakerID & key) const
		{
			//return std::hash<int>()(static_cast<int>(key.azimuth*1000 + key.elevation));
			return std::hash<int>()(static_cast<int>(key.azimuth)) ^ std::hash<int>()(static_cast<int>(key.elevation));
		}
	};
}

/** \brief
* Class storing Ambisonics Room Impulse Response data, for Loudspeakers spatialization reverb
*	\details This is an instance of CAIR template
*/
using CARIR = Common::CAIR<8, TLoudspeakersSpeakerID>;

/** \brief
* Instance of TBFormatChannelData for use with ARIR
*/
using TARIRChannelData = TBFormatChannelData<TLoudspeakersSpeakerID>;

#endif

