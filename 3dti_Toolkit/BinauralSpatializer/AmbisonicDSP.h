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

#ifndef _CCAMBISONICDSP_H_
#define _CCAMBISONICDSP_H_

#include <Common/AHRIR.h>
#include <BinauralSpatializer/Listener.h>
#include <Common/Buffer.h>
#include <Common/Fprocessor.h>
#include <Common/CommonDefinitions.h>
#include <BinauralSpatializer/UPCAnechoic.h>
#include <vector>
#include <memory>

enum ambisonicNormalization {N3D, SN3D, maxN};

namespace Binaural {

	class CCore;
	class CSingleSourceDSP;
	class CHRTF;

	/**    \details Class for implementation of binaural environment simulation.*/
	class CAmbisonicDSP
	{
	public:

		/** \brief Constructor with parameters
		 *	\param [in] ownerCore pointer to owner core
		 *   \eh Nothing is reported to the error handler.
		 */
		CAmbisonicDSP(CCore* ownerCore);

		/** \brief Get AudioState of the owner Core of this Environment
		*	\retval TAudioStateStruct current audio state set in core
		*/
		Common::TAudioStateStruct GetCoreAudioState() const;

		/** \brief Get ABIR of environment
		 *	\retval ABIR reference to current environment ABIR
		 *   \eh Nothing is reported to the error handler.
		 */
		const CAHRBIR& GetAHRBIR() const;

		/** \brief Reset the reverb play buffers
		*   \details This must be called when all sources have been stopped
		*   \eh Nothing is reported to the error handler.
		*/
		void ResetAmbisonicBuffers();

		/** \brief Configure AIR class (with the partitioned impulse responses) using BRIR data for the UPC algorithm
		*	\retval	boolean to indicate if calculation was successful
		*   \eh Nothing is reported to the error handler.
		*/
		bool CalculateAHRBIRPartitioned();

		/** \brief Process virtual ambisonics reverb for all sources with binaural output in separate mono buffers
		*	\details Internally takes as input the (updated) buffers of all registered audio sources
		*	\param [out] outBufferLeft output buffer with the processed reverb for left ear
		*	\param [out] outBufferRight output buffer with the processed reverb for right ear
		*   \param [in] numberOfSilencedFrames number of initial silenced frames in the reverb stage
		*	\sa SetBuffer, SingleSourceDSP
		*   \eh Warnings may be reported to the error handler.
		*/
		void ProcessVirtualAmbisonicAnechoic(CMonoBuffer<float> & outBufferLeft, CMonoBuffer<float> & outBufferRight, int numberOfSilencedFrames = 0);
		void ProcessVirtualAmbisonicISM(CMonoBuffer<float>& outBufferLeft, CMonoBuffer<float>& outBufferRight, vector<CSingleSourceDSP>& virtualSources, int numberOfSilencedFrames = 0);
			

		/** \brief Process virtual ambisonics reverb for all sources with binaural output in a single stereo buffer
		*	\details Internally takes as input the (updated) buffers of all registered audio sources
		*	\param [out] outBuffer stereo output buffer with the processed reverb
		*   \param [in] numberOfSilencedFrames number of initial silenced frames in the reverb stage
		*	\sa SetBuffer, SingleSourceDSP
		*   \eh Nothing is reported to the error handler.
		*/
		void ProcessVirtualAmbisonicAnechoic(CStereoBuffer<float> & outBuffer, int numberOfSilencedFrames = 0);

		int GetOrder();

		int GetTotalLoudspeakers();

		void SetOrder(int _order);

		int GetTotalChannels();

		std::vector<float> GetambisonicAzimut();

		std::vector<float> GetambisonicElevation();

		//void CalculateSourceCoordinates(Common::CTransform _sourceTransform, Common::CVector3 & _vectorToListener, float & _distanceToListener, float & centerElevation, float & centerAzimuth, float & interauralAzimuth);
		//void CalculateSourceCoordinates(Common::CTransform _sourceTransform, Common::CVector3 & _vectorToListener, float & _distanceToListener, float & leftElevation, float & leftAzimuth, float & rightElevation, float & rightAzimuth, float & centerElevation, float & centerAzimuth, float & interauralAzimuth);

		void getRealSphericalHarmonics(float _ambisonicAzimut, float _ambisonicElevation, std::vector<float> & _factors);

		CMonoBuffer<float> mixChannels(std::vector< CMonoBuffer<float>> Ahrbir_FFT);

		bool SetAHRBIR();

		void CAmbisonicDSP::DegreesToRadians(std::vector<float>& _degrees);

		void CAmbisonicDSP::DegreesToRadians(float& _degrees);
		// Reset AHRBIR
		void ResetAHRBIR();

		void SetInterpolation(bool _interpolation);
		bool GetInterpolation();

		void SetNormalization(ambisonicNormalization _normalization) {
			normalization = _normalization;
			ResetAHRBIR();
		}

		ambisonicNormalization GetNormalization() {
			return normalization;
		}

		void convertN3DtoSN3D(std::vector<float>& _factors);

		void convertN3DtoMaxN(std::vector<float>& _factors);

		void ExpansionMethod(CMonoBuffer<float>& input, CMonoBuffer<float>& output, CMonoBuffer<float>& delayBuffer, int newDelay);

	private:

		// Set AHRBIR of environment. Create AIR class using ambisonic codification. Also, initialize convolution buffers
		// Calculate the HRTF again
		void CalculateHRTF();

		// ATTRIBUTES

		bool interpolation;
		bool separateCoding;
		ambisonicNormalization normalization;

		CCore* ownerCore;									//Owner Core
		CAHRBIR	environmentAHRBIR;							// ABIR of environment		

		std::vector<std::shared_ptr<CUPCAnechoic>> left_UPConvolutionVector;	//Vector with buffers to perform Uniformly Partitioned Convolution (Left)
		std::vector<std::shared_ptr<CUPCAnechoic>> right_UPConvolutionVector;   //Vector with buffers to perform Uniformly Partitioned Convolution (Right)

		//Ambisonic order
		int AmbisonicOrder;

		/** \brief Type definition for position of virtual speakers in HRIR */
		std::vector<float> ambisonicAzimut_order1 = { 90, 270,  0,  0,  0, 180 };
		std::vector<float> ambisonicElevation_order1 = { 0,   0,  90, 270, 0,  0 };

		std::vector<float> ambisonicAzimut_order2 = { 328.28, 31.72, 148.28, 211.72,      270,       90,     270,      90,      180,       0,      180,       0 };
		std::vector<float> ambisonicElevation_order2 = { 0,       0,        0,        0, 328.28, 328.28, 31.72, 31.72, 301.72, 301.72, 58.28, 58.28 };

		std::vector<float> ambisonicAzimut_order3 = { 290.91, 69.1, 249.1, 110.91,     315,      45,     225,      135,      315,       45,      225,      135,        0,      180,       0,     180,     270,      90,     270,        90 };
		std::vector<float> ambisonicElevation_order3 = { 0,       0,        0,        0, 35.26, 35.26, 35.26,  35.26, 324.74, 324.74, 324.74, 324.74, 339.1, 339.1, 20.91, 20.91, 69.1, 69.1, 290.91, 290.91 };

		friend class CCore;									//Friend class definition
		friend class CHRTF;
	};

}
#endif