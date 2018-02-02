/*
* \class CErrorHandler
*
* \brief Definition of CErrorHandler class.
*
* Error handler class for error reporting and watching variables.
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

#include <Common/ErrorHandler.h>

#if defined (SWITCH_ON_3DTI_ERRORHANDLER) || defined(_3DTI_ANDROID_ERRORHANDLER)

namespace Common {

	//////////////////////////////////////////////////////////
	// Last result reporting

		// Generic method for obtaining description and suggestions of a result ID
	void CErrorHandler::GetDescriptionAndSuggestion(TResultID result, string& description, string& suggestion)
	{
		// Set specific strings for each error type. Suggestions are generic and might be replaced with the one specified when calling to SetResult
		switch (result)
		{
		case RESULT_OK: { description = "OK"; suggestion = "Nothing to do"; break;  }
		case RESULT_ERROR_UNKNOWN: { description = "Unknown error"; suggestion = "There are no specific details about this error type"; break;  }
		case RESULT_ERROR_NOTSET: { description = "Value not set"; suggestion = "Tried to use a parameter and its value was not set"; break;  }
		case RESULT_ERROR_BADALLOC: { description = "Memory allocation failure"; suggestion = "Bad alloc exception thrown using New"; break;  }
		case RESULT_ERROR_NULLPOINTER: { description = "Null pointer"; suggestion = "Attempt to use a null pointer"; break;  }
		case RESULT_ERROR_DIVBYZERO: { description = "Division by zero"; suggestion = ""; break;  }
		case RESULT_ERROR_CASENOTDEFINED: { description = "Case not defined"; suggestion = "A switch statement went through an unexpected default case"; break;  }
		case RESULT_ERROR_PHYSICS: { description = "Violation of physics"; suggestion = "You tried to do something which is not physically correct"; break;  }
		case RESULT_ERROR_OUTOFRANGE: { description = "Out of range"; suggestion = "Trying to access an array or vector position outside its size"; break;  }
		case RESULT_ERROR_BADSIZE: { description = "Bad size"; suggestion = "Trying to fill a data structure with a bad size"; break;  }
		case RESULT_ERROR_NOTINITIALIZED: { description = "Not initialized"; suggestion = "Using or returning a value which was not initialized"; break;  }
		case RESULT_ERROR_INVALID_PARAM: { description = "Invalid parameter"; suggestion = "One or more parameters passed to a method have an incorrect value"; break;  }
		case RESULT_ERROR_SYSTEMCALL: { description = "Error in System Call"; suggestion = "Some platform-specific system call returned an error"; break;  }
		case RESULT_ERROR_NOTALLOWED: { description = "Not allowed"; suggestion = "Attempt to do something which is not allowed in the current context"; break;  }
		case RESULT_ERROR_NOTIMPLEMENTED: { description = "Not implemented yet"; suggestion = "Call to a method not implemented yet in this version of the toolkit core"; break;  }
		case RESULT_ERROR_FILE: { description = "File handling error"; suggestion = "Wrong attempt to open, read or write a file"; break;  }
		case RESULT_ERROR_EXCEPTION: { description = "Exception cuaght"; suggestion = "An exception was thrown and caught"; break;  }
		case RESULT_WARNING: { description = "Warning!"; suggestion = "This is not an error, only a warning"; break;  }
		default: { description = "Unknown error type"; suggestion = "The error handler was not properly used for setting result"; }
		}
	}

	//////////////////////////////////////////////////////////

	// Returns a struct with the last result info
	TResultStruct CErrorHandler::GetLastResultStruct()
	{
		return lastResult;
	}

	//////////////////////////////////////////////////////////

	// Returns just the ID of the last error
	TResultID CErrorHandler::GetLastResult()
	{
		return lastResult.id;
	}

	//////////////////////////////////////////////////////////

	// Set error value of last operation
	void CErrorHandler::SetResult(TResultID resultID, string suggestion, string filename, int linenumber)
	{
		if (assertMode != ASSERT_MODE_EMPTY)	// Alternative: put this before logging to file
		{
			lock_guard<mutex> lock(errorHandlerMutex);

			// Set result struct

			lastResult.id = resultID;
			lastResult.linenumber = linenumber;
			lastResult.filename = filename;

			// Set specific strings for each result type. Suggestions are generic and might be replaced with the one specified
			string defaultDescription, defaultSuggestion;
			GetDescriptionAndSuggestion(lastResult.id, defaultDescription, defaultSuggestion);
			lastResult.description = defaultDescription;

			// Replace default suggestion with the provided one, if it was specified
			if (suggestion != "")
				lastResult.suggestion = suggestion;
			else
				lastResult.suggestion = defaultSuggestion;

			// For filename, remove the path (WARNING! This may be platform-dependent)
			const size_t last_slash = lastResult.filename.find_last_of("\\/");
			if (std::string::npos != last_slash)
				lastResult.filename.erase(0, last_slash + 1);

			// SET FIRST ERROR 
			if (resultID != RESULT_OK)
			{
				if (firstError.id == RESULT_OK)
				{
					firstError = lastResult;
				}
			}

			// LOG TO FILE
			if (errorLogFile.is_open())
				LogErrorToFile(lastResult);

			// LOG TO STREAM
			if (logToStream)
				LogErrorToStream(*errorLogStream, lastResult);

			// TERMINATE PROGRAM IF ERROR IN PARANOID MODE 
			// TO THINK: Do we include RESULT_WARNING here????
			if ((lastResult.id != RESULT_OK) && (assertMode == ASSERT_MODE_PARANOID))
			{
				std::terminate();
			}
		}
	}

	//////////////////////////////////////////////////////////
	// First error reporting

	// Changes the state, so that the next error will be stored as the first error
	void CErrorHandler::ResetErrors()
	{
		if (assertMode != ASSERT_MODE_EMPTY)
		{
			string description, suggestion;
			firstError.id = RESULT_OK;
			GetDescriptionAndSuggestion(firstError.id, description, suggestion);
			firstError.description = description;
			firstError.suggestion = suggestion;
			firstError.filename = "Nobody";
			firstError.linenumber = -1;
		}
	}

	//////////////////////////////////////////////////////////

	// Returns a struct with the first error info
	TResultStruct CErrorHandler::GetFirstErrorStruct()
	{
		return firstError;
	}

	//////////////////////////////////////////////////////////

	// Returns just the ID of the first error
	TResultID CErrorHandler::GetFirstError()
	{
		return firstError.id;
	}

	//////////////////////////////////////////////////////////
	// Verbosity modes

	// Set verbosity mode from one of the presets
	void CErrorHandler::SetVerbosityMode(int presetMode)
	{
		// By default, all presets show all attributes of the error handler result
		verbosityMode.showID = true;
		verbosityMode.showDescription = true;
		verbosityMode.showSuggestion = true;
		verbosityMode.showFilename = true;
		verbosityMode.showLinenumber = true;

		// What type of results to show, depending on preset
		switch (presetMode)
		{
		case VERBOSITY_MODE_SILENT:
			verbosityMode.showErrors = false;
			verbosityMode.showOk = false;
			verbosityMode.showWarnings = false;
			//SET_RESULT(RESULT_OK, "Verbosity mode changed to Silent.");	// actually, setting this result is nonsense :)
			break;
		case VERBOSITY_MODE_ONLYERRORS:
			verbosityMode.showErrors = true;
			verbosityMode.showOk = false;
			verbosityMode.showWarnings = false;
			//SET_RESULT(RESULT_OK, "Verbosity mode changed to Only Errors.");	// actually, setting this result is nonsense :)
			break;
		case VERBOSITY_MODE_ALL:
			verbosityMode.showErrors = true;
			verbosityMode.showOk = true;
			verbosityMode.showWarnings = true;
			//SET_RESULT(RESULT_OK, "Verbosity mode changed to All.");
			break;
		case VERBOSITYMODE_ERRORSANDWARNINGS:
			verbosityMode.showErrors = true;
			verbosityMode.showOk = false;
			verbosityMode.showWarnings = true;
			//SET_RESULT(RESULT_OK, "Verbosity mode changed to Errors and Warnings.");
			break;
		default:
			verbosityMode.showErrors = false;
			verbosityMode.showOk = false;
			verbosityMode.showWarnings = false;
			//SET_RESULT(RESULT_ERROR_CASENOTDEFINED, "Preset not found for verbosity mode.");
			break;
		}
	}

	//////////////////////////////////////////////////////////

	// Set custom verbosity mode
	void CErrorHandler::SetVerbosityMode(TVerbosityMode _verbosityMode)
	{
		verbosityMode = _verbosityMode;
	}

	//////////////////////////////////////////////////////////
	// Logging to file

	// Enable/disable log to file, using current verbosity mode (Optional: include verbosity mode as a parameter?)	
	void CErrorHandler::SetErrorLogFile(string filename, bool logOn)
	{
		// TO DO: check errors!

		if (errorLogFile.is_open())
			errorLogFile.close();

		if (logOn)
		{
			errorLogFile.open(filename, std::ofstream::out | std::ofstream::app);	// Using append, we allow enabling/disabling log to the same file in runtime
			// TO DO: Put a text header in log file each time you open it? (for example, with a time stamp, but this might be platform-dependent)
		}
	}

	//////////////////////////////////////////////////////////

	void CErrorHandler::SetErrorLogStream(ostream* outStream, bool logOn)
	{
		errorLogStream = outStream;
		logToStream = logOn;
	}

	//////////////////////////////////////////////////////////

	// Log to file
	void CErrorHandler::LogErrorToFile(TResultStruct result)
	{
		LogErrorToStream(errorLogFile, result);
	}

	//////////////////////////////////////////////////////////

	// Log to stream
	void CErrorHandler::LogErrorToStream(ostream& outStream, TResultStruct result)
	{
		// Return if we want to log an OK in a verbosity mode not logging OK
		if ((!verbosityMode.showOk) && (result.id == RESULT_OK))
			return;

		// Return if we want to log an error in a verbosity mode not logging errors
		if ((!verbosityMode.showErrors) && (result.id != RESULT_OK))
			return;

		// Return if we want to log a warning in a verbosity mode not logging warnings
		if ((!verbosityMode.showWarnings) && (result.id == RESULT_WARNING))
			return;

		// Go ahead with loggin in any other case
		// TO DO: coherent text, brackets, etc for custom verbosity modes
		if (verbosityMode.showID)
		{
			if (result.id == RESULT_OK)
				outStream << "    OK";	// We put spaces to clearly spot errors at first sight
			else
			{
				if (result.id == RESULT_WARNING)
					outStream << "  Warning";
				else
					outStream << "ERROR #" << result.id;
			}
		}
		if (verbosityMode.showFilename)
		{
			outStream << " in " << result.filename << " (";
		}
		if (verbosityMode.showLinenumber)
		{
			outStream << result.linenumber << "): ";
		}
		if (verbosityMode.showDescription)
		{
			outStream << result.description;
		}
		if (verbosityMode.showSuggestion)
		{
			outStream << " - " << result.suggestion;
		}
		outStream << std::endl;
	}

	//////////////////////////////////////////////////////////
	// Assert modes

	// Sets an assert mode. Defines what to do when an error happens
	void CErrorHandler::SetAssertMode(TAssertMode _assertMode)
	{
		assertMode = _assertMode;
		if (assertMode == ASSERT_MODE_EMPTY)
		{
			lastResult.id = RESULT_OK;
			lastResult.description = "No results";
			lastResult.suggestion = "Assert mode is empty; results are not being reported.";
			lastResult.filename = "";
			lastResult.linenumber = -1;
			firstError = lastResult;
		}
	}

	//////////////////////////////////////////////////////////

	// Test a condition and generate error if false, doing the action specified by the assert mode
	void CErrorHandler::AssertTest(bool condition, TResultID errorID, string suggestionError, string suggestionOK, string filename, int linenumber)
	{
		if (assertMode != ASSERT_MODE_EMPTY)
		{
			if (condition)
			{
				if (suggestionOK != "")
					SetResult(RESULT_OK, suggestionOK, filename, linenumber);
			}
			else
			{
				SetResult(errorID, suggestionError, filename, linenumber);

				if (assertMode == ASSERT_MODE_ABORT)
				{
					std::terminate();
				}
			}
		}
	}

	//////////////////////////////////////////////////////////
	// Variable watcher

	// Reset all watches
	void CErrorHandler::ResetWatcher()
	{
		for (int i = 0; i < WV_END; i++)
		{
			watcherVariables[i] = false;
		}
	}

	//////////////////////////////////////////////////////////

	// Add a variable to the list of variables to watch
	void CErrorHandler::AddVariableWatch(TWatcherVariable whichVar)
	{
		watcherVariables[whichVar] = true;
	}

	//////////////////////////////////////////////////////////

	// Remove a variable from the list of variables to watch
	void CErrorHandler::RemoveVariableWatch(TWatcherVariable whichVar)
	{
		watcherVariables[whichVar] = false;
	}

	//////////////////////////////////////////////////////////

	// Enable/disable log to file of watched variables
	void CErrorHandler::SetWatcherLogFile(TWatcherVariable whichVar, string filename, bool logOn)
	{
		// TO DO: check errors!

		if (watcherLogFiles[whichVar].is_open())
			watcherLogFiles[whichVar].close();

		if (logOn)
		{
			watcherLogFiles[whichVar].open(filename, std::ofstream::out | std::ofstream::app);	// Using append, we allow enabling/disabling log to the same file in runtime
																								// TO DO: Put a text header in log file each time you open it? (for example, with a time stamp, but this might be platform-dependent)			
		}
	}

	//////////////////////////////////////////////////////////

		// Enable/disable watcher output to stream
		//void CErrorHandler::SetWatchStream(std::ostream stream, bool streamOn)
		//{
		//}

	//////////////////////////////////////////////////////////

}//end namespace Common

#endif