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

#include <Common/Channel.h>
#include <Common/ErrorHandler.h>

namespace Common {
	void CChannel::PushBack(CMonoBuffer<float> & _buffer) {
		// Add frame at the end of the buffer. Circular buffer is fixed size
		// so the latest frame (at the front) will be deleted. 
		circular_buffer.insert(circular_buffer.end(), _buffer.begin(), _buffer.end());

		// Save copy of this most recent Buffer
		mostRecentBuffer = _buffer; 
	}
	CMonoBuffer<float> CChannel::PopFront() const
	{
		// FIXME: properly get the frame size which is hardcoded as 512
		// Pop really doesn't pop. The next time a buffer is pushed, it will be removed.  
		CMonoBuffer<float> returnbuffer(circular_buffer.begin(), circular_buffer.begin() + 512);
		return returnbuffer;
	}
	void CChannel::SetDelayInSamples(int samples)
	{
		if (circular_buffer.capacity() < samples) { // Buffer has to grow
			try {
				// FIXME: For the moment this is just a fixed delay, 
				// FIXME: hardcoded frameSize as 512. 
				circular_buffer.resize(samples + 512); // Is it samples or samples + frame size??
			}
			catch (std::bad_alloc & e)
			{
				SET_RESULT(RESULT_ERROR_BADALLOC, "Bad alloc in delay buffer");
				return;
			}
		}
		else { // Buffer has to shrink
			// FIXME: hardcoded frameSize as 512. 
			circular_buffer.set_capacity(samples+512);
		}
	}
	CMonoBuffer<float> CChannel::GetMostRecentBuffer() const
	{
		return mostRecentBuffer;
	}
}