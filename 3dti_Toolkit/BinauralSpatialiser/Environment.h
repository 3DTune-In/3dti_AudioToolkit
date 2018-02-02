/**
* \class CEnvironment
*
* \brief Declaration of CEnvironment interface.
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

#ifndef _CCENVIRONMENT_H_
#define _CCENVIRONMENT_H_

#include <Common/AIR.h>
#include <BinauralSpatializer/Listener.h>
#include <Common/Buffer.h>
#include <Common/Fprocessor.h>
#include <Common/UPCEnvironment.h>
#include <Common/CommonDefinitions.h>
#include <vector>
#include <memory>

/** \brief Type definition for position of virtual speakers in BRIR
*/
enum VirtualSpeakerPosition {
	NORTH = 0,				///<	SPK1 (north)
	SOUTH,					///<	SPK3 (south)
	EAST,					///<	SPK4 (east)
	WEST,					///<	SPK2 (west)
};


namespace Binaural {

    class CCore;
	class CSingleSourceDSP;
	class CBRIR;

	/**    \details Class for implementation of binaural environment simulation.*/
    class CEnvironment
    {
    public:
        
		/** \brief Constructor with parameters
         *	\param [in] ownerCore pointer to owner core
		 *   \eh Nothing is reported to the error handler.
         */
        CEnvironment(CCore* ownerCore);

		/** \brief Get AudioState of the owner Core of this Environment
		*	\retval AudioState_Struct current audio state set in core
		*/
		Common::AudioState_Struct GetCoreAudioState() const;

		/** \brief Get BRIR of the environment
		*	\retval BRIR pointer to current BRIR set for this environment
		*   \eh Nothing is reported to the error handler.
		*/
		CBRIR* GetBRIR() const;

        /** \brief Get ABIR of environment
         *	\retval ABIR reference to current environment ABIR         
		 *   \eh Nothing is reported to the error handler.
         */
        const CABIR& GetABIR() const;
		        
		/** \brief Reset the reverb play buffers
		*   \details This must be called when all sources have been stopped
		*   \eh Nothing is reported to the error handler.
		*/
		void ResetReverbBuffers();
              		
		/** \brief Configure AIR class (with the partitioned impulse responses) using BRIR data for the UPC algorithm
		*   \eh Nothing is reported to the error handler.
		*/
		void CalculateABIRPartitioned();

		/** \brief Configure AIR class using BRIR data for the basic convolution algorithm
		*   \eh Nothing is reported to the error handler.
		*/
		void CalculateABIRwithoutPartitions();
        
		/** \brief Process virtual ambisonics reverb for all sources with binaural output in separate mono buffers
		*	\details Internally takes as input the (updated) buffers of all registered audio sources 
		*	\param [out] outBufferLeft output buffer with the processed reverb for left ear
		*	\param [out] outBufferRight output buffer with the processed reverb for right ear
		*	\sa SetBuffer, SingleSourceDSP
		*   \eh Warnings may be reported to the error handler.
		*/
		void ProcessVirtualAmbisonicReverb(CMonoBuffer<float> & outBufferLeft, CMonoBuffer<float> & outBufferRight);
		
		/** \brief Process virtual ambisonics reverb for all sources with binaural output in a single stereo buffer
		*	\details Internally takes as input the (updated) buffers of all registered audio sources 
		*	\param [out] outBuffer stereo output buffer with the processed reverb				
		*	\sa SetBuffer, SingleSourceDSP
		*   \eh Nothing is reported to the error handler.
		*/
		void ProcessVirtualAmbisonicReverb(CStereoBuffer<float> & outBuffer);

		/** \brief Process reverb for a single b-format channel encoded with 1st order ambisonics
		*	\details This might be useful to implement reverb in some wrappers, such as the Unity Wrapper
		*	\param [in] channel which b-format channel was encoded
		*	\param [in] encoderIn input buffer with the encoded channel
		*	\param [out] output output buffer with the processed reverb
		*   \eh Nothing is reported to the error handler.
		*/
		void ProcessEncodedChannelReverb(TBFormatChannel channel, CMonoBuffer<float> encoderIn, CMonoBuffer<float> & output);

    private:
		
		// Set ABIR of environment. Create AIR class using ambisonic codification. Also, initialize convolution buffers
		void SetABIR();
		// Calculate the BRIR again
		void CalculateBRIR();
		// Apply the directionality to simulate the hearing aid device
		void ProcessDirectionality(CMonoBuffer<float> &buffer, float directionalityAttenutaion);
		// Reset BRIR
		void ResetBRIR_ABIR();

		// ATTRIBUTES

		CCore* ownerCore;									//Owner Core
		CABIR	environmentABIR;							// ABIR of environment		
		std::unique_ptr<CBRIR> environmentBRIR;				// BRIR of the environment
#ifdef USE_FREQUENCY_COVOLUTION_WITHOUT_PARTITIONS_REVERB 
        Common::CFprocessor outputLeft;						//Ambisonic Reverb Convolutions
		Common::CFprocessor outputRight;					//Ambisonic Reverb Convolutions
#else		
		Common::CUPCEnvironment wLeft_UPConvolution;		//Buffers to perform Uniformly Partitioned Convolution
		Common::CUPCEnvironment xLeft_UPConvolution;		//Buffers to perform Uniformly Partitioned Convolution
		Common::CUPCEnvironment yLeft_UPConvolution;		//Buffers to perform Uniformly Partitioned Convolution
		Common::CUPCEnvironment wRight_UPConvolution;		//Buffers to perform Uniformly Partitioned Convolution
		Common::CUPCEnvironment xRight_UPConvolution;		//Buffers to perform Uniformly Partitioned Convolution
		Common::CUPCEnvironment yRight_UPConvolution;		//Buffers to perform Uniformly Partitioned Convolution
#endif
		int HADirectionality_LeftChannel_version;			//HA Directionality left version
		int HADirectionality_RightChannel_version;			//HA Directionality right version
                
        friend class CCore;									//Friend class definition
		friend class CBRIR;
};

}
#endif