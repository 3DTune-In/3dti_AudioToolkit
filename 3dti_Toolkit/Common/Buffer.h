/**
* \class CBuffer
*
* \brief Declaration of CBuffer interface.
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

#ifndef _CBUFFER_H_
#define _CBUFFER_H_

#define _USE_MATH_DEFINES // TODO: Test in windows! Might also be problematic for other platforms??
#include <cmath>
#include <vector>
#include <algorithm>
#include <cassert>
#include <Common/ErrorHandler.h>
#include <Common/Magnitudes.h>
//#include <initializer_list>

/*! \file */

namespace Common {

	/** \details This is a template class to manage audio streamers and buffers
	*/
	template <
		unsigned int NChannels,
		class stored
	>
		class CBuffer : public std::vector<stored, std::allocator<stored>>
	{
	public:
		using std::vector<stored>::vector;    //   inherit all std::vector constructors
		using std::vector<stored>::size;      //   MacOSX clang seems to need this to compile
		using std::vector<stored>::begin;     //   MacOSX clang seems to need this to compile
		using std::vector<stored>::end;       //   MacOSX clang seems to need this to compile
		using std::vector<stored>::resize;     //   MacOSX clang seems to need this to compile

		/** \brief Get number of channels in the buffer
		*	\retval nChannels number of channels
		*   \eh Nothing is reported to the error handler.
		*/
		constexpr unsigned int GetNChannels() const
		{
			return NChannels;
		}

		/** \brief Add all sample values from another buffer
		*   \eh Nothing is reported to the error handler.
		*/
		CBuffer<NChannels, stored> & operator+= (const CBuffer<NChannels, stored> & oth)
		{
			//assert(GetNChannels()==oth.GetNChannels()); // TODO: agree on error handling
			//assert(size()==oth.size());
			//ASSERT(GetNChannels() == oth.GetNChannels(), "Attempt to mix two buffers of different sizes", ""); 
			//ASSERT(size() == oth.size(), "Attempt to mix two buffers of different sizes", "");

			std::transform(begin(), end(), oth.begin(), begin(), [](stored a, stored b) { return a + b; });
			return *this;
		}

		/** \brief Substract all sample values of another buffer
		*   \eh Nothing is reported to the error handler.
		*/
		CBuffer<NChannels, stored> & operator-= (const CBuffer<NChannels, stored> & oth)
		{
			//assert(GetNChannels()==oth.GetNChannels()); // TODO: agree on error handling
			//assert(size()==oth.size());
			//ASSERT(GetNChannels() == oth.GetNChannels(), "Attempt to mix two buffers of different sizes", ""); 
			//ASSERT(size() == oth.size(), "Attempt to mix two buffers of different sizes", "");

			std::transform(begin(), end(), oth.begin(), begin(), [](stored a, stored b) { return a - b; });
			return *this;
		}

		/** \brief Multiply the values in the buffer by a constant
		*   \eh Nothing is reported to the error handler.
		*/
		// Following recommendations for binary arithmetic operators in: http://en.cppreference.com/w/cpp/language/operators
		friend CBuffer<NChannels, stored> operator* (CBuffer<NChannels, stored> buffer, const stored& gain)
		{
			buffer.ApplyGain(gain);
			return buffer;
		}

		/** \brief Add the values of two buffers
		*   \eh Nothing is reported to the error handler.
		*/
		// Following recommendations for binary arithmetic operators in: http://en.cppreference.com/w/cpp/language/operators
		friend CBuffer<NChannels, stored> operator+ (CBuffer<NChannels, stored> lhs, const CBuffer<NChannels, stored>& rhs)
		{
			lhs += rhs;
			return lhs;
		}

		/** \brief Multiply the values in the buffer by a constant gain
		*	\param [in] gain constant gain value to apply
		*   \eh Nothing is reported to the error handler.
		*/
		void ApplyGain(stored gain)
		{
			//SET_RESULT(RESULT_OK, "Gain applied to buffer succesfully");
			std::for_each(begin(), end(), [gain](stored & a) { a *= gain; });
		}

		/** \brief Multiply the values in the buffer by a no-constant gain (calculate using Weighted moving average method)
		*	\param [in] previousAttenuation Last frame attenuation
		*	\param [in] attenuation Current frame attenuation
		*	\param [in] bufferSize Number of samples in the frame
		*   \eh On error, an error code is reported to the error handler.
		*/
		void ApplyGainGradually(stored previousAttenuation, float attenuation, int bufferSize)
		{
			//Calculate the attenuation increment 
			float attenuationInc = (attenuation - previousAttenuation) / bufferSize;

			//Apply atennuation to each sample
			int nChannels = GetNChannels();
			if (nChannels == 1)
			{
				int i = 0;
				std::for_each(begin(), end(), [previousAttenuation, attenuationInc, &i](stored & a) {
					a *= (previousAttenuation + attenuationInc * i++);
				});
			}
			else if (nChannels == 2)
			{
				int j = 0;
				int halfBufferSize = (int)(*this).size() / 2;
				for (int i = 0; i < halfBufferSize; i++) {
					(*this)[j++] *= previousAttenuation + attenuationInc * i;
					(*this)[j++] *= previousAttenuation + attenuationInc * i;
				}
			}
			else
			{
				SET_RESULT(RESULT_ERROR_NOTIMPLEMENTED, "Apply a gradual gain to a buffer with more than two channels is not implemented yet");
			}
		}

		/** \brief Multiply the values in the buffer by a no-constant gain (calculate using Exponential moving average method)
		*	\param [in] previousAttenuation_Channel1 Last frame attenuation, for mono buffers or left channel of stereo buffer
		*	\param [in] previousAttenuation_Channel2 Last frame attenuation, for right channel of stereo buffer
		*	\param [in] attenuation Current frame attenuation
		*	\param [in] bufferSize Number of samples in the frame
		*	\param [in] sampleRate sample rate, in Hz
		*   \eh On error, an error code is reported to the error handler.
		*/
		void ApplyGainExponentially(float &previousAttenuation_Channel1, float &previousAttenuation_Channel2, float attenuation, int bufferSize, int sampleRate)
		{
			float previousGainChannel2 = previousAttenuation_Channel2;
			float gainChannel2 = 0.0f;
			float previousGainChannel1 = previousAttenuation_Channel1;
			float gainChannel1 = 0.0f;
			float alpha;

			//Calculate alpha
			float denominator = ATTACK_TIME_DISTANCE_ATTENUATION * sampleRate;
			if (denominator > EPSILON_ATTACK_SAMPLES) { alpha = 1 - std::exp(1000 * std::log(0.01f) / denominator); }
			else { alpha = 1; }

			//Apply atennuation to each sample
			int nChannels = GetNChannels();
			if (nChannels == 1)
			{
				int j = 0;
				for (int i = 0; i < (*this).size(); i++) {
					gainChannel1 = (attenuation - previousGainChannel1)* alpha + previousGainChannel1;
					(*this)[j++] *= gainChannel1;
					previousGainChannel1 = gainChannel1;
				}
				previousAttenuation_Channel1 = previousGainChannel1;
			}
			else if (nChannels == 2)
			{
				int j = 0;
				int halfBufferSize = (int)(*this).size() / 2;
				for (int i = 0; i < halfBufferSize; i++) {
					gainChannel1 = (attenuation - previousGainChannel1)* alpha + previousGainChannel1;
					(*this)[j++] *= gainChannel1;
					previousGainChannel1 = gainChannel1;
					gainChannel2 = (attenuation - previousGainChannel2)* alpha + previousGainChannel2;
					(*this)[j++] *= gainChannel2;
					previousGainChannel2 = gainChannel2;
				}
				previousAttenuation_Channel1 = previousGainChannel1;
				previousAttenuation_Channel2 = previousGainChannel2;
			}
			else
			{
				SET_RESULT(RESULT_ERROR_NOTIMPLEMENTED, "Apply a gradual gain to a buffer with more than two channels is not implemented yet");
			}
		}

		/** \brief Fill nToFill samples of the buffer with the same value
		*	\param [in] nToFill number of samples to fill
		*	\param [in] value value to fill with
		*   \eh On success, RESULT_OK is reported to the error handler.
		*/
		void Fill(size_t nToFill, stored value)
		{
			SET_RESULT(RESULT_OK, "Buffer filled with single value succesfully");
			// Assign is the fastest implementation, after memset. See: http://stackoverflow.com/questions/8848575/fastest-way-to-reset-every-value-of-stdvectorint-to-0
			this->assign(nToFill, value);
		}


		/* Combine two buffers by interlacing them into a new bigger buffer.
		If buffers have different sizes, the missing samples will be filled with zeros
		If firstBuffer = FB_ch1_s1, FB_ch2_s1, FB_ch1_s2, FB_ch2_s2....
		And secondBuffer = SB_ch1_s1, SB_ch2_s1, SB_ch1_s2, SB_ch2_s2....
		Then new buffer will be = FB_ch1_s1, SB_ch1_s1, FB_ch2_s1, SB_ch2_s1, FB_ch1_s2, FB_ch1_s2, FB_ch2_s2, SB_ch2_s2.... */
#if 0 // yet untested, leave out for the moment
		CBuffer Interlace(CBuffer firstBuffer, CBuffer secondBuffer)
		{
			// Setup new buffer
			constexpr unsigned int newBufferChannels = firstBuffer.GetNChannels() + secondBuffer.GetNChannels();
			unsigned long newBufferNSamples = firstBuffer.GetNSamples() > secondBuffer.GetNsamples() ? firstBuffer.GetNsamples() : secondBuffer.GetNsamples();
			CBuffer<newBufferChannels, stored> newBuffer;

			// Interlace!
			for (int i = 0; i < newBufferNSamples; i++)
			{
				if (firstBuffer.size() < i)
					newBuffer.push_back(firstBuffer[i]);
				else
					newBuffer.push_back(0);

				if (secondBuffer.size() < i)
					newBuffer.push_back(secondBuffer[i]);
				else
					newBuffer.push_back(0);
			}

			return newBuffer;
		}
#endif

		/** \brief Mix a mono buffer into one channel of this buffer.
		*	\details If sourceBuffer has more samples than this buffer, this buffer size is expanded.
		*	\param [in] sourceBuffer mono buffer to mix from
		*	\param [in] nChannel channel where the buffer will be mixed
		*   \eh On success, RESULT_OK is reported to the error handler.
		*/
		void AddToChannel(CBuffer<1, stored> & sourceBuffer, unsigned int nChannel)
		{
			// error handler: no possible error sources, other than reallocation failure after push_back (not expected)
			SET_RESULT(RESULT_OK, "Samples mixed into channel of buffer succesfully");

			for (int sourceSample = 0; sourceSample < sourceBuffer.size(); sourceSample++)
			{
				if (sourceSample < GetNsamples())
					(*this)[sourceSample*GetNChannels() + nChannel] += sourceBuffer[sourceSample];

				else // Expand all channels!
				{
					for (int ch = 0; ch < GetNChannels(); ch++)
					{
						if (ch == nChannel)
							this->push_back(sourceBuffer[sourceSample]);
						else
							this->push_back(0);
					}
				}
			}
		}

		// Mix any number of buffers into a single mixed buffer
		//CBuffer<NChannels, stored> Mix(std::vector<CBuffer<NChannels, stored>> mixChannels)
		//{
		//	// TO DO: Check number of channels, size...
		//	// TO DO: Faster implementation

		//	CBuffer<NChannels, stored> mixedBuffer;
		//	mixedBuffer = mixChannels[0];

		//	for (int i = 1; i < mixChannels.size(); i++)
		//		mixedBuffer += mixChannels[i];

		//	return mixedBuffer;
		//}

		/** \brief Expand mono buffer into a stereo buffer, duplicating channels
		*	\retval stereoBuffer expanded stereo buffer
		*   \eh On success, RESULT_OK is reported to the error handler.
		*/
		CBuffer<2, stored> FromMonoToStereo() const
		{
			// error handler: no possible error sources, other than reallocation failure after push_back (not expected)
			SET_RESULT(RESULT_OK, "Succesfull conversion of buffer from mono to stereo");

			CBuffer<2, stored> stereoBuffer;

			//// PREcondition: source buffer is mono
			//if (GetNChannels() != 1)
			//	return stereoBuffer; // <- this wont work

			for (int i = 0; i < size(); i++)
			{
				stereoBuffer.push_back((*this)[i]);
				stereoBuffer.push_back((*this)[i]);
			}

			return stereoBuffer;
		}

		/** \brief Expand mono buffer into a stereo buffer, with different gains for each channel
		*	\param [in] leftGain gain applied to left channel
		*	\param [in] rightGain gain applied to right channel
		*	\retval stereoBuffer expanded stereo buffer
		*   \eh On success, RESULT_OK is reported to the error handler.
		*/
		CBuffer<2, stored> FromMonoToStereo(float leftGain, float rightGain) const
		{
			// error handler: no possible error sources, other than reallocation failure after push_back (not expected)
			SET_RESULT(RESULT_OK, "Succesfull weighted conversion of buffer from mono to stereo");

			CBuffer<2, stored> stereoBuffer;

			//// PREcondition: source buffer is mono
			//if (GetNChannels() != 1)
			//	return stereoBuffer; // <- this wont work

			for (int i = 0; i < size(); i++)
			{
				stereoBuffer.push_back(leftGain  * (*this)[i]);
				stereoBuffer.push_back(rightGain * (*this)[i]);
			}

			return stereoBuffer;
		}

		// TO DO: Delete this, because now we have Interlace method???
		/** \brief Compose a stereo buffer out of two mono buffers
		*	\param [in] left mono buffer for left channel
		*	\param [in] right mono buffer for right channel
		*	\pre this must be a stereo buffer
		*	\pre left and right must have the same size
		*	\throws May throw exceptions and errors to debugger
		*   \eh On success, RESULT_OK is reported to the error handler.
		*       On error, an error code is reported to the error handler.
		*/
		void FromTwoMonosToStereo(CBuffer<1, stored> & left, CBuffer<1, stored> & right)
		{
			// PRECONDITION: stereo buffer
			//if (GetNChannels() != 2)
			//{
			//	SET_RESULT(RESULT_ERROR_BADSIZE, "Attempt to compose a buffer from two mono sources, and buffer is not stereo");
			//	return;
			//}
			ASSERT(GetNChannels() == 2, RESULT_ERROR_BADSIZE, "Attempt to compose a buffer from two mono sources, and buffer is not stereo", "");

			// PRECONDITION: buffer sizes
			//if (left.size() != right.size())
			//{
			//	SET_RESULT(RESULT_ERROR_BADSIZE, "Attempt to combine two mono buffers into one stereo, and buffers are of different length");
			//	return;
			//}
			ASSERT(left.size() == right.size(), RESULT_ERROR_BADSIZE, "Attempt to combine two mono buffers into one stereo, and buffers are of different length", "");

			SET_RESULT(RESULT_OK, "Stereo buffer composed out of two mono buffers succesfully");

			// Start with a clean buffer
			this->clear();

			// Interlace channels
			for (int sample = 0; sample < left.size(); sample++)
			{
				this->push_back(left[sample]);
				this->push_back(right[sample]);
			}
		}


		/** \brief Get one channel of a multi-channel buffer into a new mono buffer
		*	\param [in] nchannel channel from which compose mono buffer
		*	\retval monoBuffer new mono buffer with data extracted from channel
		*   \eh On success, RESULT_OK is reported to the error handler.
		*/
		CBuffer<1, stored> GetMonoChannel(int nchannel) const
		{
			// error handler: no possible error sources, other than reallocation failure after push_back (not expected)
			SET_RESULT(RESULT_OK, "Obtained mono buffer from one channel of a bigger buffer succesfully");

			CBuffer<1, stored> monoBuffer;

			/// PREcondition: source buffer has at least nchannel channels
			//if (GetNChannels() < nchannel)
			//	return monoBuffer; // <- this wont work

			for (int i = 0; i < size(); i += GetNChannels())
			{
				monoBuffer.push_back((*this)[i + nchannel]);
			}

			return monoBuffer;
		}

		/** \brief Get number of samples in each channel of the buffer
		*	\pre Buffer must have at least one channel
		*	\retval nSamples number of samples per channel
		*   \eh Nothing is reported to the error handler.
		*/
		unsigned long GetNsamples() const
		{
			return size() / GetNChannels();
		}

		// TODO Delete this
		/** \brief Feed a buffer with data coming from an array
		*	\param [in] _data array of data
		*	\param [in] _length size of data array
		*	\param [in] _nchannels number of channels
		*   \eh On success, RESULT_OK is reported to the error handler.
		*/
		void Feed(stored* _data, int _length, int _nchannels)
		{
			// error handler: no possible error sources, other than reallocation failure after resize (not expected)
			// NOTE: nchannels is not being used
			SET_RESULT(RESULT_OK, "Buffer fed succesfully");

			resize(_length);
			for (int i = 0; i < size(); i++)
			{
				(*this)[i] = _data[i];
			}
		}

		/** \brief Interlace two mono buffers into one stereo buffer
		*	\param [in] left mono buffer for left channel
		*	\param [in] right mono buffer for right channel
		*	\pre this must be a stereo buffer
		*	\pre left and right must have the same size
		*   \eh On error, an error code is reported to the error handler.
		*/
		void Interlace(CBuffer<1, stored> & left, CBuffer<1, stored> & right)
		{
			// Preconditions check
			ASSERT(GetNChannels() == 2, RESULT_ERROR_BADSIZE, "Attempt to interlace into a non-stereo buffer", "");
			ASSERT(left.size() == right.size(), RESULT_ERROR_BADSIZE, "Attempt to interlace two mono buffers of different length", "");
			//SET_RESULT(RESULT_OK, "Stereo buffer interlaced from two mono buffers succesfully");

			// Start with a clean buffer
			this->clear();
			this->reserve(2 * left.size());

			// Interlace channels
			for (int sample = 0; sample < left.size(); sample++)
			{
				this->push_back(left[sample]);
				this->push_back(right[sample]);
			}
		}

		/** \brief Deinterlace stereo buffer into two mono buffers
		*	\param [out] left mono buffer for left channel
		*	\param [out] right mono buffer for right channel
		*	\pre this must be a stereo buffer
		*   \eh On error, an error code is reported to the error handler.
		*/
		void Deinterlace(CBuffer<1, stored> & left, CBuffer<1, stored> & right)
		{
			// Preconditions check
			ASSERT(GetNChannels() == 2, RESULT_ERROR_BADSIZE, "Attempt to deinterlace a non-stereo buffer", "");
			//SET_RESULT(RESULT_OK, "Stereo buffer deinterlaced into two mono buffers succesfully");

			// Start with clean buffers
			left.clear();
			right.clear();

			// Deinterlace channels
			for (int sample = 0; sample < this->GetNsamples() * 2; sample += 2)
			{
				left.push_back((*this)[sample]);
				right.push_back((*this)[sample + 1]);
			}
		}

		/** \brief Copy one buffer into another
		*	\param [in] sourceBuffer source buffer
		*	\pre size (number of channels and number of samples) of source and this buffers must be the same
		*   \eh On error, an error code is reported to the error handler.
		*/
		void SetFromCopy(const CBuffer & sourceBuffer)
		{
			ASSERT(GetNChannels() == sourceBuffer.GetNChannels(), RESULT_ERROR_BADSIZE, "Attempt to copy one buffer into another with different number of channels", "");
			ASSERT(GetNsamples() == sourceBuffer.GetNsamples(), RESULT_ERROR_BADSIZE, "Attempt to copy one buffer into another with different number of samples", "");
			for (int i = 0; i < size(); i++)
			{
				(*this)[i] = sourceBuffer[i];
			}
		}

		///* Mix three buffers into another one
		//*	param [in] a first buffer to mix
		//*	param [in] b second buffer to mix
		//*	param [in] c third buffer to mix
		//*	pre size of the three source buffers must be the same
		//*	throws May throw exceptions and errors to debugger
		//*/
		//void SetFromMix(const CBuffer & a, const CBuffer & b, const CBuffer & c)
		//{
		//	ASSERT(a.size() == b.size(), RESULT_ERROR_BADSIZE, "Attempt to mix three buffers of different size", "");		
		//	ASSERT(a.size() == c.size(), RESULT_ERROR_BADSIZE, "Attempt to mix three buffers of different size", "");

		//	this->clear();
		//	for (int i = 0; i < a.size(); i++)
		//	{
		//		this->push_back(a[i] + b[i] + c[i]);
		//	}
		//}//Mix

		/** \brief Mix any number of buffers into another one
		*	\details example of use: myBuf.SetFromMix({buf1, buf2, buf3});
		*	\param [in] sourceBuffers initializer list of source buffers to mix
		*	\pre size of all source buffers must be the same
		*   \eh On error, an error code is reported to the error handler.
		*/
		// TO DO: compare with other alternative implementations, such as variadic templates	
		void SetFromMix(std::initializer_list<CBuffer> sourceBuffers)
		{
			// Get size of all sourceBuffers and check they are the same
			size_t bufferSize = 0;
			for (typename std::initializer_list<CBuffer>::iterator it = sourceBuffers.begin(); it != sourceBuffers.end(); ++it)
			{
				if (bufferSize == 0)
					bufferSize = (*it).size();
				ASSERT((*it).size() == bufferSize, RESULT_ERROR_BADSIZE, "Attempt to mix buffers with different sizes", "");
			}

			// Iterate through all samples
			this->clear();
			for (int i = 0; i < bufferSize; i++)
			{
				// Iterate through all source buffers
				float sum = 0.0f;
				for (typename std::initializer_list<CBuffer>::iterator it = sourceBuffers.begin(); it != sourceBuffers.end(); ++it)
				{
					sum += (*it)[i];
				}
				this->push_back(sum);
			}
		}

		/** \brief Set buffer from a full-scale upward/downward ramp function
		*	\details For debugging purposes
		*	\param [in] upward if true, creates an upward ramp. If False, create a downward ramp
		*   \eh Nothing is reported to the error handler.
		*/
		void SetFromRamp(bool upward)
		{
			size_t sample = 0;
			float increment = 1.0f / (GetNsamples() - 1);
			float value;

			if (upward)
				value = 0.0f;
			else
				value = 1.0f;

			for (int i = 0; i < GetNsamples(); i++)
			{
				for (int c = 0; c < GetNChannels(); c++)
				{
					(*this)[sample++] = value;
					//WATCH(WV_BUFFER_TEST, value, float);
				}
				if (upward)
					value += increment;
				else
					value -= increment;
			}
		}

		/** \brief Set buffer from a step function with any number of steps
		*	\details For debugging purposes
		*	\param [in] stepWidth number of samples per step
		*	\param [in] stepValues list with values
		*	\param [in] interpolate if true, linear interpolation will be done between steps
		*	\post size of buffer will be stepWidth * number of steps (from initializer list) * number of channels
		*   \eh Nothing is reported to the error handler.
		*/
		void SetFromStepFunction(size_t stepWidth, std::initializer_list<float> stepValues, bool interpolate = false)
		{
			this->clear();

			for (typename std::initializer_list<float>::iterator it = stepValues.begin(); it != stepValues.end(); ++it)
			{
				if (interpolate && std::next(it) == stepValues.end())
					break;

				float value = (*it);

				for (int i = 0; i < stepWidth; i++)
				{
					if (interpolate)
					{
						float leftValue = (*it);
						float rightValue = (*std::next(it));
						value = leftValue + (rightValue - leftValue)*i / stepWidth;
					}
					for (int c = 0; c < GetNChannels(); c++)
					{
						this->push_back(value);
						//WATCH(WV_BUFFER_TEST, value, float);
					}
				}
			}
		}

		/** \brief Set buffer from white noise
		*	\details For debugging purposes. Samples are randomized also among channels
		*   \eh Nothing is reported to the error handler.
		*/
		void SetFromWhiteNoise()
		{
			for (int i = 0; i < size(); i++)
			{
				float halfscale = (float)(RAND_MAX) / 2.0f;
				float value = ((float)rand() - halfscale) / halfscale;
				(*this)[i] = value;
				//WATCH(WV_BUFFER_TEST, value, float);
			}
		}

		/** \brief Set buffer from additive synthesis of any number of tones
		*	\details For debugging purposes
		*	\param [in] samplingRate sampling rate in Hzs
		*	\param [in] frequencieslist list with frequencies of each tone, in Hzs
		*	\param [in] amplitudeslist list with amplitudes of each tone, as gain (typically, between 0.0 and 1.0)
		*	\param [in] phaseslist list with phases of each tone, in radians
		*   \eh On error, an error code is reported to the error handler.
		*   \eh On error, an error code is reported to the error handler.
		*/
		void SetFromAdditiveTones(float samplingRate, std::initializer_list<float> frequencieslist, std::initializer_list<float> amplitudeslist, std::initializer_list<float> phaseslist)
		{
			if ((frequencieslist.size() != amplitudeslist.size()) || (frequencieslist.size() != phaseslist.size()))
			{
				SET_RESULT(RESULT_ERROR_BADSIZE, "When creating buffers from additive tones, the size of frequencies, amplitudes and phases lists need to be the same");
				return;
			}

			vector<float> frequencies;
			vector<float> amplitudes;
			vector<float> phases;

			for (typename std::initializer_list<float>::iterator it = frequencieslist.begin(); it != frequencieslist.end(); ++it)
				frequencies.push_back(*it);
			for (typename std::initializer_list<float>::iterator it = amplitudeslist.begin(); it != amplitudeslist.end(); ++it)
				amplitudes.push_back(*it);
			for (typename std::initializer_list<float>::iterator it = phaseslist.begin(); it != phaseslist.end(); ++it)
				phases.push_back(*it);

			Fill(size(), 0.0f);
			for (int i = 0; i < frequencies.size(); i++)
			{
				CBuffer oneTone(size());
				oneTone.SetFromTone(samplingRate, frequencies[i], amplitudes[i], phases[i]);
				(*this) += oneTone;
			}

			//// WATCH (for Mono buffers):
			//for (int i = 0; i < size(); i++)
			//{
			//	WATCH(WV_BUFFER_TEST, (*this)[i], float);
			//}
		}

		/** \brief Set buffer from a pure tone
		*	\details For debugging purposes
		*	\param [in] samplingRate sampling rate in Hzs
		*	\param [in] frequency frequency of tone in Hzs
		*	\param [in] amplitude amplitude of tone, as gain (typically, between 0.0 and 1.0)
		*	\param [in] phase phase of tone in radians
		*   \eh Nothing is reported to the error handler.
		*/
		void SetFromTone(float samplingRate, float frequency, float amplitude = 1.0f, float phase = 0.0f)
		{
			for (size_t i = 0; i < GetNsamples(); i++)
			{
				for (int c = 0; c < GetNChannels(); c++)
				{
					float value = amplitude * std::sin(2.0f * M_PI*frequency*((float)i / samplingRate) + phase);
					(*this)[i] = value;
					//WATCH(WV_BUFFER_TEST, value, float);
				}
			}
		}

		/** \brief Calculate power of a mono buffer signal
		*	\retval power power
		*   \eh Nothing is reported to the error handler.
		*/
		float GetPower()
		{
			return GetAutocorrelation( 0 );
		}

		/** \brief Calculate autocorrelation of a mono buffer 
		*	\param [in] n displacement in number of samples 
		*	\details The output is not normalized. To normalize, divide it by GetPower()
		*	\retval coefficient n-th coefficient of autocorrelation
		*   \eh Nothing is reported to the error handler.
		*/
		float GetAutocorrelation( int n )
		{
			ASSERT(GetNChannels() == 1, RESULT_ERROR_BADSIZE, "Attempt to calculate autocorrelation of a non-mono buffer", "");
			ASSERT((*this).size() > 0, RESULT_ERROR_BADSIZE, "Attempt to calculate autocorrelation of a empty buffer", "");
			ASSERT((*this).size() > n && n >= 0, RESULT_ERROR_INVALID_PARAM, "Invalid displacement in GetAutocorrelation", "");

			int bufferSize = (*this).size();
			float acum = 0.0f;

			int overlapedSamples = bufferSize - n;

			// Calculate Autocorrelation = (1/(bufferSize-n)) *sum(x[i] * x[i-n])
			for(int c=0; c < overlapedSamples; c++ )
				acum += (*this)[c] * (*this)[c+n];
			
			return overlapedSamples <= 0 ? 0 : acum / ((float)overlapedSamples);
		}
	};
}

/** \brief One channel specialization of CBuffer
*/
template<class stored>
using CMonoBuffer = Common::CBuffer<1,stored>;

/** \brief Two channels specialization of CBuffer
*/
template<class stored>
using CStereoBuffer = Common::CBuffer<2,stored>;

/** \brief Non-enforcing buffer 
*	\details Current implementation does not inherits from CBuffer
*/
template<class stored>
using CMultiChannelBuffer = std::vector<stored>; // TO DO: why isnt it CBuffer?????



namespace Common
{
	/** \brief Stream output for float Mono buffers
	*/
	inline std::ostream & operator<<(std::ostream & out, const CMonoBuffer<float> & b)
	{
		for (size_t i = 0; i < b.size(); i++)
			out << b[i] << std::endl;
		return out;
	}
}

#endif
