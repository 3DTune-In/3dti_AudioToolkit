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
#include <boost/math/interpolators/cardinal_cubic_b_spline.hpp>

constexpr float EPSILON = 0.0001;

namespace Common {
	void CChannel::PushBack(CMonoBuffer<float> & _buffer, float currentDistance) {
		float changeInDistance = currentDistance - lastDistance;
		lastDistance = currentDistance;
		float step = 1.0f / 44100; // FIXME Get this from audio framework
		// Distance is shrinking or augmenting 
		if (std::fabs(changeInDistance) > EPSILON) {
			boost::math::interpolators::cardinal_cubic_b_spline<float> spline(_buffer.begin(), _buffer.end(), 0, step);
			float changeInMilliseconds = changeInDistance / 343.0f; // FIXME Get this from audio framework. 
			float changeInNSamples = changeInMilliseconds * 44.1; // FIXME Get this from audio framework. 
			size_t sizeOfInterpolatedBuffer = std::nearbyint(_buffer.size() + changeInNSamples);
			CMonoBuffer<float> interpolatedBuffer(sizeOfInterpolatedBuffer);
			float factor = _buffer.size() / (_buffer.size() + changeInNSamples);
			for (size_t i = 0; i < sizeOfInterpolatedBuffer; i++) {
				float j = i * factor;
				interpolatedBuffer[i] = spline(j);
			}
			try {
				// Now change the capacity to acommodate the new delay. 
				circular_buffer.rset_capacity(circular_buffer.capacity() - 512 + sizeOfInterpolatedBuffer); // FIXME hardcoded frame size to 512
			}
			catch (std::bad_alloc &) {
				SET_RESULT(RESULT_ERROR_BADALLOC, "Bad alloc in delay buffer (pushing back frame)");
				return;
			}
			// and add the new frame
			circular_buffer.insert(circular_buffer.end(), interpolatedBuffer.begin(), interpolatedBuffer.end());
		}
		// No change
		else  {
			// Just add frame at the end of the buffer.Circular buffer is fixed size
			// so the latest frame (at the front) will be deleted. 
			circular_buffer.insert(circular_buffer.end(), _buffer.begin(), _buffer.end());
		}
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
		try {
			if (circular_buffer.capacity() < samples) { // Buffer has to grow

					// FIXME: For the moment this is just a fixed delay, 
					// FIXME: hardcoded frameSize as 512. 
				circular_buffer.resize(samples + 512); // Is it samples or samples + frame size??

			}
			else { // Buffer has to shrink
				// FIXME: hardcoded frameSize as 512. 
				circular_buffer.rset_capacity(samples + 512); // FIXME: should be rset_capacity so that we get rid of the oldest samples

			}
		}
		catch (std::bad_alloc & e)
		{
			SET_RESULT(RESULT_ERROR_BADALLOC, "Bad alloc in delay buffer");
			return;
		}
	}
	CMonoBuffer<float> CChannel::GetMostRecentBuffer() const
	{
		return mostRecentBuffer;
	}
}