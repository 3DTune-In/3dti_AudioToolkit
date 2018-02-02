/*
* \class CProfiler
*
* \brief Definition of CProfiler class.
*
* Class with profiling tools for measuring performance of algorithms
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

// TO DO: Move this to a global .h with platform definitions
#ifndef PLATFORM_DEFINED
#if defined(_WIN32) 
#define PLATFORM_WIN32
#elif defined(_WIN64)
#define PLATFORM_WIN64
#elif defined(__ANDROID_API__)
#define PLATFORM_ANDROID
#endif
#define PLATFORM_DEFINED
#endif

#include <Common/Profiler.h>
#include <Common/ErrorHandler.h>

#if defined(PLATFORM_WIN32) || defined (PLATFORM_WIN64)
#include "Windows.h"
#endif

#ifdef PLATFORM_ANDROID
#define CLOCK_SOURCE CLOCK_PROCESS_CPUTIME_ID
//#define CLOCK_SOURCE CLOCK_MONOTONIC_RAW
// CLOCK_THREAD_CPUTIME_ID could be used, but it is not correctly supported by all Linux kernels (from 2.6.12?)
#ifndef DEBUG_ANDROID
#define DEBUG_ANDROID(...) ((void)__android_log_print(ANDROID_LOG_DEBUG, "AndroidProject1.NativeActivity", __VA_ARGS__))
#endif
#endif

#include<iostream>
#include <fstream>
#include <cmath>

namespace Common {

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Class CTimeMeasure
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		// Set units (default: ticks)
	void CTimeMeasure::SetUnits(unsigned int unitsPreset)
	{
		units = unitsPreset;
	}

	////////////////////////////////////////

		// Set only value
	void CTimeMeasure::SetValue(TInt64 _value)
	{
		value = _value;
	}

	////////////////////////////////////////

		// Set value and units
	void CTimeMeasure::SetValue(TInt64 _value, unsigned int unitsPreset)
	{
		SetUnits(unitsPreset);
		SetValue(_value);
	}

	////////////////////////////////////////

	TInt64 CTimeMeasure::GetValue() const
	{
		return value;
	}

	////////////////////////////////////////

	unsigned int CTimeMeasure::GetUnits() const
	{
		return units;
	}

	////////////////////////////////////////

	CTimeMeasure CTimeMeasure::operator-(const CTimeMeasure _rightHand) const
	{
		CTimeMeasure result;
		result.value = value - _rightHand.value;
		result.units = units;
		return result;
	}

	////////////////////////////////////////

	CTimeMeasure CTimeMeasure::operator+(const CTimeMeasure _rightHand) const
	{
		CTimeMeasure result;
		result.value = value + _rightHand.value;
		result.units = units;
		return result;
	}

	////////////////////////////////////////

		// Convert a measure in number of ticks to the units previously set, depending on tick frequency
	CTimeMeasure CTimeMeasure::FromTicksToUnits(TInt64 tickFrequency) const
	{
		CTimeMeasure result;
		result.units = units;

		TInt64 unitsPerSecond;
		switch (units)
		{
		case UNITS_MICROSECONDS:
			unitsPerSecond = MICROSECONDS_IN_ONE_SECOND;
			break;
		case UNITS_NANOSECONDS:
			unitsPerSecond = NANOSECONDS_IN_ONE_SECOND;
			break;
		case UNITS_TICKS:
		default:
			SET_RESULT(RESULT_WARNING, "Conversion from ticks to ticks in time measure; set units first");
			result.SetInvalid();
			return result;
			break;
		}

		SET_RESULT(RESULT_OK, "Conversion from ticks to units was succesfull");
		result.value = value * unitsPerSecond;
		result.value /= tickFrequency;

		return result;
	}

	////////////////////////////////////////

		// Mark this time measure as invalid (it does not contains an actual time measure)
	void CTimeMeasure::SetInvalid()
	{
		value = -1;
	}

	////////////////////////////////////////

		// Returns false if it was marked as invalid or if it is negative
	bool CTimeMeasure::IsValid() const
	{
		return (value >= 0);
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Class CProfilerDataSet
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		// Constructor. Sets default data size
	CProfilerDataSet::CProfilerDataSet()
	{
		dataSize = 0;
		sampleType = TSampleType::ST_RELATIVE;
		sampling = false;

		relativeStart.SetInvalid();
		isAutomatic = false;

#ifdef PROFILER_USE_ARRAY
		maxDataSize = DEFAULT_PROFILER_SAMPLES;
#else
		SetMaximumSize(DEFAULT_PROFILER_SAMPLES);
#endif
	}

	////////////////////////////////////////

#ifdef PROFILER_USE_VECTOR
	// Set data size (maximum number of samples) before sampling starts. Static memory must be used to guarantee deterministic time sampling
	void CProfilerDataSet::SetMaximumSize(long _maxDataSize)
	{
		maxDataSize = _maxDataSize;

		try
		{
			samples.reserve(maxDataSize);
			SET_RESULT(RESULT_OK, "Maximum size for profiler data set succesfully set");
		}
		catch (std::bad_alloc const&)
		{
			SET_RESULT(RESULT_ERROR_BADALLOC, "Trying to allocate too much memory for profiler data set");
		}
	}
#endif

	////////////////////////////////////////

	void CProfilerDataSet::Start()
	{
		SET_RESULT(RESULT_OK, "Profiler data set started succesfully");
		dataSize = 0;
		sampling = true;

		if (sampleType == TSampleType::ST_RELATIVE)
			relativeStart.SetInvalid();

		// vector.clear is supposed not to affect capacity (only size), but in my own tests capacity is reset!!!!!
#ifdef PROFILER_USE_VECTOR
		long maxSizeBackup = maxDataSize;
		samples.clear();
		SetMaximumSize(maxSizeBackup);
#endif
	}

	////////////////////////////////////////

	void CProfilerDataSet::AddSample(CTimeMeasure sample)
	{
		if (sampling)
		{
			if (dataSize < maxDataSize)
			{
#if defined(PROFILER_USE_ARRAY)
				samples[dataSize] = sample;
#elif defined(PROFILER_USE_VECTOR)
				samples.push_back(sample);
#endif				
				dataSize++;

				if (sampleType == TSampleType::ST_RELATIVE)
					relativeStart.SetInvalid();

				if (isAutomatic)
				{
					if (dataSize >= nAutomaticSamples)
						End();
				}
				// We avoid using error handler while succesfully profiling, to reduce overhead
				//SET_RESULT(RESULT_OK, "Sample added succesfully to profiler data set.");
			}
			else
				SET_RESULT(RESULT_WARNING, "Profiler data set is full. New samples are being ignored");
		}
		else
			SET_RESULT(RESULT_WARNING, "Adding samples to a profiler data set which has not started sampling");
	}

	////////////////////////////////////////

	void CProfilerDataSet::End()
	{
		// error handler: trust in WriteToFile for setting result 
		sampling = false;
		if (isAutomatic)
			WriteToFile(automaticFileName, automaticTickFrequency);
	}

	////////////////////////////////////////

		// True if sampling has started
	bool CProfilerDataSet::IsSampling() const
	{
		// We avoid using error handler while succesfully profiling, to reduce overhead
		//SET_RESULT(RESULT_OK, "");
		return sampling;
	}

	////////////////////////////////////////

		// Number of samples already stored
	long CProfilerDataSet::GetCurrentSize() const
	{
		return dataSize;
	}

	////////////////////////////////////////

	void CProfilerDataSet::SetSampleType(TSampleType _sampleType)
	{
		//SET_RESULT(RESULT_OK, "Sample type for profiler data set succesfully set");
		sampleType = _sampleType;
	}

	////////////////////////////////////////

		// Automatic write to file when dataset is full
	void CProfilerDataSet::SetAutomaticWrite(std::string filename, long nSamples, TInt64 tickFrequency, bool setAutomatic)
	{
		SET_RESULT(RESULT_OK, "Automatic write to file for profiler data set succesfully set");
		automaticFileName = filename;
		nAutomaticSamples = nSamples;
		automaticTickFrequency = tickFrequency;
		isAutomatic = setAutomatic;
	}

	////////////////////////////////////////

		// Write dataset into a stream, depending on sample type
	void CProfilerDataSet::WriteToStream(std::ostream & out, TInt64 tickFrequency) const
	{
		// Generic implementation for both types of data sets (array and vector)

		// Check errors
		if (dataSize <= 0)
		{
			SET_RESULT(RESULT_ERROR_OUTOFRANGE, "Attempt to read a profiler data set which was not previously sampled");
			return;
		}
		else if (sampling)
		{
			SET_RESULT(RESULT_ERROR_NOTALLOWED, "Writing a profiler data set to file breaks determinism while sampling. Please, end sampling first");
			return;
		}
		else
			SET_RESULT(RESULT_OK, "Profiler data set written to stream succesfully");

		// Write samples
		if (sampleType == TSampleType::ST_ABSOLUTE)
		{
			for (int i = 1; i < dataSize; i++)
			{
				out << samples[i].FromTicksToUnits(tickFrequency).GetValue() << endl;
			}
		}
		else	// RELATIVE SAMPLES
		{
			for (int i = 1; i < dataSize; i++)
			{
				out << samples[i].GetValue() << endl;
			}
		}
	}

	////////////////////////////////////////

		// The same, but writing to file (caller does not need to worry about fstreams...)
	void CProfilerDataSet::WriteToFile(string fileName, TInt64 tickFrequency) const
	{
		// TO DO: error handler
		ofstream dataSetFile;
		dataSetFile.open(fileName);
		WriteToStream(dataSetFile, tickFrequency);
		dataSetFile.close();
		SET_RESULT(RESULT_WARNING, "Profiler wrote dataset to file" + fileName);
	}

	////////////////////////////////////////

		// Currently not in use
	void CProfilerDataSet::ComputeStatistics()
	{
		// TO DO: IF WE WANT TO USE THIS METHOD, WE SHOULD GIVE ANY OUTPUT OF THESE LOCAL VARS!!!!!
		CTimeMeasure worst;
		CTimeMeasure average;
		CTimeMeasure deviation;
		//

		// Init statistics
		worst.SetValue(0);
		average.SetValue(0);
		deviation.SetValue(0);

		// Average and worst case
		for (int i = 0; i < dataSize; i++)
		{
			// Check for worst
			if (samples[i].GetValue() > worst.GetValue())
				worst = samples[i];

			// Add to average
			average = average + samples[i];
		}
		// Divide average by number of samples
		average.SetValue(average.GetValue() / dataSize);

		// Standard deviation				
		for (int i = 0; i < dataSize; i++)
		{
			// Add one sample to deviation
			TInt64 difference = samples[i].GetValue() - average.GetValue();
			deviation.SetValue(deviation.GetValue() + difference*difference);
		}
		// Square root of summation
		deviation.SetValue(std::sqrt(deviation.GetValue() / dataSize));
	}

	////////////////////////////////////////

	CTimeMeasure CProfilerDataSet::GetRelativeStart() const
	{
		return relativeStart;
	}

	////////////////////////////////////////

	void CProfilerDataSet::SetRelativeStart(CTimeMeasure _time)
	{
		relativeStart = _time;
	}


	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Class CProfiler
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////
	// GENERAL METHODS

#ifdef PLATFORM_ANDROID
// Set default resolution. Setup other platform-dependent stuff. 
// In Android, set external data path and set initial number of seconds
	void CProfiler::InitProfiler(std::string externalDataPath)
	{
		//SetResolution(UNITS_MICROSECONDS);				
		SetResolution(UNITS_NANOSECONDS);
		//isAutomatic = false;

		dataPath = externalDataPath;

		struct timespec now;
		if (clock_gettime(CLOCK_SOURCE, &now) != 0)
		{
			DEBUG_ANDROID("Error in call to clock_gettime!");
			SET_RESULT(RESULT_ERROR_SYSTEMCALL, "Could not setup high-performance system timer for profiling (ANDROID platform)");
			startingSeconds = 0;
		}
		else
		{
			SET_RESULT(RESULT_OK, "Profiler was initalized succesfully");
			startingSeconds = (TInt64)now.tv_sec;
			isInitialized = true;
		}
	}

#else
	// Set default resolution. Setup other platform-dependent stuff. 
	// In windows, get QPF (from Vista and newer, this can be read only once)
	void CProfiler::InitProfiler(string externalDataPath)
	{
		//SetResolution(UNITS_MICROSECONDS);				
		SetResolution(UNITS_NANOSECONDS);
		//isAutomatic = false;

#if defined(PLATFORM_WIN32) || defined (PLATFORM_WIN64)
		LARGE_INTEGER qfc;
		if (!QueryPerformanceFrequency(&qfc))
			SET_RESULT(RESULT_ERROR_SYSTEMCALL, "Error in QueryPerformanceFrequency (WINDOWS platform)");	// This should never happen
		else
		{
			SET_RESULT(RESULT_OK, "Profiler was initalized succesfully");
			isInitialized = true;
		}

		TSCFrequency = qfc.QuadPart;	// This cast seems to work ok
#endif
	}
#endif

	////////////////////////////////////////

		// Set resolution for all time measures
	void CProfiler::SetResolution(unsigned int unitsPreset)
	{
		if ((unitsPreset == UNITS_MICROSECONDS) || (unitsPreset == UNITS_NANOSECONDS))
			SET_RESULT(RESULT_OK, "Resolution for profiler succesfully set");
		else
			SET_RESULT(RESULT_WARNING, "Profiler should use only microseconds or nanoseconds resolution, not ticks");
		resolution = unitsPreset;
	}

	////////////////////////////////////////

		// Just for debugging the profiler itself
	TInt64 CProfiler::GetTSCFrequency() const
	{
		if (isInitialized)
			return TSCFrequency;
		else
		{
			SET_RESULT(RESULT_ERROR_NOTINITIALIZED, "Profiler is not initialized. Please, call to InitProfiler before using profiler");
			return 0;
		}
	}

	////////////////////////////////////////

		// Just for debugging the profiler itself
	unsigned int CProfiler::GetResolution() const
	{
		if (isInitialized)
			return resolution;
		else
		{
			SET_RESULT(RESULT_ERROR_NOTINITIALIZED, "Profiler is not initialized. Please, call to InitProfiler before using profiler");
			return 0;
		}
	}


	////////////////////////////////////////
	// ONE SHOT PROFILER

		// Get current time
	CTimeMeasure CProfiler::GetTimeMeasure() const
	{
		CTimeMeasure currentTime;
		currentTime.SetInvalid();

		if (isInitialized)
		{
#if defined(PLATFORM_WIN32) || defined (PLATFORM_WIN64)
			// NOTE: SO must be Windows Vista or newer for performant and reliable (between CPUs) time measures using QPC

			LARGE_INTEGER now;
			if (!QueryPerformanceCounter(&now))
			{
				SET_RESULT(RESULT_ERROR_SYSTEMCALL, "Error in QueryPerformanceCounter (WINDOWS platform)");	// This should never happen in XP or above
				currentTime.SetInvalid();
				return currentTime;
			}
			else
			{
				// We avoid using error handler while succesfully profiling, to reduce overhead
				//SET_RESULT(RESULT_OK, "");
			}

			currentTime.SetValue(now.QuadPart, resolution);	// This cast seems to work ok	

#elif defined(PLATFORM_ANDROID)	
			struct timespec now;
			if (clock_gettime(CLOCK_SOURCE, &now) != 0)
			{
				DEBUG_ANDROID("Error in call to clock_gettime!");
				//SET_RESULT(RESULT_ERROR_SYSTEMCALL, "Could not setup high-performance system timer for profiling (ANDROID platform)");
				currentTime.SetInvalid();
				return currentTime;
			}

			TInt64 newValue = ((TInt64)now.tv_sec - startingSeconds) * NANOSECONDS_IN_ONE_SECOND + (TInt64)now.tv_nsec;
			currentTime.SetValue(newValue, UNITS_NANOSECONDS); // NOTE: currently, it is forced to be nanoseconds			
#endif
		}
		else
			SET_RESULT(RESULT_ERROR_NOTINITIALIZED, "Profiler is not initialized. Please, call to InitProfiler before using profiler");

		return currentTime;
	}

	////////////////////////////////////////

		// Get elapsed time 
	CTimeMeasure CProfiler::GetTimeFrom(CTimeMeasure& _fromTime) const
	{
		CTimeMeasure elapsedTime;
		elapsedTime.SetInvalid();

		if (isInitialized)
		{
#if defined(PLATFORM_WIN32) || defined (PLATFORM_WIN64)
			CTimeMeasure currentTime = GetTimeMeasure();
			elapsedTime = currentTime - _fromTime;

			elapsedTime = elapsedTime.FromTicksToUnits(TSCFrequency);	// error handler: we trust in this call for setting result
#else			
			CTimeMeasure currentTime = GetTimeMeasure();
			elapsedTime = currentTime - _fromTime;
#endif
		}
		else
			SET_RESULT(RESULT_ERROR_NOTINITIALIZED, "Profiler is not initialized. Please, call to InitProfiler before using profiler");

		return elapsedTime;
	}


	////////////////////////////////////////
	// MULTI-SAMPLE PROFILER ABSOLUTE

		// Starts sampling absolute time measures
	void CProfiler::StartAbsoluteSampling(CProfilerDataSet &dataSet) const
	{
		if (isInitialized)
		{
			if (dataSet.IsSampling())
			{
				SET_RESULT(RESULT_ERROR_NOTALLOWED, "Profiling was already started on this dataset");
				return;
			}
			else
				SET_RESULT(RESULT_OK, "Absolute sampling in profiler started succesfully");

			dataSet.SetSampleType(TSampleType::ST_ABSOLUTE);
			dataSet.Start();
			//dataSet.AddSample(GetTimeMeasure());	// Give first sample for post-processing relative samples
		}
		else
			SET_RESULT(RESULT_ERROR_NOTINITIALIZED, "Profiler is not initialized. Please, call to InitProfiler before using profiler");
	}

	////////////////////////////////////////

		// Take one (absolute) sample between Start and End
	void CProfiler::TakeAbsoluteSample(CProfilerDataSet &dataSet) const
	{
		// TO DO: Check dataset sample type

		if (isInitialized)
		{
			if (!dataSet.IsSampling())
				SET_RESULT(RESULT_ERROR_NOTALLOWED, "Attempt to write a sample to a dataset without starting sampling first");
			else
			{
				// We avoid using error handler while succesfully profiling, to reduce overhead
				//SET_RESULT(RESULT_OK, "");
			}

			dataSet.AddSample(GetTimeMeasure());
		}
		else
			SET_RESULT(RESULT_ERROR_NOTINITIALIZED, "Profiler is not initialized. Please, call to InitProfiler before using profiler");
	}

	////////////////////////////////////////

		// Ends sampling (bot for absolute and relative samples)	
	void CProfiler::EndSampling(CProfilerDataSet &dataSet) const
	{
		if (isInitialized)
		{
			if (!dataSet.IsSampling())
				SET_RESULT(RESULT_WARNING, "Ending sampling for a dataset which was not sampling");
			else
				SET_RESULT(RESULT_OK, "Sampling data set in profiler ended succesfully");

			dataSet.End();
		}
		else
			SET_RESULT(RESULT_ERROR_NOTINITIALIZED, "Profiler is not initialized. Please, call to InitProfiler before using profiler");
	}


	////////////////////////////////////////
	// MULTI-SAMPLE PROFILER RELATIVE

		// Starts sampling one data set with relative samples
	void CProfiler::StartRelativeSampling(CProfilerDataSet &dataSet)
	{
		if (isInitialized)
		{
			if (dataSet.IsSampling())
			{
				//SET_RESULT(RESULT_ERROR_NOTALLOWED, "Profiling was already started on this dataset");
				return;
			}
			else
				SET_RESULT(RESULT_OK, "Sampling data set in profiler started succesfully");

			//relativeSampleStart.SetInvalid();
			dataSet.SetSampleType(TSampleType::ST_RELATIVE);
			dataSet.Start();
		}
		else
			SET_RESULT(RESULT_ERROR_NOTINITIALIZED, "Profiler is not initialized. Please, call to InitProfiler before using profiler");
	}

	////////////////////////////////////////

		// Starts one relative sample
	void CProfiler::RelativeSampleStart(CProfilerDataSet &dataSet)
	{
		if (isInitialized)
		{
			//if (relativeSampleStart.IsValid())
			//	SET_RESULT(RESULT_WARNING, "This relative sample was already started. Previous start point will be ignored.");
			//else
			//	SET_RESULT(RESULT_OK, "");

			//relativeSampleStart = GetTimeMeasure();
			dataSet.SetRelativeStart(GetTimeMeasure());
		}
		else
			SET_RESULT(RESULT_ERROR_NOTINITIALIZED, "Profiler is not initialized. Please, call to InitProfiler before using profiler");
	}

	////////////////////////////////////////

		// Ends one relative sample
	void CProfiler::RelativeSampleEnd(CProfilerDataSet &dataSet)
	{
		if (isInitialized)
		{
			if (!dataSet.IsSampling())
			{
				//SET_RESULT(RESULT_ERROR_NOTALLOWED, "Attempt to write a sample to a dataset without starting sampling first.");
				return;
			}
			else if (!dataSet.GetRelativeStart().IsValid())
			{
				SET_RESULT(RESULT_ERROR_NOTALLOWED, "Attempt to write a relative sample without establishing reference first (please, use RelativeSampleStart)");
				return;
			}
			else
			{
				// We avoid using error handler while succesfully profiling, to reduce overhead
				//SET_RESULT(RESULT_OK, "");
			}

			CTimeMeasure relativeStart = dataSet.GetRelativeStart();
			dataSet.AddSample(GetTimeFrom(relativeStart));

			//relativeSampleStart.SetInvalid();

			// Automatic write to file
			//if (isAutomatic)
			//{
			//	if (dataSet.GetCurrentSize() >= nAutomaticSamples)
			//	{
			//		dataSet.End();
			//		dataSet.WriteToFile(automaticFileName, TSCFrequency);
			//	}
			//}
		}
		else
			SET_RESULT(RESULT_ERROR_NOTINITIALIZED, "Profiler is not initialized. Please, call to InitProfiler before using profiler");
	}

	////////////////////////////////////////

		// Write data to file
	void CProfiler::WriteToFile(CProfilerDataSet dataSet, string fileName) const
	{
		if (isInitialized)
		{
			// We trust in dataset WriteToFile for setting error handler
			dataSet.WriteToFile(fileName, TSCFrequency);
		}
		else
			SET_RESULT(RESULT_ERROR_NOTINITIALIZED, "Profiler is not initialized. Please, call to InitProfiler before using profiler");
	}

	////////////////////////////////////////

	void CProfiler::SetAutomaticWrite(CProfilerDataSet& dataSet, std::string filename, long nSamples, bool setAutomatic)
	{
		if (isInitialized)
		{
			// We trust in dataset SetAutomaticWrite for setting error handler
			//isAutomatic = setAutomatic;
			//nAutomaticSamples = nSamples;
			//automaticFileName = filename;
#ifdef PLATFORM_ANDROID
			dataSet.SetAutomaticWrite(dataPath + filename, nSamples, TSCFrequency, setAutomatic);
#else
			dataSet.SetAutomaticWrite(filename, nSamples, TSCFrequency, setAutomatic);
#endif
		}
		else
			SET_RESULT(RESULT_ERROR_NOTINITIALIZED, "Profiler is not initialized. Please, call to InitProfiler before using profiler");
	}
}//end namespace Common
