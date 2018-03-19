/**
* \class CILD
*
* \brief Declaration of CILD class interface.
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
#ifndef _CILD_H_
#define _CILD_H_

#include <unordered_map>
#include <Common/FiltersChain.h>
#include <Common/Buffer.h>
#include <Common/CommonDefinitions.h>


	/** \brief Class to be used as Key in the hash table used by CILD
	*/
	class CILD_Key
	{
	public:
		int distance;      ///< Distance to the center of the head, in millimeters 
		int azimuth;       ///< Azimuth angle of interaural coordinates, in degrees

		CILD_Key() :CILD_Key{ 0,0 } {}

		CILD_Key(int _distance, int _azimuth) :distance{ _distance }, azimuth{ _azimuth } {}

		bool operator==(const CILD_Key& key) const
		{
			return (this->azimuth == key.azimuth && this->distance == key.distance);
		}
	};

	namespace std
	{
		template<>
		struct hash<CILD_Key>
		{
			// adapted from http://en.cppreference.com/w/cpp/utility/hash
			size_t operator()(const CILD_Key & key) const
			{
				size_t h1 = std::hash<int>()(key.distance);
				size_t h2 = std::hash<int>()(key.azimuth);
				return h1 ^ (h2 << 1);  // exclusive or of hash functions for each int.
			}
		};
	}

	/** \brief Template class to hold the coefficients of a set of biquad filters.
	*/
	template <int NUMBER_OF_BIQUAD_FILTERS>
	class CILD_BiquadFilterCoefs
	{
	public:
		float coefs[5 * NUMBER_OF_BIQUAD_FILTERS];   /**< Holds the coefficients of one or more biquad filters.
														Each biquad filter has 5 coefficients.
														Format: f1_b0, f1_b1, f1_b2, f1_a1, f1_a2, f2_b0, f2_b1, f2_b2, f2_a1, f2_a2, ...
															where fx means filter xth     */
	};

	/** \brief Type definition to work with a set of coefficients for two biquad filters
	*/
	typedef CILD_BiquadFilterCoefs<2> T_ILD_TwoBiquadFilterCoefs;

	/** \brief Hash table that contains a set of coefficients for two biquads filters that are indexed through a pair of distance
	 and azimuth values (interaural coordinates). */
	typedef std::unordered_map<CILD_Key, T_ILD_TwoBiquadFilterCoefs> T_ILD_HashTable;

namespace Binaural {

	/** \details This class models the effect of frequency-dependent Interaural Level Differences when the sound source is close to the listener
	*/
	class CILD
	{

	public:
		/////////////
		// METHODS
		/////////////

		/** \brief Default constructor.
		*	\details Leaves ILD Table empty. Use SetILDNearFieldEffectTable to load.
		*   \eh Nothing is reported to the error handler.
		*/
		CILD();

		/** \brief Add the hash table for computing ILD Near Field Effect
		*	\param [in] newTable data for hash table
		*   \eh Nothing is reported to the error handler.
		*/					
		void AddILDNearFieldEffectTable(T_ILD_HashTable && newTable);

		/** \brief Add the hash table for computing ILD Spatialization
		*	\param [in] newTable data for hash table
		*   \eh Nothing is reported to the error handler.
		*/
		void AddILDSpatializationTable(T_ILD_HashTable && newTable);

		/** \brief Get the internal hash table used for computing ILD Near Field Effect
		*	\retval hashTable data from the hash table
		*   \eh Nothing is reported to the error handler.
		*/
		const T_ILD_HashTable & GetILDNearFieldEffectTable() { return t_ILDNearFieldEffect; }
		
		/** \brief Get the internal hash table used for computing ILD Spatialization
		*	\retval hashTable data from the hash table
		*   \eh Nothing is reported to the error handler.
		*/
		const T_ILD_HashTable & GetILDSpatializationTable() { return t_ILDSpatialization; }
		
		/** \brief Get IIR filter coefficients for ILD Near Field Effect, for one ear
		*	\param [in] ear ear for which we want to get the coefficients
		*	\param [in] distance_m distance, in meters
		*	\param [in] azimuth azimuth angle, in degrees
		*	\retval std::vector<float> contains the coefficients following this order [f1_b0, f1_b1, f1_b2, f1_a1, f1_a2, f2_b0, f2_b1, f2_b2, f2_a1, f2_a2]
		*   \eh On error, an error code is reported to the error handler.
		*/				
		std::vector<float> GetILDNearFieldEffectCoefficients(Common::T_ear ear, float distance_m, float azimuth);
		
		/** \brief Get IIR filter coefficients for ILD Spatialization, for one ear
		*	\param [in] ear ear for which we want to get the coefficients
		*	\param [in] distance_m distance, in meters
		*	\param [in] azimuth azimuth angle, in degrees
		*	\retval std::vector<float> contains the coefficients following this order [f1_b0, f1_b1, f1_b2, f1_a1, f1_a2, f2_b0, f2_b1, f2_b2, f2_a1, f2_a2]
		*   \eh On error, an error code is reported to the error handler.
		*/
		std::vector<float> GetILDSpatializationCoefficients(Common::T_ear ear, float distance_m, float azimuth);	

	private:
		///////////////
		// ATTRIBUTES
		///////////////	

		T_ILD_HashTable t_ILDNearFieldEffect;
		int ILDNearFieldEffectTable_AzimuthStep;  //In degress
		int ILDNearFieldEffectTable_DistanceStep; //In milimeters
												  
		T_ILD_HashTable t_ILDSpatialization;	  
		int ILDSpatializationTable_AzimuthStep;		//In degress
		int ILDSpatializationTable_DistanceStep;	//In milimeters
	};
}
#endif
