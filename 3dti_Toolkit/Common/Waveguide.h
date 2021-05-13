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
#include <Common/Vector3.h>
#include <boost/circular_buffer.hpp>

namespace Common {
	class CWaveguide
	{
	public:

		struct TSourcePosition {
			float x;
			float y;
			float z;
			int beginIndex;
			int endIndex;	
			
			TSourcePosition(int _beginIndex, int _endIndex, CVector3 _sourcePosition) { 
				x = _sourcePosition.x;
				y = _sourcePosition.y;
				z = _sourcePosition.z;
				beginIndex = _beginIndex;
				endIndex = _endIndex;
			}

		};

		/** \brief Constructor
		*/		
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


		/** \brief Insert the new frame into the waveguide
		*/		
		//void PushBack(CMonoBuffer<float> & _buffer, const Common::TAudioStateStruct& audioState, float soundSpeed, float currentDistanceToListener);
		void PushBack(CMonoBuffer<float> & _buffer, const Common::TAudioStateStruct& audioState, float soundSpeed, const CVector3 & _sourcePosition, float currentDistanceToListener);
		
		/** \brief Get next frame frame after pass throught the waveguide
		*/
		CMonoBuffer<float> PopFront(const Common::TAudioStateStruct& audioState);
		
		/** \brief Get most recent Buffer
		*/
		CMonoBuffer<float> GetMostRecentBuffer() const;
				
	private:
		/// Calculate the new delay in samples.
		size_t CalculateDelay(Common::TAudioStateStruct audioState, float soundSpeed, float distanceToListener);		
		
		/// Resize the circular buffer
		void ResizeCirculaBuffer(size_t newSize);
		
		/// Changes de circular buffer capacity, throwing away the oldest samples
		void CWaveguide::RsetCirculaBuffer(size_t newSize);
		
		/// Execute a buffer expansion or compression
		void ProcessExpansionCompressionMethod(const CMonoBuffer<float>& input, CMonoBuffer<float>& output);
		/// Execute a buffer expansion or compression, and introduce the samples directly into the circular buffer
		void ProcessExpansionCompressionMethod(const CMonoBuffer<float>& input, int outputSize);		
		
		//void ResizeSourcePositionBuffer(size_t newSize, CVector3 sourcePosition);
		//CMonoBuffer<CVector3> GetSourcePositionsBuffer(size_t size, CVector3 sourcePosition);
		void InsertSourcePositionBuffer(int bufferSize, const CVector3 & sourcePosition);
		void InsertSourcePositionBuffer(int begin, int end, const CVector3 & sourcePosition);
		void InitSourcePositionBuffer(int numberOFZeroSamples);
		void ShiftSourcePositionsBuffer(int samples);
		int GetIndexOfCirculaBuffer(boost::circular_buffer<float>::iterator it);
		
		const float CalculateDistance(const CVector3 & position1, const CVector3 & position2) const;
		float CalculateSourceDistanceChange(const CVector3 & newSourcePosition);
		CVector3 GetSourceOldPosition();
		///////////////
		// Vars
		///////////////
		bool enablePropagationDelay;					/// To store if the propagation delay is enabled or not		
		CMonoBuffer<float> mostRecentBuffer;			/// To store the last buffer introduced into the waveguide
		boost::circular_buffer<float> circular_buffer;	/// To store the samples into the waveguide			
		
		vector<TSourcePosition> sourcePositionsBuffer;	/// To store the source positions in each frame
		CVector3 listenerOldPosition;					/// To store the last position of the listener

		//TO DO Delete me
		//int contadorDani; 
		//void CoutCircularBuffer();
		//void CoutBuffer(const CMonoBuffer<float> & _buffer, string bufferName) const;
	};
}
#endif
