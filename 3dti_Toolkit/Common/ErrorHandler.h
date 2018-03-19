/**
* \class CErrorHandler
*
* \brief Declaration of CErrorHandler class interface.
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


#ifndef _ERROR_HANDLER_H_
#define _ERROR_HANDLER_H_

#include <string>
#include <mutex>
#include <fstream>

/*! \file */

using namespace std;

/** \brief If SWITCH_ON_3DTI_ERRORHANDLER is undefined, the error handler is completely disabled, causing 0 overhead
*/

#define SWITCH_ON_3DTI_ERRORHANDLER

#ifdef _3DTI_ANDROID_ERRORHANDLER

#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, "3DTI_CORE", __VA_ARGS__))
#define LOGV(...) ((void)__android_log_print(ANDROID_LOG_VERBOSE, "3DTI_CORE", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "3DTI_CORE", __VA_ARGS__))

#define ERRORHANDLER3DTI Common::CErrorHandler::Instance()

#define SET_RESULT(errorID, suggestion) Common::CErrorHandler::Instance().AndroidSetResult(errorID, suggestion, __FILE__, __LINE__)

#define ASSERT(condition, errorID, suggestionError, suggestionOK) Common::CErrorHandler::Instance().AndroidAssertTest(condition, errorID, suggestionError, suggestionOK, __FILE__, __LINE__)

#define WATCH(whichVar, varValue, className) ((void)0)

#define GET_LAST_RESULT() Common::CErrorHandler::Instance().GetLastResult()

#define GET_LAST_RESULT_STRUCT() Common::CErrorHandler::Instance().GetLastResultStruct()

#define GET_FIRST_ERROR_STRUCT() Common::CErrorHandler::Instance().GetFirstErrorStruct()

#define RESET_ERRRORS() Common::CErrorHandler::Instance().ResetErrors()

#endif

#if !defined (SWITCH_ON_3DTI_ERRORHANDLER) && !defined(_3DTI_ANDROID_ERRORHANDLER)

///////////////////////////////////////////////////
/// Dummy Macro definitions 

#define ERRORHANDLER3DTI ((void)0)

#define SET_RESULT(errorID, suggestion) ((void)0)

#define ASSERT(condition, errorID, suggestionError, suggestionOK) ((void)0)

#define WATCH(whichVar, varValue, className) ((void)0)

#define GET_LAST_RESULT() ((void)0)

#define GET_LAST_RESULT_STRUCT() ((void)0)

#define GET_FIRST_ERROR_STRUCT() ((void)0)

#define RESET_ERRORS() ((void)0)

#endif

#if defined(SWITCH_ON_3DTI_ERRORHANDLER)

///////////////////////////////////////////////////
/// Macro definitions for asserts, setting results and watching variables

/** \brief Macro used for easy access to error handler singleton
*/
#define ERRORHANDLER3DTI Common::CErrorHandler::Instance()

/** \brief Macro used by internal classes for reporting results to error handler
*/
#define SET_RESULT(errorID, suggestion) Common::CErrorHandler::Instance().SetResult(errorID, suggestion, __FILE__, __LINE__)

/** \brief Macro used by internal classes for throwing asserts to error handler
*/
#define ASSERT(condition, errorID, suggestionError, suggestionOK) Common::CErrorHandler::Instance().AssertTest(condition, errorID, suggestionError, suggestionOK, __FILE__, __LINE__)

/** \brief Macro used by internal classes to allow watch of internal variables 
*/
#define WATCH(whichVar, varValue, className) Common::CErrorHandler::Instance().Watch<className>(whichVar, varValue)

/** \brief Macro for getting (only the ID of) the last result reported to the error handler
*	\details Please, check in the documentation which methods report errors/warnings to the error handler
*	\retval result ID of last result/error/warning
*/
#define GET_LAST_RESULT() Common::CErrorHandler::Instance().GetLastResult()

/** \brief Macro for getting (the full structure of) the last result reported to the error handler
*	\details Please, check in the documentation which methods report errors/warnings to the error handler
*	\retval resultStruct full structure with information on the last result/error/warning (See \link TResultStruct \endlink)
*/
#define GET_LAST_RESULT_STRUCT() Common::CErrorHandler::Instance().GetLastResultStruct()

/** \brief Macro for getting (the full structure of) the first error reported to the error handler in a block of code
*	\details Please, check in the documentation which methods report errors/warnings to the error handler
*	\pre The starting point of the block of code is marked using \link ResetErrors \endlink
*	\retval errorStruct full structure with information on the first result/error/warning (See \link TResultStruct \endlink)
*/
#define GET_FIRST_ERROR_STRUCT() Common::CErrorHandler::Instance().GetFirstErrorStruct()

/** \brief Macro for doing a reset of last reported result
*/
#define RESET_ERRORS() Common::CErrorHandler::Instance().ResetErrors()

#endif

#if defined(SWITCH_ON_3DTI_ERRORHANDLER) || defined(_3DTI_ANDROID_ERRORHANDLER)

//
// Result/Error data structures
//

/** \brief ID of result reported to the error handler
*/
enum TResultID
{ 
	// No error
	RESULT_OK,						///< No error. Everything went ok

	// General errors
	RESULT_ERROR_UNKNOWN,			///< Unknown error (use only for weird situations, when you don't have any clue of the error source)
	RESULT_ERROR_NOTSET,			///< The value of some parameter was not set
	RESULT_ERROR_BADALLOC,			///< Memory allocation failure
	RESULT_ERROR_NULLPOINTER,		///< Trying to use a pointer which is null
	RESULT_ERROR_DIVBYZERO,			///< Division by zero
	RESULT_ERROR_CASENOTDEFINED,	///< Some case in a switch was not defined (typically, use this for the "default" case of a switch)	
	RESULT_ERROR_PHYSICS,			///< Trying to do something which is not physically correct
	RESULT_ERROR_INVALID_PARAM,     ///< Param value is not valid
	RESULT_ERROR_OUTOFRANGE,		///< Trying to access an array or vector position outside its size
	RESULT_ERROR_BADSIZE,			///< Trying to fill a data structure with a bad data size
	RESULT_ERROR_NOTINITIALIZED,	///< Using or returning a value which is not initialized
	RESULT_ERROR_SYSTEMCALL,		///< A system call returned an error
	RESULT_ERROR_NOTALLOWED,		///< Trying to do something which is not allowed in the current context
	RESULT_ERROR_NOTIMPLEMENTED,	///< A method was defined in the interface for future versions, but it is not implemented yet
	RESULT_ERROR_FILE,				///< Error trying to handle a file
	RESULT_ERROR_EXCEPTION,			///< Exception caught

	// More errors...

	// Warnings
	RESULT_WARNING					///< Description to be specified in suggestion	
};

/** Struct with full information about one error/result/warning
*/
struct TResultStruct
{
	TResultID id;			///< ID of result
	string description;		///< Description of result
	string suggestion;		///< Suggestion for fixing error or further information about result
	string filename;		///< File from which result was reported
	int linenumber;			///< Line number at which result was reported (within filename file)
};

/** \brief Stream output of \link TResultStruct \endlink
*/
inline std::ostream & operator<<(std::ostream & out, const TResultStruct & r)
{
	out << "RESULT #" << r.id << " in File " << r.filename << "(" << r.linenumber << "): " << r.description << " - " << r.suggestion;
	return out;
}

//
// Verbosity modes data structures and presets
//

/*********************************************/

/** \brief Preset verbosity modes
*/
#define VERBOSITY_MODE_SILENT			0	///< Nothing to show
#define VERBOSITYMODE_ERRORSANDWARNINGS	1	///< Show errors and warnings, but not OK results
#define VERBOSITY_MODE_ONLYERRORS		2	///< Show only errors, not OK nor warnings
#define VERBOSITY_MODE_ALL				3	///< Show every type of result: error, warning and OK. Use this with caution, may report a huge amount of information...

/*********************************************/

/** \brief Type definition for verbosity modes
*/
struct TVerbosityMode
{
	bool showErrors;		///< Do show error results
	bool showWarnings;		///< Do show warning results
	bool showOk;			///< Do show OK results

	bool showID;			///< Do show ID of result
	bool showDescription;	///< Do show description of result
	bool showSuggestion;	///< Do show suggestion of result
	bool showFilename;		///< Do show filename of result
	bool showLinenumber;	///< Do show linenumber of result
};

//
// Assert modes 
//

/** \brief Type definition of assert modes
*/
enum TAssertMode	{ASSERT_MODE_EMPTY,		///< Do nothing. Ignore even result reporting. The error handler becomes useless with this setting. For maximum performance, undefine \link SWITCH_ON_3DTI_ERRORHANDLER \endlink
					ASSERT_MODE_CONTINUE,	///< Allow reporting of results, but do nothing with them. Will never terminate program execution
					ASSERT_MODE_ABORT,		///< Abort execution when an ASSERT is evaluated as false. The error will be reported/logged before terminating
					ASSERT_MODE_PARANOID	///< Abort execution if any error is reported to the error handler, even if it was reported using SET_RESULT rather than ASSERT. The error will be reported/logged before terminating
					};


//
// Definitions of variables for variable watcher
//

/** \brief Definition of variables reported to the variable watcher
*	\details This is just an example, you can add here any variables you may need
*/
enum TWatcherVariable	{WV_ANECHOIC_AZIMUTH_LEFT,		///< Azimuth of an audio source for listener left ear
						WV_ANECHOIC_AZIMUTH_RIGHT,		///< Azimuth of an audio source for listener right ear
						WV_ANECHOIC_OUTPUT_LEFT,		///< Left output buffer of anechoic process for one source
						WV_ANECHOIC_OUTPUT_RIGHT,		///< Right output buffer of anechoic process for one source
	                    WV_ENVIRONMENT_OUTPUT_LEFT,		///< Left output buffer of environment process 
						WV_ENVIRONMENT_OUTPUT_RIGHT,	///< Right output buffer of environment process 
						WV_HEARINGLOSS_OUTPUT_LEFT,		///< Left output buffer of hearing loss simulation process 
						WV_HEARINGLOSS_OUTPUT_RIGHT,	///< Right output buffer of hearing loss simulation process 
						WV_HEARINGAID_OUTPUT_LEFT,		///< Left output buffer of hearing aid simulation process 
						WV_HEARINGAID_OUTPUT_RIGHT,		///< Right output buffer of hearing aid simulation process 
						WV_LISTENER_POSITION,			///< Listener position
						// .... Add here your own variable watches... 
						WV_END};

/*********************************************/

namespace Common {

	/** \details Error handler class for error reporting and watching variables
	*	\details Follows Meyers Singleton design pattern
	*/
	class CErrorHandler
	{
	public:
		// PUBLIC METHODS:

			/** \brief Access to singleton instance with lazy initialization
			*	\details Use CErrorHandler::Instance().Method to call any error handler method, or use the defined MACROS instead
			*	\sa SET_RESULT, ASSERT, GET_RESULT, GET_RESULT_STRUCT, GET_FIRST_ERROR_STRUCT, WATCH
			*/
		static CErrorHandler& Instance()
		{
			static CErrorHandler singletonInstance;
			return singletonInstance;
		}

		//
		// Result reporting
		//

		/** \brief Get a struct with the info of the last reported result
		*	\retval resultStruct info of last reported result
		*   \eh Nothing is reported to the error handler.
		*/
		TResultStruct GetLastResultStruct();

		/** \brief Get the ID of the last reported result
		*	\retval result ID of last reported result
		*   \eh Nothing is reported to the error handler.
		*/
		TResultID GetLastResult();

		/** \brief Set result of last operation
		*	\note Instead of calling this method, using the macros \link SET_RESULT \endlink or \link ASSERT \endlink is recommended
		*	\param [in] resultID ID of result
		*	\param [in] suggestion suggestion or further information about result
		*	\param [in] filename file from which result is being reported
		*	\param [in] linenumber line number at which result is being reported (whithin filename file)
		*/
		void SetResult(TResultID resultID, string suggestion, string filename, int linenumber);

#if defined (_3DTI_ANDROID_ERRORHANDLER)
		void AndroidSetResult(TResultID resultID, string suggestion, string filename, int linenumber)
		{
			string newdescription;
			string newsuggestion;
			GetDescriptionAndSuggestion(resultID, newdescription, newsuggestion);

			if (suggestion != "")
				newsuggestion = suggestion;

			if (resultID == RESULT_OK)
				LOGV("OK: %s in file %s (%d)", newsuggestion.c_str(), filename.c_str(), linenumber);
			else
			{
				if (resultID == RESULT_WARNING)
					LOGW("WARNING: %s in file %s (%d)", newsuggestion.c_str(), filename.c_str(), linenumber);
				else
					LOGE("ERROR (%s): %s in file %s (%d)", newdescription.c_str(), newsuggestion.c_str(), filename.c_str(), linenumber);
			}
		}
#endif

		//
		// First error (error reporting in blocks of code)
		//

		/** \brief Inits the first error report, so that the next error will be stored as the first error
		*	\details Used to mark the starting point of the code block
		*   \eh Nothing is reported to the error handler.
		*/
		void ResetErrors();

		/** \brief Get a struct with the info of the first reported error in code block
		*	\retval resultStruct info of first reported error in code block
		*   \eh Nothing is reported to the error handler.
		*/
		TResultStruct GetFirstErrorStruct();

		/** \brief Get the ID of the first reported error in code block
		*	\retval result ID of first reported error in code block
		*   \eh Nothing is reported to the error handler.
		*/
		TResultID GetFirstError();

		//
		// Verbosity modes
		//

		/** \brief Set verbosity mode from one of the presets
		*	\sa VERBOSITY_MODE_SILENT, VERBOSITYMODE_ERRORSANDWARNINGS, VERBOSITY_MODE_ONLYERRORS, VERBOSITY_MODE_ALL
		*   \eh Nothing is reported to the error handler.
		*/
		void SetVerbosityMode(int presetMode);

		/** \brief Set custom verbosity mode
		*	\param [in] _verbosityMode definition of custom verbosity mode
		*   \eh Nothing is reported to the error handler.
		*/
		void SetVerbosityMode(TVerbosityMode _verbosityMode);

		//
		// Logging to file
		//

		/** \brief Enable/disable log of reported results to file, using current verbosity mode
		*	\param [in] filename name of log file
		*	\param [in] logOn switch on/off logging to file (default, true)
		*   \eh Nothing is reported to the error handler.
		*/
		void SetErrorLogFile(string filename, bool logOn = true);

		/** \brief Enable log of reported results to output stream, using current verbosity mode
		*	\param [in] outStream output stream
		*	\param [in] logOn switch on/off logging to stream (default, true)
		*   \eh Nothing is reported to the error handler.
		*/
		void SetErrorLogStream(ostream* outStream, bool logOn = true);

		//
		// Assert modes
		//

		/** \brief Set an assert mode
		*	\details Defines what to do when an error is reported
		*	\param [in] _assertMode one of the preset assert modes
		*	\sa ASSERT_MODE_EMPTY, ASSERT_MODE_CONTINUE, ASSERT_MODE_ABORT, ASSERT_MODE_PARANOID
		*/
		void SetAssertMode(TAssertMode _assertMode);

		/** \brief Test a condition and report error if false, doing the action specified by the assert mode
		*	\details Internally used by the \link ASSERT \endlink macro. Using the macro instead of this method is recommended
		*	\param [in] condition condition to evaluate
		*	\param [in] errorID ID of error to report if condition is evaluated false
		*	\param [in] suggestionError suggestion for reported result struct if condition is evaluated false (result error)
		*	\param [in] suggestionOK suggestion for reported result struct if condition is evaluated true (result OK)
		*	\param [in] filename filename for reported result struct
		*	\param [in] linenumber linenumber for reported result struct
		*/
		void AssertTest(bool condition, TResultID errorID, string suggestionError, string suggestionOK, string filename, int linenumber);

#if defined (_3DTI_ANDROID_ERRORHANDLER)
		void AndroidAssertTest(bool condition, TResultID errorID, string suggestionError, string suggestionOK, string filename, int linenumber)
		{
			if (condition)
			{
				if (suggestionOK != "")
					AndroidSetResult(RESULT_OK, suggestionOK, filename, linenumber);
			}
			else
				AndroidSetResult(errorID, suggestionError, filename, linenumber);
		}
#endif

		//
		// Variable watcher
		//

		/** \brief Add a variable to the list of variables to watch
		*	\param [in] whichVar variable to watch, which has to be added first to the \link TWatcherVariable \endlink enum
		*/
		void AddVariableWatch(TWatcherVariable whichVar);

		/** \brief Remove a variable from the list of variables to watch
		*	\param [in] whichVar which variable to stop watching
		*	\pre Variable was added to watch first
		*/
		void RemoveVariableWatch(TWatcherVariable whichVar);

		/** \brief Enable/disable log to file of a specific watched variable
		*	\param [in] whichVar which variable to log to file
		*	\param [in] filename name of file with the log
		*	\param [in] logOn switch on/off file logging for this war (default, true)
		*	\pre Variable was added to watch first
		*/
		void SetWatcherLogFile(TWatcherVariable whichVar, string filename, bool logOn = true);

		/** \brief Sends the value of a variable to the watcher
		*	\details The value will be recorded ONLY if the variable is on the list of watched variables. No overhead if the variable is not in the list
		*	\param [in] whichVar from which variable are we reporting value
		*	\param [in] varValue the value we are reporting
		*/
		template <class T>
		void Watch(TWatcherVariable whichVar, const T& varValue)
		{
			// Check first if this variable is being watched
			if (!watcherVariables[whichVar])
				return;

			// Log to file
			if (watcherLogFiles[whichVar].is_open())
			{
				watcherLogFiles[whichVar] << varValue << std::endl;
			}
		}


	// PRIVATE METHODS
	protected:

		/* Default constructor
		*	Resets all result and error reporting, sets verbosity mode to \link VERBOSITY_MODE_ONLYERRORS \endlink and assert mode to \link ASSERT_MODE_ABORT \endlink
		*/
		CErrorHandler()
		{
			lastResult.id = RESULT_OK;
			lastResult.filename = "Nobody";
			lastResult.linenumber = -1;
			lastResult.suggestion = "Nothing has been reported to the error handler yet";

			ResetErrors();
			SetVerbosityMode(VERBOSITY_MODE_ONLYERRORS);
			SetAssertMode(ASSERT_MODE_ABORT);
			ResetWatcher();
			logToStream = false;
			errorLogStream = NULL;
		}

		/* Destructor
		*  Closes all open files (error log and watcher)
		*/
		~CErrorHandler()
		{
			if (errorLogFile.is_open())
				errorLogFile.close();
			for (int i = 0; i < WV_END; i++)
			{
				if (watcherLogFiles[i].is_open())
					watcherLogFiles[i].close();
			}
		}

		// Generic method for obtaining description and suggestions of a result ID
		void GetDescriptionAndSuggestion(TResultID result, string& description, string& suggestion);

		// Log to file/stream
		void LogErrorToFile(TResultStruct result);
		void LogErrorToStream(ostream& outStream, TResultStruct result);

		// Reset all watches
		void ResetWatcher();

	private:
		// ATTRIBUTES:

		mutex errorHandlerMutex;

		// Last Result handling
		TResultStruct lastResult;

		// First error (error reporting in blocks of code)
		TResultStruct firstError;

		// Verbosity modes
		// TO DO: we can think in having different modes for different things: log to file, stream output...
		TVerbosityMode verbosityMode;

		// Logging to file/stream
		std::ofstream errorLogFile;
		std::ostream* errorLogStream;
		bool logToStream;

		// Assert modes
		TAssertMode assertMode;

		// Variable watcher
		bool watcherVariables[TWatcherVariable::WV_END];
		std::ofstream watcherLogFiles[TWatcherVariable::WV_END];
		//std::ostream watcherStream;	
	};
}

#endif

#endif 
