/**
* \class CSingleSourceDSP
*
* \brief Declaration of CSingleSourceDSP interface.
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

#ifndef _CSINGLESOURCEDSP_H_
#define _CSINGLESOURCEDSP_H_

#include <Common/Buffer.h>
#include <Common/Transform.h>
#include <Common/CommonDefinitions.h>
#include <BinauralSpatializer/Core.h>
#include <BinauralSpatializer/Listener.h>
#include <Common/DistanceAttenuator.h>
#include <Common/FarDistanceEffects.h>
#include <BinauralSpatializer/ILD.h>
#include <Common/FarDistanceEffects.h>
#include <BinauralSpatializer/UPCAnechoic.h>
#include <Common/FiltersChain.h>

//#define USE_UPC_WITHOUT_MEMORY
#define EPSILON 0.0001f
#define ELEVATION_SINGULAR_POINT_UP 90.0
#define ELEVATION_SINGULAR_POINT_DOWN 270.0

namespace Binaural {
	
	/** Type definition for Spatialization Modes
	*/
	enum TSpatializationMode {
		NoSpatialization,					///<    No spatialization
		HighPerformance,			///<	Spatialize using the high performance method
		HighQuality					///<	Spatialize using the high quality method
	};

	class CCore;

	/** \details This class manages the anechoic spatialization of a single source*/
	class CSingleSourceDSP
	{
		public:
		////////////
		// Methods
		////////////

		/** \brief Constructor with parameters
		*	\details links the source to the binaural core
		*	\param [in] _ownerCore pointer to the binaural core
		*   \eh On error, an error code is reported to the error handler.
		*/
		CSingleSourceDSP(CCore* _ownerCore);

		/** \brief Update internal buffer
		*	\details This must be called before calling to ProcessAnechoic or ProcessVirtualAmbisonicReverb 
		*	\param [in] buffer reference to new buffer content
		*	\sa ProcessAnechoic, ProcessVirtualAmbisonicReverb
		*   \eh Nothing is reported to the error handler.
		*/
		void SetBuffer(CMonoBuffer<float> & buffer);					

		/** \brief Get internal buffer
		*	\retval buffer internal buffer content
		*   \eh Nothing is reported to the error handler.
		*/
		const CMonoBuffer<float> GetBuffer() const;						

		/** \brief Move source (position and orientation)
		*	\param [in] _sourceTransform new position and orientation of source
		*   \eh Nothing is reported to the error handler.
		*/
		void SetSourceTransform(Common::CTransform _sourceTransform);

		/** \brief Get source transform (position and orientation)
		*	\retval transform reference to current position and orientation of source
		*   \eh Nothing is reported to the error handler.
		*/
		const Common::CTransform & GetSourceTransform() const;
																			
		/** \brief Get the attenuation in anechoic process for a given distance 	
		*	\param [in] distance distance for checking attenuation
		*	\retval gain attenuation, as gain (typically, between 0 and 1)		
		*   \eh Nothing is reported to the error handler.
		*/
		float GetAnechoicDistanceAttenuation(float distance) const;			

		/** \brief Get the attenuation in reverb process for a given distance
		*	\param [in] distance distance for checking attenuation
		*	\retval gain attenuation, as gain (typically, between 0 and 1)		
		*   \eh Nothing is reported to the error handler.
		*/
		float GetReverbDistanceAttenuation(float distance) const;			
						
		/** \brief Enable run-time HRTF interpolation 
		*   \eh Nothing is reported to the error handler.
		*/
		void EnableInterpolation();
		
		/** \brief Disable run-time HRTF interpolation 
		*/
		void DisableInterpolation();
		
		/** \brief Get the flag for run-time HRTF interpolation 
		*	\retval IsInterpolationEnabled if true, run-time HRTF interpolation is enabled for this source
		*   \eh Nothing is reported to the error handler.
		*/
		bool IsInterpolationEnabled();

		/** \brief Enable anechoic spatialization process for this source
		*   \eh Nothing is reported to the error handler.
		*/
		void EnableAnechoicProcess();

		/** \brief Disable anechoic spatialization process for this source
		*   \eh Nothing is reported to the error handler.
		*/
		void DisableAnechoicProcess();

		/** \brief Get the flag for anechoic spatialization process enabling
		*	\retval anechoicEnabled if true, anechoic spatialization process is enabled for this source
		*   \eh Nothing is reported to the error handler.
		*/
		bool IsAnechoicProcessEnabled();

		/** \brief Enable reverb process for this source
		*   \eh Nothing is reported to the error handler.
		*/
		void EnableReverbProcess();

		/** \brief Disable reverb process for this source
		*   \eh Nothing is reported to the error handler.
		*/
		void DisableReverbProcess();
		
		/** \brief Get the flag for reverb process enabling
		*	\retval reverbEnabled if true, reverb process is enabled for this source
		*   \eh Nothing is reported to the error handler.
		*/
		bool IsReverbProcessEnabled();

		/** \brief Enable far distance effect for this source
		*   \eh Nothing is reported to the error handler.
		*/
		void EnableFarDistanceEffect();
		
		/** \brief Disable far distance effect for this source
		*   \eh Nothing is reported to the error handler.
		*/
		void DisableFarDistanceEffect();
		
		/** \brief Get the flag for far distance effect enabling
		*	\retval farDistanceEffectEnabled if true, far distance effect is enabled for this source
		*   \eh Nothing is reported to the error handler.
		*/
		bool IsFarDistanceEffectEnabled();
		
		/** \brief Enable distance attenuation effect for this source for anechoic path
		*   \eh Nothing is reported to the error handler.
		*/
		void EnableDistanceAttenuationAnechoic();
		
		/** \brief Disable distance attenuation effect for this source for anechoic path
		*   \eh Nothing is reported to the error handler.
		*/
		void DisableDistanceAttenuationAnechoic();
		
		/** \brief Get the flag for distance attenuation effect enabling for anehcoic path
		*	\retval distanceAttenuationEnabled if true, distance attenuation effect is enabled for this source
		*   \eh Nothing is reported to the error handler.
		*/
		bool IsDistanceAttenuationEnabledAnechoic();

		/** \brief Enable distance attenuation effect for this source for reverb path
		*   \eh Nothing is reported to the error handler.
		*/
		void EnableDistanceAttenuationReverb();

		/** \brief Disable distance attenuation effect for this source for reverb path
		*   \eh Nothing is reported to the error handler.
		*/
		void DisableDistanceAttenuationReverb();

		/** \brief Get the flag for distance attenuation effect enabling for reverb path
		*	\retval distanceAttenuationEnabled if true, distance attenuation effect is enabled for this source
		*   \eh Nothing is reported to the error handler.
		*/
		bool IsDistanceAttenuationEnabledReverb();
		
		/** \brief Enable near field effect for this source
		*   \eh Nothing is reported to the error handler.
		*/
		void EnableNearFieldEffect();
		
		/** \brief Disable near field effect for this source
		*   \eh Nothing is reported to the error handler.
		*/
		void DisableNearFieldEffect();
		
		/** \brief Get the flag for near field effect enabling
		*	\retval nearFieldEffectEnabled if true, near field effect is enabled for this source
		*   \eh Nothing is reported to the error handler.
		*/
		bool IsNearFieldEffectEnabled();
				
		/** \brief Reset the play buffers of this source
		*   \details This must be called when the source has been stopped
		*   \eh Nothing is reported to the error handler.
		*/
		void ResetSourceBuffers();
		
		/** \brief Set the spatialization mode for this source
		*   \eh Nothing is reported to the error handler.
		*/
		void SetSpatializationMode(TSpatializationMode _spatializationMode);


		/** \brief Get the current spatialization mode for this source
		*	\retval spatializationMode Current spatialization mode for this source
		*/
		TSpatializationMode GetSpatializationMode();

		/** \brief Process data from internal buffer to generate anechoic spatialization (direct path)
		*	\param [out] outLeftBuffer output mono buffer with spatialized audio for the left channel
		*	\param [out] outRightBuffer output mono buffer with spatialized audio for the right channel
		*	\pre Internal buffer must be updated before any call to this method (See \link SetBuffer \endlink)
		*   \eh On error, an error code is reported to the error handler.
		*/
		void ProcessAnechoic(CMonoBuffer<float> &outLeftBuffer, CMonoBuffer<float> &outRightBuffer);

		/** \brief Process data from internal buffer to generate anechoic spatialization (direct path)
		*	\param [out] outBuffer output stereo buffer with spatialized audio for both channels
		*	\pre Internal buffer must be updated before any call to this method (See \link SetBuffer \endlink)
		*   \eh On error, an error code is reported to the error handler.
		*/
		void ProcessAnechoic(CStereoBuffer<float> & outBuffer);

		/** \brief Process data from input buffer to generate anechoic spatialization (direct path)
		*	\param [in] inBuffer input buffer with anechoic audio
		*	\param [out] outLeftBuffer output mono buffer with spatialized audio for the left channel
		*	\param [out] outRightBuffer output mono buffer with spatialized audio for the right channel
		*   \eh On error, an error code is reported to the error handler.
		*/
		void ProcessAnechoic(const CMonoBuffer<float> & inBuffer, CMonoBuffer<float> &outLeftBuffer, CMonoBuffer<float> &outRightBuffer);

		/** \brief Process data from input buffer to generate anechoic spatialization (direct path)
		*	\param [in] inBuffer input buffer with anechoic audio
		*	\param [out] outBuffer output stereo buffer with spatialized audio for the both channels
		*   \eh On error, an error code is reported to the error handler.
		*/
		void ProcessAnechoic(const CMonoBuffer<float> & inBuffer, CStereoBuffer<float> & outBuffer);

		/** \brief Returns the azimuth of the specified ear.
		*	\param [in] ear must be Common::T_ear::LEFT or Common::T_ear::RIGHT
		*   \eh On error, an error code is reported to the error handler.
		*/
		float GetEarAzimuth( Common::T_ear ear ) const;

		/** \brief Returns the elevation of the specified ear.
		*	\param [in] ear must be Common::T_ear::LEFT or Common::T_ear::RIGHT
		*   \eh On error, an error code is reported to the error handler.
		*/
		float GetEarElevation(Common::T_ear ear) const;

	private:	
		/////////////
		// METHODS	
		/////////////

		// Make the spatialization using HRTF convolution
		void ProcessHRTF(CMonoBuffer<float> &inBuffer, CMonoBuffer<float> &outLeftBuffer, CMonoBuffer<float> &outRightBuffer, float leftAzimuth, float leftElevation, float rightAzimuth, float rightElevation, float _azCenter, float _elCenter);
		/// Make the spatialization using a ILD aproach				
		void ProccesILDSpatializationAndAddITD(CMonoBuffer<float> &leftBuffer, CMonoBuffer<float> &rightBuffer, float distance, float interauralAzimuth, float leftAzimuth, float leftElevation, float rightAzimuth, float rightElevation);
		void ProcessILDSpatialization(CMonoBuffer<float> &leftBuffer, CMonoBuffer<float> &rightBuffer, float distance_m, float azimuth);		
		
		// Apply distance attenuation
		void ProcessDistanceAttenuationAnechoic(CMonoBuffer<float> &buffer, int bufferSize, int sampleRate, float distance);	
		// Apply Far distance effect
		void ProcessFarDistanceEffect(CMonoBuffer<float> &buffer, float distance);										
		// Apply Near field effects (ILD)		
		void ProcessNearFieldEffect(CMonoBuffer<float> &leftBuffer, CMonoBuffer<float> &rightBuffer, float distance, float interauralAzimuth);				
		
		// Apply the directionality to simulate the hearing aid device
		void ProcessDirectionality(CMonoBuffer<float> &leftBuffer, CMonoBuffer<float> &rightBuffer, float angleToForwardAxisRadians);
		void ProcessDirectionality(CMonoBuffer<float> &buffer, float directionalityAttenutaion, float angleToForwardAxis_rad);		

		// Apply doppler effect simulation
		void ProcessAddDelay_ExpansionMethod(CMonoBuffer<float>& input, CMonoBuffer<float>& output, CMonoBuffer<float>& delayBuffer, int newDelay);
		// Reset source convolution buffers
		void ResetSourceConvolutionBuffers(shared_ptr<CListener> listener);
		// return the flag which tells if the buffer is updated and ready for a new anechoic process
		bool IsAnechoicProcessReady();
		// return the flag which tells if the buffer is updated and ready for a new reverb process
		bool IsReverbProcessReady();
		// sets the ready flag for reverb process to false
		void SetReverbProcessNotReady();
		// In orther to obtain the position where the HRIR is needed, this method calculate the projection of each ear in the sphere where the HRTF has been measured
		const Common::CVector3 GetSphereProjectionPosition(Common::CVector3 vectorToEar, Common::CVector3 earLocalPosition, float distance) const;

		// Calculates the values ot attributes related to the relative position between sound source and
		// the listener.
		void CalculateSourceCoordinates();
				
		///////////////
		// ATTRIBUTES
		///////////////
		const CCore* ownerCore;					// Reference to the core where information shared by all sources is stored (listener, room and audio state attributes)	
		Common::CTransform sourceTransform;		// Position and orientation of source
		CMonoBuffer<float> internalBuffer;		// Buffer storage
					
	#ifdef USE_FREQUENCY_COVOLUTION_WITHOUT_PARTITIONS_ANECHOIC
		Common::CFconvolver outputLeft;   						// Object to make the inverse fft of the left channel
		Common::CFconvolver outputRight;						// Object to make the inverse fft of the rigth channel
	#else
		Binaural::CUPCAnechoic outputLeftUPConvolution;		// Object to make the inverse fft of the left channel with the UPC method
		Binaural::CUPCAnechoic outputRightUPConvolution;	// Object to make the inverse fft of the rigth channel with the UPC method
	#endif							
		
		CMonoBuffer<float> leftChannelDelayBuffer;			// To store the delay of the left channel of the expansion method
		CMonoBuffer<float> rightChannelDelayBuffer;			// To store the delay of the right channel of the expansion method
					
		Common::CDistanceAttenuator distanceAttenuatorAnechoic;	// Computes the attenuation for far and medium distances		
		Common::CDistanceAttenuator distanceAttenuatorReverb;	// Computes the attenuation for far and medium distances			
		Common::CFarDistanceEffects farDistanceEffect;			// Computes filtering effect for far distances in anechoic 
				
		Common::CEarPair<Common::CFiltersChain> nearFieldEffectFilters;		// Computes the Near field effects
		Common::CEarPair<Common::CFiltersChain> ILDSpatializationFilters;	// Computes the ILD Spatialization

		Common::CVector3 lastSourceProjectionVector;	//To store the last projection vector of the source position. Use to apply the restrictions to the source position;

		bool enableAnechoic;	// Flags for independent control of processes
		bool enableReverb;		// Flags for independent control of processes
		bool readyForAnechoic;	// Flags for independent control of processes
		bool readyForReverb;	// Flags for independent control of processes

		bool enableInterpolation;		// Enables/Disables the interpolation on run time			
		bool enableFarDistanceEffect;	// Enables/Disables the low pass filtering that is applied at far distances
		bool enableDistanceAttenuationAnechoic;	// Enables/Disables the attenuation that depends on the distance to the listener for anechoic path
		bool enableDistanceAttenuationReverb;	// Enables/Disables the attenuation that depends on the distance to the listener for reverb path
		bool enableNearFieldEffect;     // Enables/Disables the ILD (Interaural Level Difference) processing		
		
		TSpatializationMode spatializationMode; //Select the spatialization method

		float leftAzimuth;     // Left ear's azimuth
		float leftElevation;   // Left ear's elevation

		float rightAzimuth;    // Right ear's azimuth
		float rightElevation;  // Right ear's elevation

		float centerAzimuth;   // Azimuth from the center of the head
		float centerElevation; // Elevation from the center of the head 

		float distanceToListener; // Distance to the listener
		float interauralAzimuth;  // Iteraural azimuth

		Common::CVector3 vectorToListener;  // Vector to the listener

		friend class CEnvironment;		//Friend Class definition
		friend class CCore;				//Friend Class definition		
	};   
}
#endif
