/**
* \class CChannel
*
* \brief Declaration of CChannel interface.
* \date	October 2020
*
* \authors 3DI-DIANA Research Group (University of Malaga), in alphabetical order: M. Cuevas-Rodriguez, D. Gonzalez-Toledo, L. Molina-Tanco ||
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

#ifndef _CWAVEGUIDE_H_
#define _CWAVEGUIDE_H_

#include <Common/Buffer.h>
#include <Common/AudioState.h>
#include <boost/circular_buffer.hpp>

namespace Common {
	class CWaveguide
	{
	public:

		/** \brief Constructor
		*/
		//CWaveguide() : enablePropagationDelay(false), contadorDani(0){}
		CWaveguide() : enablePropagationDelay(false) {}


		/** \brief Enable propagation delay for this waveguide
		*   \eh Nothing is reported to the error handler.
		*/
		void EnablePropagationDelay();

		/** \brief Disable propagation delay for this waveguide
		*   \eh Nothing is reported to the error handler.
		*/
		void DisablePropagationDelay();

		/** \brief Get the flag for propagation delay enabling
		*	\retval propagationDelayEnabled if true, propagation delay simulation is enabled for this source
		*   \eh Nothing is reported to the error handler.
		*/
		bool IsPropagationDelayEnabled();


		/** \brief Add frame to channel buffer
		*/
		//void PushBack(CMonoBuffer<float> & _buffer, float currentDistance);
		void PushBack(CMonoBuffer<float> & _buffer, const Common::TAudioStateStruct& audioState, float soundSpeed, float currentDistanceToListener);

		/** \brief Get next frame from channel buffer
		*/
		CMonoBuffer<float> PopFront(const Common::TAudioStateStruct& audioState) const;

		/** \brief Set Delay directly in samples
		*/
		//void SetDelayInSamples(int frames);

		/** \brief Get most recent Buffer
		*/
		CMonoBuffer<float> GetMostRecentBuffer() const;
		
		//void SetBuffer(CCore* _ownerCore);		
	private:
		size_t CalculateDelay(Common::TAudioStateStruct audioState, float soundSpeed, float distanceToListener);		
		void ResizeCirculaBuffer(size_t newSize);
		void CWaveguide::RsetCirculaBuffer(size_t newSize);
		void ProcessExpansionCompressionMethod(const CMonoBuffer<float>& input, CMonoBuffer<float>& output);
		//void ProcessExpansionCompressionMethod(const CMonoBuffer<float>& input, size_t resultSize);
		
		//void CalculateExpandCompressBuffer(const CMonoBuffer<float> & _inpuBuffer, CMonoBuffer<float> & interpolatedBuffer, const Common::TAudioStateStruct& audioState, int newDelayInSamples, int currentDelayInSamples);		
		void CoutCircularBuffer();
		void CoutBuffer(const CMonoBuffer<float> & _buffer, string bufferName) const;
		
		// vars
		CMonoBuffer<float> mostRecentBuffer;
		boost::circular_buffer<float> circular_buffer;
		//double lastDistance;
		bool enablePropagationDelay;


		//int contadorDani; //TO DO Delete me
	};
}
#endif
