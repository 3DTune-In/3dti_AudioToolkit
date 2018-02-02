/**
* \class CHRTF
*
* \brief Declaration of CHRTF class interface
* \version 
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


#ifndef _CHRTF_H_
#define _CHRTF_H_

#include <unordered_map>
#include <vector>
#include <utility>
#include <list>
#include <cstdint>
#include <BinauralSpatializer/Listener.h>
#include <Common/Buffer.h>
#include <Common/ErrorHandler.h>
#include <Common/Fprocessor.h>
#include <Common/Magnitudes.h>
#include <Common/CommonDefinitions.h>


#ifndef PI 
#define PI 3.14159265
#endif
#ifndef DEFAULT_RESAMPLING_STEP
#define DEFAULT_RESAMPLING_STEP 5
#endif

#define MAX_DISTANCE_BETWEEN_ELEVATIONS 5
#define NUMBER_OF_PARTS 4 
#define AZIMUTH_STEP  15
//#define EPSILON 0.01f;

/*! \file */

// Structs and types definitions 

/** \brief Defines and holds data to work with orientations
*/
struct orientation
{
	int32_t azimuth;	///< Azimuth angle in degrees
	int32_t elevation;	///< Elevation angle in degrees
    orientation(int32_t _azimuth, int32_t _elevation):azimuth{_azimuth}, elevation{_elevation}{}
    orientation():orientation{0,0}{}
    bool operator==(const orientation& oth) const
    {
        return ((this->azimuth == oth.azimuth) && (this->elevation == oth.elevation));
    }
};

/** \brief Type definition for a left-right pair of impulse response with the ITD removed and stored in a specific struct field
*/
struct HRIR_struct {
	uint64_t leftDelay;				///< Left delay, in number of samples
	uint64_t rightDelay;			///< Right delay, in number of samples
	CMonoBuffer<float> leftHRIR;	///< Left impulse response data
	CMonoBuffer<float> rightHRIR;	///< Right impulse response data
};

/** \brief Type definition for a left-right pair of impulse response subfilter set with the ITD removed and stored in a specific struct field
*/
struct HRIR_Partitioned_struct {
	uint64_t leftDelay;				///< Left delay, in number of samples
	uint64_t rightDelay;			///< Right delay, in number of samples
	std::vector<CMonoBuffer<float>> leftHRIR_Partitioned;	///< Left partitioned impulse response data
	std::vector<CMonoBuffer<float>> rightHRIR_Partitioned;	///< Right partitioned impulse response data
};

/** \brief Type definition for an impulse response with the ITD removed and stored in a specific struct field
*/
struct oneEarHRIR_struct {
	uint64_t delay;				///< Delay, in number of samples
	CMonoBuffer<float> HRIR;	///< Impulse response data
};

/** \brief Type definition for an impulse response subfilter set with the ITD removed and stored in a specific struct field
*/
struct oneEarHRIR_Partitioned_struct {
	std::vector<CMonoBuffer<float>> HRIR_Partitioned;	///< Partitioned impulse response data
	uint64_t delay;				///< Delay, in number of samples
};

/**	\brief Type definition for barycentric coordinates
*/
struct barycentricCoordinates_struct {
	float alpha;	///< Coordinate alpha
	float beta;		///< Coordinate beta
	float gamma;	///< Coordinate gamma
};

namespace std
{
	//[TBC]
    template<>
    struct hash<orientation>
    {
        // adapted from http://en.cppreference.com/w/cpp/utility/hash
        size_t operator()(const orientation & key) const
        {
            size_t h1 = std::hash<int32_t>()(key.azimuth);
            size_t h2 = std::hash<int32_t>()(key.elevation);
            return h1 ^ (h2 << 1);  // exclusive or of hash functions for each int.
        }
    };
}


/** \brief Type definition for the HRTF table
*/
typedef std::unordered_map<orientation, HRIR_struct> T_HRTFTable;

/** \brief Type definition for the HRTF partitioned table used when UPConvolution is activated
*/
typedef std::unordered_map<orientation, HRIR_Partitioned_struct> T_HRTFPartitionedTable;

/** \brief Type definition for a distance-orientation pair
*/
typedef std::pair <float, orientation> T_PairDistanceOrientation;

namespace Binaural 
{
	class CCore;
	class CListener;

	/** \details This class gets impulse response data to compose HRTFs and implements different algorithms to interpolate the HRIR functions.
	*/
	class CHRTF
	{
	public:
		
		/** \brief Constructor with parameters
		*	\param [in] _ownerListener pointer to already created listener object
		*	\details By default, customized ITD is switched off and resampling step is set to 5 degrees
		*   \eh Nothing is reported to the error handler.
		*/
		CHRTF(CListener* _ownerListener) 	
			:ownerListener{ _ownerListener }, enableCustomizedITD{ false }, resamplingStep{ DEFAULT_RESAMPLING_STEP }, HRIRlength{ 0 }, HRTFLoaded{ false }, setupInProgress{ false }
		{}

		/** \brief Default Constructor
		*	\details By default, customized ITD is switched off, resampling step is set to 5 degrees and listener is a null pointer
		*   \eh Nothing is reported to the error handler.
		*/
		CHRTF()
			:ownerListener{ nullptr }, enableCustomizedITD{ false }, resamplingStep{ DEFAULT_RESAMPLING_STEP }, HRIRlength{ 0 }, HRTFLoaded{ false }, setupInProgress{ false }
		{}

		/** \brief Get size of each HRIR buffer
		*	\retval size number of samples of each HRIR buffer for one ear
		*   \eh Nothing is reported to the error handler.
		*/
		int32_t GetHRIRLength() const
		{
			return HRIRlength;
		}

		/** \brief Start a new HRTF configuration
		*	\param [in] _HRIRlength buffer size of the HRIR to be added		
		*   \eh On success, RESULT_OK is reported to the error handler.
		*       On error, an error code is reported to the error handler.
		*/
		void BeginSetup(int32_t _HRIRlength);
		
		/** \brief Set the full HRIR matrix.
		*	\param [in] newTable full table with all HRIR data
		*   \eh Nothing is reported to the error handler.
		*/
		void AddHRTFTable(T_HRTFTable && newTable);

		/** \brief Add a new HRIR to the HRTF table
		*	\param [in] azimuth azimuth angle in degrees
		*	\param [in] elevation elevation angle in degrees
		*	\param [in] newHRIR HRIR data for both ears
		*   \eh Warnings may be reported to the error handler.
		*/
		void AddHRIR(float azimuth, float elevation, HRIR_struct && newHRIR);

		/** \brief Stop the HRTF configuration		
		*   \eh On success, RESULT_OK is reported to the error handler.
		*       On error, an error code is reported to the error handler.
		*/
		void EndSetup();

		/** \brief Switch on ITD customization in accordance with the listener head radius
		*   \eh Nothing is reported to the error handler.
		*/
		void EnableHRTFCustomizedITD();

		/** \brief Switch off ITD customization in accordance with the listener head radius
		*   \eh Nothing is reported to the error handler.
		*/
		void DisableHRTFCustomizedITD();

		/** \brief Get the flag for HRTF cutomized ITD process
		*	\retval HRTFCustomizedITD if true, the HRTF ITD customization process based on the head circumference is enabled
		*   \eh Nothing is reported to the error handler.
		*/
		bool IsHRTFCustomizedITDEnabled();

		/** \brief Get interpolated HRIR buffer with Delay, for one ear
		*	\param [in] ear for which ear we want to get the HRIR 
		*	\param [in] azimuth azimuth angle in degrees
		*	\param [in] elevation elevation angle in degrees
		*	\param [in] runTimeInterpolation switch run-time interpolation
		*	\retval HRIR interpolated buffer with delay for specified ear
		*   \eh On error, an error code is reported to the error handler.
		*       Warnings may be reported to the error handler.
		*/
		const oneEarHRIR_struct GetHRIR_frequency(Common::T_ear ear, float azimuth, float elevation, bool runTimeInterpolation) const;

		/** \brief Get interpolated and partitioned HRIR buffer with Delay, for one ear
		*	\param [in] ear for which ear we want to get the HRIR
		*	\param [in] azimuth azimuth angle in degrees
		*	\param [in] elevation elevation angle in degrees
		*	\param [in] runTimeInterpolation switch run-time interpolation
		*	\retval HRIR interpolated buffer with delay for specified ear
		*   \eh On error, an error code is reported to the error handler.
		*       Warnings may be reported to the error handler.
		*/
		const oneEarHRIR_Partitioned_struct GetHRIR_partitioned(Common::T_ear ear, float azimuth, float elevation, bool runTimeInterpolation) const;

		/** \brief	Get the number of subfilters (blocks) in which the HRIR has been partitioned
		*	\retval n Number of HRIR subfilters
		*   \eh Nothing is reported to the error handler.
		*/
		const int32_t GetHRIRNumberOfSubfilters() const;

		/** \brief	Get the size of subfilters (blocks) in which the HRIR has been partitioned, every subfilter has the same size
		*	\retval size Size of HRIR subfilters
		*   \eh Nothing is reported to the error handler.
		*/
		const int32_t GetHRIRSubfilterLength() const;

		/** \brief	Get if the HRTF has been loaded
		*	\retval isLoadead bool var that is true if the HRTF has been loaded
		*   \eh Nothing is reported to the error handler.
		*/
		bool IsHRTFLoaded();

		/** \brief Get raw HRTF table
		*	\retval table raw HRTF table
		*   \eh Nothing is reported to the error handler.
		*/
		const T_HRTFTable & GetRawHRTFTable() const;

		/** \brief	Calculate the ITD value for a specific source
		*   \param [in]	_azimuth		source azimuth in degrees
		*   \param [in]	_elevation		source elevation in degrees
		*   \param [in]	ear				ear where the ITD is calculated (RIGHT, LEFT)
		*   \return ITD ITD calculated with the current listener head circunference
		*   \eh Nothing is reported to the error handler.
		*/
		const unsigned long GetCustomizedDelay(float _azimuth, float _elevation, Common::T_ear ear)const;

	private:
		///////////////
		// ATTRIBUTES
		///////////////
		CListener* ownerListener;						// owner Listener
		int32_t HRIRlength;								// HRIR vector length
		int32_t bufferSize;								// Input signal buffer size
		//int32_t sampleRate;							// Sample Rate		
		int32_t HRIR_partitioned_NumberOfSubfilters;	// Number of subfilters (blocks) for the UPC algorithm
		int32_t HRIR_partitioned_SubfilterLength;		// Size of one HRIR subfilter

		float sphereBorder;						// Define spheere "sewing"
		float epsilon_sewing = 0.001f;

		bool setupInProgress;						// Variable that indicates the HRTF add and resample algorithm are in process
		bool HRTFLoaded;							// Variable that indicates if the HRTF has been loaded correctly
		bool bInterpolatedResampleTable;			// If true: calculate the HRTF resample matrix with interpolation
		int resamplingStep; 						// HRTF Resample table step (azimuth and elevation)
		bool enableCustomizedITD;					// Indicate the use of a customized delay



		// HRTF tables			
		T_HRTFTable				t_HRTF_DataBase;
		T_HRTFTable				t_HRTF_Resampled_frequency;
		T_HRTFPartitionedTable	t_HRTF_Resampled_partitioned;

		// Empty object to return in some methods
		HRIR_struct						emptyHRIR;
		HRIR_Partitioned_struct			emptyHRIR_partitioned;
		CMonoBuffer<float>				emptyMonoBuffer;
		oneEarHRIR_struct				emptyOneEarHRIR;
		oneEarHRIR_Partitioned_struct	emptyOneEarHRIR_partitioned;

		/////////////
		// METHODS
		/////////////

		//	Fill out the HRTF for every azimuth and two specific elevations: 90 and 270 degrees
		void CalculateHRIR_InPoles();

		//	Calculate the HRIR in the pole of one of the hemispheres
		//param hemisphereParts	vector of the HRTF orientations of the hemisphere
		HRIR_struct CalculateHRIR_InOneHemispherePole(vector<orientation> hemisphereParts);

		//	Calculate the resample matrix using the Barycentric interpolation Method (copy the HRIR function of the nearest orientation)
		//param resamplingStep	HRTF resample matrix step for both azimuth and elevation
		void CalculateResampled_HRTFTable(int resamplingStep);

		//	Split the input HRIR data in subfilters and get the FFT to apply the UPC algorithm
		//param	newData_time	HRIR value in time domain
		HRIR_Partitioned_struct SplitAndGetFFT_HRTFData(const HRIR_struct & newData_time);

		//		Calculate the distance between two points [(azimuth1, elevation1) and (azimuth2, elevation2)] using the Haversine formula
		//return	float	the distance value
		float CalculateDistance_HaversineFormula(float azimuth1, float elevation1, float azimuth2, float elevation2);

		//	Calculate the HRIR of a specific orientation (newazimuth, newelevation) using the Barycentric interpolation Method
		//param newAzimuth		azimuth of the orientation of interest (the one whose HRIR will be calculated)
		//param newElevation	elevation of the orientation of interest (the one whose HRIR will be calculated)
		HRIR_struct CalculateHRIR_offlineMethod(int newAzimuth, int newElevation);

		//		Calculate the barycentric coordinates of three vertex [(x1,y1), (x2,y2), (x3,y3)] and the orientation of interest (x,y)
		const barycentricCoordinates_struct GetBarycentricCoordinates(float newAzimuth, float newElevation, float x1, float y1, float x2, float y2, float x3, float y3) const;

		//		Transform the orientation in order to move the orientation of interest to 180 degrees
		//returnval	float	transformed azimuth
		float TransformAzimuth(float azimuthOrientationOfInterest, float originalAzimuth);

		//		Transform the orientation in order to express the elevation in the interval [-90,90]
		//returnval float transformed elevation
		float TransformElevation(float elevationOrientationOfInterest, float originalElevation);

		//	Calculate the distance between the given orientation (newAzimuth, newElevation) and all other values of the databsde HRTF table. And store these values in a sorted list
		//param	newAzimuth		azimuth of the orientation of interest in degrees
		//param	newElevation	elevation of the orientation of interest in degrees
		//return the distances sorted list
		std::list<T_PairDistanceOrientation> GetSortedDistancesList(int newAzimuth, int newElevation);

		//	Get HRIR from resample table using a barycentric interpolation of the three nearest orientation.
		const oneEarHRIR_struct GetHRIR_InterpolationMethod(Common::T_ear ear, int azimuth, int elevation) const;

		//	Calculate from resample table HRIR subfilters using a barycentric interpolation of the three nearest orientation.
		const oneEarHRIR_Partitioned_struct GetHRIR_partitioned_InterpolationMethod(Common::T_ear ear, float _azimuth, float _elevation) const;

		//	Calculate HRIR using a barycentric coordinates of the three nearest orientation.
		const oneEarHRIR_struct CalculateHRIRFromBarycentricCoordinates(Common::T_ear ear, barycentricCoordinates_struct barycentricCoordinates, orientation orientation_pto1, orientation orientation_pto2, orientation orientation_pto3) const;

		//	Calculate HRIR subfilters using a barycentric coordinates of the three nearest orientation.
		const oneEarHRIR_Partitioned_struct CalculateHRIR_partitioned_FromBarycentricCoordinates(Common::T_ear ear, barycentricCoordinates_struct barycentricCoordinates, orientation orientation_pto1, orientation orientation_pto2, orientation orientation_pto3)const;

		//		Calculate and remove the common delay of every HRIR functions of the DataBase Table. Off line Method, called from EndSetUp()
		void RemoveCommonDelay_HRTFDataBaseTable();

		//		Calculate the ITD using the Lord Rayleight formula which depend on the interaural azimuth and the listener head radious
		//param		_headRadious		listener head radius, set by the App
		//param		_interauralAzimuth	source interaural azimuth
		//return	float				customizated ITD
		const  float CalculateITDFromHeadRadius(float _headRadius, float _interauralAzimuth)const;
				
		// Recalculate the HRTF FFT table partitioned or not with a new bufferSize or resampling step
		void CalculateNewHRTFTable();

		// Reset HRTF
		void Reset();

		//Compare two float values
		const bool AreSame(float a, float b, float epsilon) const;


		friend class CListener;
	};
}
#endif
