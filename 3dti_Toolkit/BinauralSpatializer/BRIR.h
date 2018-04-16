/**
* \class CBRIR
*
* \brief Declaration of CBRIR interface.
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

#ifndef _CBRIR_H_
#define _CBRIR_H_

#include <unordered_map>
#include <math.h>  
#include <BinauralSpatializer/Environment.h>
#include <Common/Buffer.h>
#include <Common/Fprocessor.h>
#include <Common/ErrorHandler.h>
#include <Common/CommonDefinitions.h>

/** \brief Type definition for virtual speakers in BRIR
*/
struct TVirtualSpeaker
{
	VirtualSpeakerPosition vsPosition;	///< Position of virtual speaker
	Common::T_ear	vsChannel;			///< Channel (left or right)
	
	TVirtualSpeaker(VirtualSpeakerPosition _vsPosition, Common::T_ear	_vsChannel) :vsPosition{ _vsPosition }, vsChannel{ _vsChannel } {}
	TVirtualSpeaker() :TVirtualSpeaker{ NORTH, Common::T_ear::LEFT } {}
	
	bool operator==(const TVirtualSpeaker& oth) const
	{
		return ((this->vsPosition == oth.vsPosition) && (this->vsChannel == oth.vsChannel));
	}
};

namespace std
{
	//[TBC]
	template<>
	struct hash<TVirtualSpeaker>
	{
		// adapted from http://en.cppreference.com/w/cpp/utility/hash
		size_t operator()(const TVirtualSpeaker & key) const
		{
			size_t h1 = std::hash<int32_t>()(key.vsPosition);
			size_t h2 = std::hash<int32_t>()(key.vsChannel);
			return h1 ^ (h2 << 1);  // exclusive or of hash functions for each int.
		}
	};
}

/** \brief Type definition for impulse response
*/
typedef CMonoBuffer<float> TImpulseResponse;

/** \brief Type definition for partitioned impulse response 
*/
typedef CMonoBuffer<TImpulseResponse> TImpulseResponse_Partitioned;

/** \brief Type definition for the HRTF table
*/
typedef std::unordered_map<TVirtualSpeaker, TImpulseResponse> TBRIRTable;

/** \brief Type definition for the HRTF table
*/
typedef std::unordered_map<TVirtualSpeaker, TImpulseResponse_Partitioned> TBRIRTablePartitioned;

namespace Binaural 
{
	class CEnvironment;
	/** \details This class gets impulse response data to compose BRIRs (Binaural Room Impulse Responses).
	*/
	class CBRIR
	{
		// METHODS
	public:
		//CBRIR();

		/**	\brief Constructor from already created environment object.
		*	\details Sets the BRIR data as not ready
		*	\param [in] _ownerEnvironment pointer to environment object
		*   \eh Nothing is reported to the error handler.
		*/
		CBRIR(CEnvironment* _ownerEnvironment) :ownerEnvironment{ _ownerEnvironment }, BRIR_ready{false} {}

		/**	\brief Default constructor 
		*	\details Sets the BRIR data as not ready and the environment object as a null pointer
		*   \eh Nothing is reported to the error handler.
		*/
		CBRIR() :ownerEnvironment{ nullptr }, BRIR_ready{ false } {}

		/**	\brief Start a new BRIR configuration.
		*	\param [in] _BRIRLength integer that indicates the BRIR length
		*
		* When this method is called: (1) Reset the previous BRIR, (2) Block the spatialization algorithm. 
		*   \eh On error, an error code is reported to the error handler.
		*/
		void BeginSetup(int32_t _BRIRLength);		
		
		/** \brief Set a new BRIR vector in the BRIR matrix
		*	\param [in] vsPosition Virtual Speaker position (N,S,E,W)
		*	\param [in] vsChannel Virtual Speaker Channel (left, right)
		*	\param [in] newBRIR BRIR vector value to add to the BRIR matrix
		*	\retval boolean to indicate if BRIR has been added correctly
		*   \eh On error, an error code is reported to the error handler.
		        Warnings may be reported to the error handler
		*/
		bool AddBRIR(VirtualSpeakerPosition vsPosition, Common::T_ear vsChannel, TImpulseResponse && newBRIR);
		
		/** \brief Set the full BRIR matrix.
		*	\param [in] newTable full table with all BRIR data
		*   \eh Nothing is reported to the error handler.
		*/
		void AddBRIRTable(TBRIRTable && newTable);

		/** \brief Stop the BRIR configuration and set the ABIR configuration		
		*   \retval boolean to indicate if setup was successful
		*   \eh On success, RESULT_OK is reported to the error handler.
		*       On error, an error code is reported to the error handler.
		*/
		bool EndSetup();

		/** \brief Get BRIR filter length in time domain
		*   \retval length int BRIR filter length in time domain
		*   \eh Nothing is reported to the error handler.
		*/
		int GetBRIRLength();

		/** \brief Get BRIR filter length in frequency domain
		*   \retval length BRIR filter length in frequency domain
		*   \eh Nothing is reported to the error handler.
		*/
		int GetBRIRLength_frequency();

		/** \brief Get BRIR sub-filter (after partition) length in frequency domain
		*   \retval length BRIR sub-filter length in frequency domain
		*   \eh Nothing is reported to the error handler.
		*/
		int GetBRIROneSubfilterLength();

		/** \brief Get number of sub-filters (blocks) fo the brir partition
		*	\retval dataLength Number of sub filters
		*	\pre Impulse response length must be setup 
		*	\sa SetupIFFT_OLA 
		*   \eh Nothing is reported to the error handler.
		*/
		int GetBRIRNumberOfSubfilters();

		/** \brief Indicates if the BRIR has been set
		*	\retval isReady Boolean to indicate if the BRIR table has been completed
		*   \eh Nothing is reported to the error handler.
		*/
		bool IsBRIRready();
		
		/** \brief Get one partitioned BRIR for one ear and one virtual speaker position
		*	\param [in] vsPos virtual speaker position
		*	\param [in] vsChannel virtual speaker channel (ear)
		*	\retval impulseResponse partitioned BRIR
		*   \eh On error, an error code is reported to the error handler.
		*       Warnings may be reported to the error handler.
		*/
		const TImpulseResponse_Partitioned & GetBRIR_Partitioned(VirtualSpeakerPosition vsPos, Common::T_ear vsChannel) const;

		/** \brief Get BRIR one BRIR for one ear and one virtual speaker position
		*	\param [in] vsPos virtual speaker position
		*	\param [in] vsChannel virtual speaker channel (ear)
		*	\retval impulseResponse BRIR 
		*   \eh On error, an error code is reported to the error handler.
		*       Warnings may be reported to the error handler.
		*/
		const TImpulseResponse & GetBRIR(VirtualSpeakerPosition vsPos, Common::T_ear vsChannel) const;

		/** \brief Get raw BRIR table
		*	\retval table raw BRIR table
		*   \eh Nothing is reported to the error handler.
		*/
		const TBRIRTable & GetRawBRIRTable() const;

		/** \brief Get raw BRIR table partitioned
		*	\retval table raw BRIR table partitioned
		*/
		//const TBRIRTablePartitioned & GetRawBRIRTablePartitioned() const;
		
		bool IsIREmpty(const TImpulseResponse_Partitioned& in);

	private:

		////////////
		//METHODS
		///////////		
		TImpulseResponse_Partitioned	CalculateBRIRFFT_partitioned(const TImpulseResponse & newData_time);
		TImpulseResponse				CalculateBRIRFFT(const TImpulseResponse & newData_time);
		TBRIRTable					CalculateBRIRFFT_Table();
		TBRIRTablePartitioned		CalculateBRIRFFT_Table_partitioned();
		
		// Recalculate the BRIR FFT table partitioned or not with a new bufferSize
		void CalculateNewBRIRTable();

		// Reset BRIR table
		void Reset();

		///////////////
		// ATTRIBUTES
		///////////////
		CEnvironment* ownerEnvironment;	// Pointer to the Environment
		int32_t BRIRLength;				// BRIR vector length
		int32_t BRIRLength_frequency;	// BRIR length in frequency domain 
		int32_t bufferSize;				// Input signal buffer size
		bool setupInProgress;			// Variable that indicates the BRIR add is in progress
		bool BRIR_ready;				// Indicate that the BRIR table is ready in order to create the ABIR table
		TBRIRTable t_BRIR_DataBase;			// Table to store the BRIR data
		TBRIRTable t_BRIRFFT;		// Table to store the BRIR data in frequency domain
		TBRIRTablePartitioned t_BRIR_partitioned; // Table to contain the BRIR data partitioned and in frequency domain

		int BRIRsubfilterLength_time;		// BRIR subfilter in time domain buffer size 
		int BRIRsubfilterLength_frequency;	// BRIR subfilter in frequency domain buffer size 
		int BRIRNumOfSubfilters;			// Number of subfilters of the partitioned BRIR

		//empty variables
		TImpulseResponse_Partitioned emptyBRIR_partitioned;
		TImpulseResponse emptyBRIR;

		friend class CEnvironment;
	};
}//end namespace Binaural 
#endif