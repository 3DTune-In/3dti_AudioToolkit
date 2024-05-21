/**
* \class CBiquadFilter
*
* \brief Declaration of CBiquadFilter class interface.
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

#ifndef _CBIQUADILTER_H_
#define _CBIQUADILTER_H_

#include <Common/Buffer.h>
#include "CommonDefinitions.h"

namespace Common {

	/** \brief Type definition for specifying the type of filter
	*/
	enum T_filterType {
		LOWPASS = 0,	///< Low pass filter
		HIGHPASS = 1,	///< High pass filter
		BANDPASS = 2,	///< Band pass filter
		LOWSHELF = 3,	///< Low shelf filter
		HIGHSHELF = 4,	///< High shelf filter
		PEAKNOTCH = 5	///< Peak Notch filter
	};

	/** \brief Type definition for a vector of filter coefficients for one biquad
	*	\details Order: b0, b1, b2, a1, a2  
	*/
	typedef std::vector<float> TBiquadCoefficients; 


	/** \details This class implements a biquad filter (two poles and two zeros).
	\n Useful diagrams can be found in:	https://en.wikipedia.org/wiki/Digital_biquad_filter */
	class CBiquadFilter
	{
	public:
		///////////////////
		// PUBLIC METHODS
		//////////////////

		/** \brief Default constructor.
		*	\details By default, sets sampling frequency to 44100Hz.
		*   \eh Nothing is reported to the error handler.
		*/
		CBiquadFilter();

		/** \brief Set up the filter
		*	\param [in] samplingRate sampling frequency, in Hertzs
		*	\param [in] b0 coefficient b0
		*	\param [in] b1 coefficient b1
		*	\param [in] b2 coefficient b2
		*	\param [in] a1 coefficient a1
		*	\param [in] a2 coefficient a2
		*   \eh On error, an error code is reported to the error handler.
		*/
		void Setup(float samplingRate, float b0, float b1, float b2, float a1, float a2);

		/** \brief Set up the filter
		*	\param [in] samplingRate sampling frequency, in Hertzs
		*	\param [in] frequency relevant frequency (cutoff or band center)
		*	\param [in] Q Q factor
		*	\param [in] filterType type of filter
		*	\param [in] gain filter gain (general Gain for LowPass, HighPass, BandPass; gain for LowShelf, HighShelf, PeakNotch)
		*   \eh On error, an error code is reported to the error handler.
		*/
		void Setup(float samplingRate, float frequency, float Q, T_filterType filterType, float gain = 1.0f);

		/** \brief Set up coefficients of the filter
		*	\param [in] b0 coefficient b0
		*	\param [in] b1 coefficient b1
		*	\param [in] b2 coefficient b2
		*	\param [in] a1 coefficient a1
		*	\param [in] a2 coefficient a2
		*   \eh Nothing is reported to the error handler.
		*/
		void SetCoefficients(float b0, float b1, float b2, float a1, float a2);

		/** \brief Set up coefficients of the filter
		*	\param [in] coefficients coefficients array. Order: b0, b1, a1, a2  
		*   \eh Nothing is reported to the error handler. */
		void SetCoefficients(float *coefficients);
		
		/** \brief Set up coefficients of the filter
		*	\param [in] coefficients coefficients vector. Order: b0, b1, a1, a2  
		*   \eh Nothing is reported to the error handler. */		
		void SetCoefficients(TBiquadCoefficients& coefficients);

		/** \brief Set up coefficients of the filter
		*	\details Lowpass, Highpass and Bandpass filters are Butterworth design
		*	\param [in] frequency relevant frequency (cutoff or band center)
		*	\param [in] Q Q factor
		*	\param [in] filterType type of filter
         *   \param [in] gain filter gain (general Gain for LowPass, HighPass, BandPass; gain for LowShelf, HighShelf, PeakNotch)
		*   \eh On error, an error code is reported to the error handler.
		*/
		void SetCoefficients(float frequency, float Q, T_filterType filterType, float gain = 1.0f);

		/** \brief Set the sampling frequency at which audio samples were acquired
		*	\param [in] _samplingFreq sampling frequency, in Hertzs
		*   \eh On success, RESULT_OK is reported to the error handler.
		*       On error, an error code is reported to the error handler.
		*/
		void SetSamplingFreq(float _samplingFreq);

		/** \brief Filter the input data according to the filter setup.
		*	\param [in] inBuffer input buffer
		*	\param [out] outBuffer output buffer
		*	\param [in] addResult when true, samples resulting from the	filtering process are added to the current value of the output buffer.
		*	\pre Input and output buffers must have the same size, which should be greater than 0.
		*   \eh On error, an error code is reported to the error handler.
		*/
		void Process(CMonoBuffer<float> &inBuffer, CMonoBuffer<float> & outBuffer, bool addResult = false);

		/**
		\overload
		*/
		void Process(CMonoBuffer<float> &buffer);

		/** \brief Set the gain of the filter 
		*	\param [in] _gain filter gain 
		*   \eh Nothing is reported to the error handler.
		*/
		void SetGeneralGain(float _gain);

		/** \brief Get the gain of the filter
		*	\retval gain filter gain
		*   \eh Nothing is reported to the error handler.
		*/
		float GetGeneralGain();

	private:
		////////////////////
		// PRIVATE METHODS
		///////////////////
		void AvoidNanValues();                                          // Prevent the filter from ending up in unstable states

		bool SetCoefsFor_BandPassFilter(double centerFreqHz, double Q); // Calculates the coefficients of a biquad band-pass filter.
		bool SetCoefsFor_LPF(double cutoffFreq, double Q);              // Calculate the coefficients of a biquad low-pass filter.
		bool SetCoefsFor_HPF(double cutoffFreq, double Q);              // Calculates the coefficients of a biquad high-pass filter.  
		bool SetCoefsFor_LowShelf(double cutoffFreq, double Q);         // Calculates the coefficients of a biquad low-shelf filter.
		bool SetCoefsFor_HighShelf(double cutoffFreq, double Q);        // Calculates the coefficients of a biquad high-shelf filter.
		bool SetCoefsFor_PeakNotch(double centerFreqHz, double Q);      // Calculates the coefficients of a biquad peak-notch filter.   

																		// Does the basic processing of the biquad filter. Receives the current sample, the coefficients and the delayed samples
																		// Returns the result of the biquad filter 
		double ProcessSample(const double sample, const double a1, const double a2, const double b0, const double b1, const double b2, double &z1, double &z2);


		void UpdateAttributesAfterCrossfading();// Set current coefficients to new cofficients and updates the delay cells and the crossfadingNeeded attribute.

		////////////////
		// ATTRIBUTES
		////////////////
		float generalGain;                                              // Gain applied to every sample obtained with Process

		double samplingFreq;                                            // Keep the sampling rate at which audio samples were taken
		double z1_l, z2_l, z1_r, z2_r;                                  // Keep last values to implement the delays of the filter (left and right channels)
		double b0, b1, b2, a1, a2;                                      // Coeficients of the Butterworth filter

		double new_b0, new_b1, new_b2, new_a1, new_a2;                  // New coefficients to implement cross fading
		double new_z1_l, new_z2_l, new_z1_r, new_z2_r;                  // Keep last values to implement the delays of the filter (left and right channels)
		bool   crossfadingNeeded;                                       // True when cross fading must be applied in the next frame
	};
}
#endif
