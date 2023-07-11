/**
* \class CAmbisonicDSP
*
* \brief Declaration of AmbisonicDSP interface.
* \date	July 2023
*
* \authors 3DI-DIANA Research Group (University of Malaga), in alphabetical order: M. Cuevas-Rodriguez, P García-Jiménez, D. Gonzalez-Toledo, L. Molina-Tanco ||
* Coordinated by , A. Reyes-Lecuona (University of Malaga) and L.Picinali (Imperial College London) ||
* \b Contact: areyes@uma.es and l.picinali@imperial.ac.uk
*
* \b Contributions: (additional authors/contributors can be added here)
*
* \b Project: SAVLab (Spatial Audio Virtual Laboratory) ||
* 
* \b Copyright: University of Malaga - 2021
*
* \b Licence: GPL v3
*
* \b Acknowledgement: This project has received funding from Spanish Ministerio de Ciencia e Innovación under the SAVLab project (PID2019-107854GB-I00)
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

		CAmbisonicDSP(CCore* ownerCore);

		void ResetAmbisonicBuffers();

		void ProcessVirtualAmbisonicAnechoic(CMonoBuffer<float> & outBufferLeft, CMonoBuffer<float> & outBufferRight, int numberOfSilencedFrames = 0);			

		void ProcessVirtualAmbisonicAnechoic(CStereoBuffer<float> & outBuffer, int numberOfSilencedFrames = 0);

		bool SetAHRBIR();
		
		void SetOrder(int _order);

		int GetOrder();

		void SetInterpolation(bool _interpolation);
		
		bool GetInterpolation();

		void SetNormalization(ambisonicNormalization _normalization);

		ambisonicNormalization GetNormalization();

	private:

		void CalculateHRTF();
		
		void OneChannelAmbisonicEnconder(const CMonoBuffer<float>& inBuffer, std::vector< CMonoBuffer<float> >& outVectorOfBuffers, float azimuth, float elevation);
		
		void TwoChannelAmbisonicEnconder(const CMonoBuffer<float>& inBufferLeft, std::vector< CMonoBuffer<float> >& outVectorOfLeftBuffers, float leftAzimuth, float leftElevation, const CMonoBuffer<float>& inBufferRight, std::vector< CMonoBuffer<float> >& outVectorOfRightBuffers, float rightAzimuth, float rightElevation);
		
		void convertN3DtoSN3D(std::vector<float>& _factors);

		void convertN3DtoMaxN(std::vector<float>& _factors);

		const CAHRBIR& GetAHRBIR() const;

		void ResetAHRBIR();

		bool CalculateAHRBIRPartitioned();

		void CAmbisonicDSP::DegreesToRadians(std::vector<float>& _degrees);

		float CAmbisonicDSP::DegreesToRadians(float& _degrees);

		void getRealSphericalHarmonics(float _ambisonicAzimut, float _ambisonicElevation, std::vector<float> & _factors);

		CMonoBuffer<float> MixChannels(std::vector< CMonoBuffer<float>> Ahrbir_FFT);

		std::vector<float> GetambisonicAzimut();

		std::vector<float> GetambisonicElevation();

		int GetTotalLoudspeakers();

		int CalculateNumberOfChannels();

		int GetTotalChannels();


		// ATTRIBUTES

		bool interpolation;
		ambisonicNormalization normalization;

		CCore* ownerCore;									//Owner Core
		CAHRBIR	environmentAHRBIR;							// ABIR of environment		

		std::vector<std::shared_ptr<CUPCAnechoic>> left_UPConvolutionVector;	//Vector with buffers to perform Uniformly Partitioned Convolution (Left)
		std::vector<std::shared_ptr<CUPCAnechoic>> right_UPConvolutionVector;   //Vector with buffers to perform Uniformly Partitioned Convolution (Right)

		//Ambisonic order
		int ambisonicOrder;
		int numberOfChannels;

		/** \brief Type definition for position of virtual speakers in HRIR */
		const std::vector<float> ambisonicAzimut_order1 = { 90, 270,  0,  0,  0, 180 };
		const std::vector<float> ambisonicElevation_order1 = { 0,   0,  90, 270, 0,  0 };

		const std::vector<float> ambisonicAzimut_order2 = { 328.28, 31.72, 148.28, 211.72,      270,       90,     270,      90,      180,       0,      180,       0 };
		const std::vector<float> ambisonicElevation_order2 = { 0,       0,        0,        0, 328.28, 328.28, 31.72, 31.72, 301.72, 301.72, 58.28, 58.28 };

		const std::vector<float> ambisonicAzimut_order3 = { 290.91, 69.1, 249.1, 110.91,     315,      45,     225,      135,      315,       45,      225,      135,        0,      180,       0,     180,     270,      90,     270,        90 };
		const std::vector<float> ambisonicElevation_order3 = { 0,       0,        0,        0, 35.26, 35.26, 35.26,  35.26, 324.74, 324.74, 324.74, 324.74, 339.1, 339.1, 20.91, 20.91, 69.1, 69.1, 290.91, 290.91 };

		friend class CCore;									//Friend class definition
		friend class CHRTF;
	};

}
#endif