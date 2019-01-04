/**
* \class CListener
*
* \brief Declaration of CListener interface.
* \date	July 2016
*
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

#ifndef _CLISTENER_H_
#define _CLISTENER_H_

#include <Common/Transform.h>
#include <Common/Magnitudes.h>
#include <Common/Buffer.h>
#include <Common/AudioState.h>
#include <HAHLSimulation/HearingAidSim.h>
#include <BinauralSpatializer/ILD.h>

#include <vector>
#include <memory>

namespace Binaural {

class CCore;
class CHRTF;

/** \details Class for handling configuration of binaural listener.
*/
	class CListener
	{    
	public:
		////////////    
		// METHODS
		////////////
		/** \brief Constructor with parameters 
		*	\details should not be called directly 
		*	\param [in] _ownerCore pointer to already created CCore object
		*	\param [in] _listenerHeadRadius listener head radius for ITD customization, in meters (defaults to 0.0875)		
		*	\sa CreateListener
		*   \eh Nothing is reported to the error handler.
		*/
		CListener(CCore* _ownerCore, float _listenerHeadRadius=0.0875f);

		/** \brief Set listener position and orientation
		*	\param [in] _listenerTransform new listener position and orientation		
		*   \eh Nothing is reported to the error handler.		
		*/
		void SetListenerTransform(Common::CTransform _listenerTransform);							

		/** \brief Get listener position and orientation
		*	\retval transform current listener position and orientation		
		*   \eh Nothing is reported to the error handler.
		*/
		const Common::CTransform GetListenerTransform() const;	
		
		/** \brief Get EarPosition local to the listenerr
		*   \param [in] ear indicates the ear which you want to knowthe position
		*	\retval ear local position
		*   \eh Nothing is reported to the error handler.
		*/
		Common::CVector3 GetListenerEarLocalPosition(Common::T_ear ear) const;

		/** \brief Get HRTF of listener
		*	\retval HRTF pointer to current listener HRTF	
		*   \eh On error, an error code is reported to the error handler.
		*/	
		CHRTF* GetHRTF() const;

		/** \brief Get ILD Near Field effect of listener
		*	\retval ILD pointer to current listener ILD Near Field effect 
		*   \eh Nothing is reported to the error handler.
		*/
		CILD* GetILD() const;
			
		/** \brief Set ILD attenuation of listener for HighPerformance Spatialization. If no attenuation is set, -6dB are applied by default
		*	\param [in] _listenerILDAttenutationDB new listener ILD attenuation, in dB		
		*   \eh Nothing is reported to the error handler.
		*/
		void SetILDAttenutaion(float _listenerILDAttenutationDB);

		/** \brief Get ILD attenuation of listener for HighPerformance Spatialization
		*	\retval attenuation value in dB of attenuation to apply the HighPerformance ILD spatialization
		*   \eh Nothing is reported to the error handler.
		*/						
		float GetILDAttenutaion();
	
		/** \brief Set head radius of listener
		*	\param [in] _listenerHeadRadius new listener head radius, in meters		
		*   \eh Nothing is reported to the error handler.
		*/
		void SetHeadRadius(float _listenerHeadRadius);										

		/** \brief Get head radius of listener
		*	\retval radius current listener head radius, in meters		
		*   \eh Nothing is reported to the error handler.
		*/
		float GetHeadRadius() const;													

		/** \brief Enable customized ITD process
		*   \eh Nothing is reported to the error handler.
		*/
		void EnableCustomizedITD();
		
		/** \brief Disable customized ITD process
		*   \eh Nothing is reported to the error handler.
		*/
		void DisableCustomizedITD();
		
		/** \brief Get the flag for customized ITD process enabling
		*	\retval customizzedITDEnabled if true, customized ITD process is enabled
		*   \eh Nothing is reported to the error handler.
		*/
		bool IsCustomizedITDEnabled();

		/** \brief Get position and orientation of one listener ear
		*	\param [in] ear listener ear for wich we want to get transform
		*	\retval transform current listener ear position and orientation
		*   \eh On error, an error code is reported to the error handler.
		*/
		Common::CTransform GetListenerEarTransform(Common::T_ear ear) const;
	
		/** \brief Get Audio State from Core owner 
		*	\retval audiostate current audio state set in core	
		*   \eh Nothing is reported to the error handler.
		*/	
		Common::TAudioStateStruct GetCoreAudioState() const;

		/** \brief Get CMagnitudes instance from owner Core
		*	\retval magnitudes magnitudes object
		*   \eh Nothing is reported to the error handler.
		*/
		Common::CMagnitudes GetCoreMagnitudes() const;

		/** \brief Enable directionality simulation for one or both ears
		*	\param [in] ear ear/s for which ear we want to enable directionality
		*   \eh Nothing is reported to the error handler.
		*/
		void EnableDirectionality(Common::T_ear ear);

		/** \brief Disable directionality simulation for one or both ears
		*	\param [in] ear ear/s for which ear we want to disable directionality
		*   \eh Nothing is reported to the error handler.
		*/
		void DisableDirectionality(Common::T_ear ear);

		/** \brief Get the flag for directionality process enabling for one ear
		*	\param [in] ear for which ear we want to get directionality flag
		*	\retval directionalityEnabled if true, directionality process is enabled for that ear
		*   \eh Nothing is reported to the error handler.
		*/
		bool IsDirectionalityEnabled(Common::T_ear ear);

		/** \brief Set anechoic directionality attenuation for one or both ears
		*	\param [in] ear for which ear we want to set directionality
		*	\param [in] _directionalityAttenuation attenuation in dB
		*   \eh Nothing is reported to the error handler.
		*/
		void SetDirectionality_dB(Common::T_ear ear, float _directionalityAttenuation);

		/** \brief Get anechoic directionality attenuation for one ear
		*	\param [in] ear for which ear we want to get directionality attenuation
		*	\retval attenuation current attenuation for that ear, in dB
		*   \eh Nothing is reported to the error handler.
		*/
		float GetAnechoicDirectionalityAttenuation_dB(Common::T_ear ear) const;

		/** \brief Get reverb directionality attenuation for one ear
		*	\param [in] ear for which ear we want to get directionality attenuation
		*	\retval attenuation current attenuation for that ear, in dB
		*   \eh Nothing is reported to the error handler.
		*/
		float GetReverbDirectionalityAttenuation_dB(Common::T_ear ear) const;

		/** \brief returns the linear attenuation due to directionality
		*	\param [in] directionalityExtend sets the directionality front-back ratio (restricted values from 0 to 30 dBs)
		*	\param [in] angleToForwardAxis_rad angle in radians between the forward direction and the vector to the sound source
		*	\retval attenuation attenuation in dB
		*   \eh Nothing is reported to the error handler.
		*/
		float CalculateDirectionalityAttenuation_dB(float directionalityExtend, float angleToForwardAxis_rad);

		/** \brief Get HRTF resampling step from owner core		
		*	\retval HRTF resampling step, in degress
		*   \eh Nothing is reported to the error handler.
		*/
		int GetHRTFResamplingStep() const;
		
	private:
		// Set the notification that a new HRTF has been loaded into CHRTF class				
		void SetHRTFLoaded();

		// Calculate the HRTF resampling and FFT again
		void CalculateHRTF();

		// Gets reverb directionality linear attenuation
		float GetReverbDirectionalityLinearAttenuation(Common::T_ear ear) const;

		// Gets reverb directionality linear attenuation
		float GetAnechoicDirectionalityLinearAttenuation(Common::T_ear ear) const;

		// Calculates reverb directionality linear attenuation
		float CalculateDirectionalityLinearAttenuation(float directionalityExtend, float angleToForwardAxis_rad);

		// Calculates reverb directionality linear attenuation
		float CalculateReverbDirectionalityLinearAttenuation(float directionalityExtend_dB);
		
		// Reset HRTF 
		void ResetHRTF();

		// Reset ILD
		void ResetILD();

		///////////////
		// ATTRIBUTES
		///////////////

		CCore* ownerCore;							// owner Core		
		std::unique_ptr<CHRTF> listenerHRTF;		// HRTF of listener														
		std::unique_ptr<CILD> listenerILD;			// ILD of listener		
		
		Common::CTransform listenerTransform;		// Transform matrix (position and orientation) of listener    
		float listenerHeadRadius;					// Head radius of listener     

		float listenerILDAttenutationDB;			// Attenuation to apply when the ILD is in use (HighPerformance)

		Common::CEarPair<float> anechoicDirectionalityLinearAttenuation;  // Max value for directionality attenuation in dBs for each channel
		Common::CEarPair<float> reverbDirectionalityLinearAttenuation;    // Max value for directionality attenuation in dBs for each channel				
		Common::CEarPair<bool>	enableDirectionality;				// True when current settings for directionality will be applied to each channel (left or right)		

		friend class CHRTF;							//Friend Class definition
		friend class CCore;							//Friend Class definition
		friend class CEnvironment;					//Friend Class definition
		friend class CSingleSourceDSP;				//Friend Class definition
		friend class CListener;						//Friend Class definition
	};    
}
#endif
