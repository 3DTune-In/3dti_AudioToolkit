/**
* \class CProfiler
*
* \brief Declaration of CProfiler class interface.
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
#ifndef _CPROFILER_H_
#define _CPROFILER_H_

#include <iostream>
#include <string>
#include <vector>

/*! \file */

// TO DO: Cleaning a lot the code!!!!!!!!!!!!! (Automatic, error handler, etc)

/** \brief Macro used for easy access to profiler singleton
*/
#define PROFILER3DTI CProfiler::Instance()

//
// Defines that you may want to use
//

// Time Units presets
#define UNITS_TICKS			0	///< Time units as processor ticks
#define UNITS_MICROSECONDS	1	///< Time units as microseconds
#define UNITS_NANOSECONDS	2	///< Time units as nanoseconds

// Keep only one of these two defines. Don't change if you don't know the meaning!!
#define PROFILER_USE_VECTOR
//#define PROFILER_USE_ARRAY		// TO DO: Array implementation is NOT tested


//
// Defines you will never need to change
//

// Some values used by resolution presets
#define MICROSECONDS_IN_ONE_SECOND	1000000
#define NANOSECONDS_IN_ONE_SECOND	1000000000

// This ugly c-like use of arrays is recommended for minimum overhead of the profiler:
#ifdef PROFILER_USE_ARRAY
#define MAX_PROFILER_SAMPLES 100000
#else
#define DEFAULT_PROFILER_SAMPLES 1000	///< Maximum number of samples stored in profiler. This ugly c-like use of arrays is recommended for minimum overhead of the profiler (avoids dynamic memory allocation)
#endif
// Instead, we use c++ vectors at the cost of a small overhead
// This overhead can be significant for array sizes <100 or >100000
// Please, see: http://assoc.tumblr.com/post/411601680/performance-of-stl-vector-vs-plain-c-arrays


/// Type redefinition for 64 bit integers
typedef long long TInt64;

///////////////////////////////////////////////////////////////////////////////

namespace Common {

	/** \brief Class for storing time measures, with units
	*/
	class CTimeMeasure
	{
		// PUBLIC METHODS:
	public:

		/** \brief Set time units
		*	\details If this method is not called, default units are \link UNITS_TICKS \endlink
		*	\param [in] unitsPreset one of the preset time units definitions
		*	\sa UNITS_TICKS, UNITS_MICROSECONDS, UNITS_NANOSECONDS
		*   \eh Nothing is reported to the error handler.
		*/
		void SetUnits(unsigned int unitsPreset);

		/** \brief Set only the value of time measure
		*	\param [in] _value value of time measure
		*   \eh Nothing is reported to the error handler.
		*/
		void SetValue(TInt64 _value);

		/** \brief Set value of time measure, specifying time units
		*	\param [in] _value value of time measure
		*	\param [in] unitsPreset one of the preset time units definitions
		*	\sa UNITS_TICKS, UNITS_MICROSECONDS, UNITS_NANOSECONDS
		*   \eh Nothing is reported to the error handler.
		*/
		void SetValue(TInt64 _value, unsigned int unitsPreset);

		/** \brief Get time measure value
		*	\retval value time measure value
		*   \eh Nothing is reported to the error handler.
		*/
		TInt64 GetValue() const;

		/** \brief Get time units
		*	\retval units one of the preset time units definitions
		*	\sa UNITS_TICKS, UNITS_MICROSECONDS, UNITS_NANOSECONDS
		*   \eh Nothing is reported to the error handler.
		*/
		unsigned int GetUnits() const;

		/** \brief Convert the value of a measure from ticks to the units set, depending on tick frequency
		*	\param [in] tickFrequency tick frequency of the processor
		*	\retval value value of measure converted from ticks to other units
		*	\pre Time units other than UNITS_TICKS were previously set for this time measure (See \link SetUnits \endlink)
		*   \eh Nothing is reported to the error handler.
		*/
		CTimeMeasure FromTicksToUnits(TInt64 tickFrequency) const;

		/** \brief Substraction of two time measures
		*/
		CTimeMeasure operator-(const CTimeMeasure _rightHand) const;

		/** \brief Addition of two time measures
		*/
		CTimeMeasure operator+(const CTimeMeasure _rightHand) const;

		/** \brief Mark this time measure as invalid (it does not contains an actual time measure)
		*   \eh Nothing is reported to the error handler.
		*/
		void SetInvalid();

		/** \brief Returns false if this was marked as invalid or if this has a negative value
		*	\retval isvalid false if this was marked as invalid or if this has a negative value
		*   \eh Nothing is reported to the error handler.
		*/
		bool IsValid() const;

		/// ATTRIBUTES:
	private:
		TInt64 value;			// Measure as a 64 bit integer
		unsigned int units;		// Units according to presets
	};

	/** \brief Stream output of profiler time measures
	*/
	inline std::ostream & operator<<(std::ostream & out, const CTimeMeasure & t)
	{
		std::string unitShortName;

		// Not sure of the cost of always creating strings for each measure... for this reason, I prefer hardcoding these names here
		switch (t.GetUnits())
		{
		case UNITS_MICROSECONDS:
			//unitShortName = "us";
			unitShortName = "\xe6s";
			break;
		case UNITS_NANOSECONDS:
			unitShortName = "ns";
			break;
		case UNITS_TICKS:
		default:
			unitShortName = " ticks";
			break;
		}

		out << t.GetValue() << unitShortName;
		return out;
	}


	/******************************************************************************/
	/******************************************************************************/

	/** \brief Type definition for profiler sample type
	*/
	enum TSampleType {
		ST_RELATIVE,	///< Relative sample (time elapsed from previous sample)
		ST_ABSOLUTE		///< Absolute sample (as read from SO)
	};

	/** \brief Class with data sets for multi-sample (periodical) profiling
	*/
	class CProfilerDataSet
	{
		// PUBLIC METHODS:
	public:

		/** \brief Default constructor
		*	\details By default, sets sample type to \link ST_RELATIVE \endlink and dataset size to \link DEFAULT_PROFILER_SAMPLES \endlink
		*/
		CProfilerDataSet();

#ifdef PROFILER_USE_VECTOR
		/** \brief Set maximum size of dataset
		*	\details Set maximum number of samples before sampling starts. Static memory must be used to guarantee deterministic time sampling
		*/
		void SetMaximumSize(long _maxDataSize);
#endif

		/** \brief Set sample type for data set
		*	\param [in] _sampleType sample type for data set
		*/
		void SetSampleType(TSampleType _sampleType);

		/** \brief Write dataset into a stream with data converted to proper time units
		*	\param [in] out output stream to write in
		*	\param [in] tickFrequency tick frequency of processor
		*	\pre This may cause an important overhead. Use only AFTER sampling ends!
		*/
		void WriteToStream(std::ostream & out, TInt64 tickFrequency) const;

		/** \brief Write dataset into a file with data converted to proper time units
		*	\param [in] fileName name of file to write in
		*	\param [in] tickFrequency tick frequency of processor
		*	\pre This may cause an important overhead. Use only AFTER sampling ends!
		*/
		void WriteToFile(std::string fileName, TInt64 tickFrequency) const;

		/** \brief Return if sampling process is active
		*	\retval issampling true if sampling process has started but not finished
		*/
		bool IsSampling() const;

		/** \brief Get number of samples already stored
		*	\retval current number of stored samples
		*/
		long GetCurrentSize() const;

		/** \brief Switch on/off automatic write to file
		*	\details Will write all data to file when enough samples are acquired. By default, this feature is switched off, unless this method is explictly called
		*	\param [in] filename name of the file to write in
		*	\param [in] nSamples number of samples to acquire before starting write to file
		*	\param [in] tickFrequency tick frequency of processor
		*	\param [in] setAutomatic switch on/off automatic write (default, true)
		*/
		void SetAutomaticWrite(std::string filename, long nSamples, TInt64 tickFrequency, bool setAutomatic = true);

		// Currently not in use
		void ComputeStatistics();

		// METHODS USED INTERNALLY BY THE PROFILER FOR BUILDING DATASET:
		//friend class CProfiler;	// Gives Profiler access to private methods of CProfilerDataSet
		//private:
		void Start();								// Used internally by profiler
		void AddSample(CTimeMeasure sample);		// Used internally by profiler
		void End();									// Used internally by profiler
		CTimeMeasure GetRelativeStart() const;		// Used internally by profiler
		void SetRelativeStart(CTimeMeasure _time);	// Used internally by profiler

	// ATTRIBUTES:
	private:
		TSampleType sampleType;							// Type of samples in the data set (absolute, relative) 
		CTimeMeasure relativeStart;						// Start time for relative samples
		bool sampling;									// True if dataset is being sampled
		long maxDataSize;								// Maximum number of samples that can be stored in data set
		long dataSize;									// Number of samples actually stored in data set		

		// Automatic write to file
		std::string automaticFileName;		// File name for automatic writing data set to file when full or after filling nSamples
		long nAutomaticSamples;				// Number of samples to store in data set before automatic write to file
		TInt64 automaticTickFrequency;		// Remember tick frequency
		bool isAutomatic;

#if defined(PROFILER_USE_VECTOR)
		std::vector<CTimeMeasure> samples;				// Data set using C++ vectors. Read the comments above
#elif defined(PROFILER_USE_ARRAY)
		CTimeMeasure samples[MAX_PROFILER_SAMPLES];		// Data set using C arrays. Read the comments above 	
#endif

	};


	/******************************************************************************/
	/******************************************************************************/

	// Profiler class
	// TO DO: create one parent and child classes for oneshot and multi-sample profilers?

	/** \details Class with profiling tools for measuring performance of algorithms.
	*	\n This class is a handler of multiple data sets, with centralized processor-wise time units
	*/
	class CProfiler
	{
		// PUBLIC METHODS
	public:

		// General methods:

			/** \brief Access to singleton instance with lazy initialization
			*	\details Use CProfiler::Instance().Method or the mabro PROFILER3DTI.Method() to call any profiler method
			*	\sa PROFILER3DTI
			*/
		static CProfiler& Instance()
		{
			static CProfiler singletonInstance;
			return singletonInstance;
		}


		/** \brief Initialize profiler
		*	\details Sets default resolution and clear all samples, and Setup other platform-dependent stuff.
		*	\n In windows: get QPF (from Vista and newer, this can be read only once)
		*/
		void InitProfiler(std::string externalDataPath = "");

		/** \brief Set resolution (time units) for all time measures
		*	\param [in] unitsPreset one of the preset time units definitions
		*	\sa UNITS_TICKS, UNITS_MICROSECONDS, UNITS_NANOSECONDS
		*/
		void SetResolution(unsigned int unitsPreset);

		/** \brief Get TSC frequency
		*	\details Used while debugging the profiler itself
		*	\retval freq TSC frequency
		*/
		TInt64 GetTSCFrequency() const;

		/** \brief Get resolution (time units) of profiler time measures
		*	\retval unitsPreset one of the preset time units definitions
		*	\sa UNITS_TICKS, UNITS_MICROSECONDS, UNITS_NANOSECONDS
		*/
		unsigned int GetResolution() const;

		/** \brief Write data to file
		*	\param [in] dataSet data set containing the data
		*	\param [in] fileName name of the file to write in
		*/
		void WriteToFile(CProfilerDataSet dataSet, std::string fileName) const;

		/** \brief Switch on/off automatic write to file
		*	\details Will write all data from a data set to file when enough samples are acquired. By default, this feature is switched off, unless this method is explictly called
		*	\param [in,out] dataSet data set containing the data
		*	\param [in] filename name of the file to write in
		*	\param [in] nSamples number of samples to acquire before starting write to file (default, \link DEFAULT_PROFILER_SAMPLES \endlink)
		*	\param [in] setAutomatic switch on/off automatic write (default, true)
		*/
		void SetAutomaticWrite(CProfilerDataSet& dataSet, std::string filename, long nSamples = DEFAULT_PROFILER_SAMPLES, bool setAutomatic = true);

		// One shot profiler:

		/** \brief Get current time measure
		*	\details Absolute one shot profiler
		*	\retval time current time measure
		*/
		CTimeMeasure GetTimeMeasure() const;

		/** Get elapsed time
		*	\details Relative one shot profiler
		*	\param [in] _fromTime reference time measure
		*	\retval time time elapsed from reference time measure to current time
		*/
		CTimeMeasure GetTimeFrom(CTimeMeasure& _fromTime) const;

		// Multi-sample profiler (absolute samples):

		/** \brief Start sampling one data set with absolute time samples
		*	\param [in,out] dataSet data set on which to start sampling
		*/
		void StartAbsoluteSampling(CProfilerDataSet &dataSet) const;

		/** \brief Take one absolute time measure and store it in data set
		*	\param [in,out] dataSet data set on which to store data
		*	\pre Absolute sampling was started in data set (See \link StartAbsoluteSampling \endlink)
		*/
		void TakeAbsoluteSample(CProfilerDataSet &dataSet) const;

		/** \brief Ends sampling one data set, regardless the type (absolute or relative)
		*	\param [in,out] dataSet data set on which to stop sampling
		*/
		void EndSampling(CProfilerDataSet &dataSet) const;

		// Multi-sample profiler (relative samples):

		/** \brief Start sampling one data set with relative time samples
		*	\param [in,out] dataSet data set on which to start sampling
		*/
		void StartRelativeSampling(CProfilerDataSet &dataSet);

		/** \brief Start one relative time measure in data set
		*	\details The current time measure will be obtained and use as reference for this sample
		*	\param [in,out] dataSet data set on which to store data
		*	\pre Relative sampling was started in data set
		*/
		void RelativeSampleStart(CProfilerDataSet &dataSet);

		/** \brief Ends one relative time measure in data set
		*	\details The time measure obtained internally when calling to \link StartRelativeSampling \endlink is used as reference for this sample
		*	\param [in,out] dataSet data set on which to store data
		*	\pre Relative sampling was started in data set (See \link StartRelativeSampling \endlink)
		*	\pre This sample was started (See \link RelativeSampleStart \endlink)
		*/
		void RelativeSampleEnd(CProfilerDataSet &dataSet);

		// HIDDEN METHODS
	protected:

		/** \brief Default constructor
		*	\details Sets the profiler as Not Initialized.
		*/
		CProfiler()
		{
			isInitialized = false;
		}

		///** \brief Destructor
		//*	\details Closes all open files 
		//*/
		//~CProfiler()
		//{
		//}

	// ATTRIBUTES:
	private:
		bool isInitialized;			// Have you called InitProfiler?
		unsigned int resolution;	// Resolution preset	
		TInt64 TSCFrequency;		// Windows specific: tick frequency of your machine, for using QPC
		TInt64 startingSeconds;		// Android/Linux specific: number of seconds in initial call to clock_gettime, to avoid overflow
		std::string dataPath;		// Android specific: path to add for external data storage (to allow file write)
	};
}//end namespace Common
#endif