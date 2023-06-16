 /**
 * \class CCore
 *
 * \brief Declaration of CCore interface.
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
#ifndef _CCORE_H_
#define _CCORE_H_

#include <BinauralSpatializer/HRTF.h>
#include <BinauralSpatializer/BRIR.h>
#include <Common/Transform.h>
#include <Common/AudioState.h>
#include <BinauralSpatializer/SingleSourceDSP.h>
#include <Common/Magnitudes.h>
#include <Common/AIR.h>
#include <Common/Buffer.h>
#include <Common/Fprocessor.h>
#include <Common/CommonDefinitions.h>
#include <vector>
#include <memory>

using namespace std;  //TODO: Try to avoid this

namespace Binaural {

    class CSingleSourceDSP;
    class CListener;
    class CEnvironment;
	class CHRTF;
    
/** \details Class for centralization of all funtionalities of the binaural spatializer, such as handling sound sources, audio state, listener and environment.
*/
class CCore
{
public:

	/** \brief Constructor with parameters
	*	\param [in] _audioState audio state configuration 
	*	\param [in] _HRTF_resamplingStep HRTF resampling step, in degrees
	*   \eh Nothing is reported to the error handler.
	*/
	CCore(Common::TAudioStateStruct _audioState, int _HRTF_resamplingStep);	

	/** \brief Default constructor 
	*	\details By default, sets sampling frequency to 44100Hz, buffer size to 512, and HRTF resampling step to 5 degrees. 
	*   \eh Nothing is reported to the error handler.
	*/
	CCore();

	/** \brief Get global audio state
	*	\retval audioState currently set audio state
	*   \eh Nothing is reported to the error handler.
	*/
	Common::TAudioStateStruct GetAudioState() const;

	/** \brief Set global audio state
	*	\param [in] _audioState audio state to set
	*   \eh Nothing is reported to the error handler.
	*/
	void SetAudioState(Common::TAudioStateStruct _audioState);

	/** \brief Set HRTF resampling step
	*	\param [in] _HRTF_resamplingStep new HRTF resampling step, in degrees
	*   \eh On error, an error code is reported to the error handler.
	*/
	void SetHRTFResamplingStep(int _HRTF_resamplingStep);

	/** \brief Get HRTF resampling stemp
	*	\retval HRTF_resamplingStep currently set HRTF resampling step, in degrees
	*   \eh Nothing is reported to the error handler.
	*/
	int GetHRTFResamplingStep();

	/** \brief Get global physical magnitudes
	*	\retval magnitudes currently set physical magnitudes
	*   \eh Nothing is reported to the error handler.
	*/
	Common::CMagnitudes GetMagnitudes() const;

	/** \brief Set global physical magnitudes
	*	\param [in] _magnitudes physical magnitudes to set
	*   \eh Nothing is reported to the error handler.
	*/
	void SetMagnitudes(Common::CMagnitudes _magnitudes);	
	
	///////////////////////
	// Listener methods
	///////////////////////

    /** \brief Creates a new listener
	*	\param [in] listenerHeadRadius listener head radius in meters (defaults to 0.0875)
     *	\retval listener shared pointer to newly created listener with empty HRTF.
	 *   \eh On success, RESULT_OK is reported to the error handler.
	 *       On error, an error code is reported to the error handler.
     */
    shared_ptr<CListener> CreateListener(float listenerHeadRadius = 0.0875f);
    
	/** \brief Get Core listener	
	*	\retval listener listener object
	*   \eh On error, an error code is reported to the error handler.
	*/
	shared_ptr<CListener> GetListener() const;

    /** \brief Removes listener 
	*   \eh On error, an error code is reported to the error handler.
    */
	void RemoveListener();
    
	////////////////////////
	// Environment methods
	////////////////////////

    /** \brief Creates a new environment
     *	\retval environment shared pointer to newly created environment with empty ABIR.
     */
    shared_ptr<CEnvironment> CreateEnvironment();
    
    /** \brief Removes one environment
     *	\param [in] environment shared pointer to remove
	 *   \eh On success, RESULT_OK is reported to the error handler.
	 *       On error, an error code is reported to the error handler.
     */
    void RemoveEnvironment(shared_ptr<CEnvironment> environment);

	/////////////////////////
	// Audio source methods
	/////////////////////////
    
    /** \brief Creates a new audio source for spatialization
     *	\retval source shared pointer to newly created audio source     
	 *   \eh On success, RESULT_OK is reported to the error handler.
	 *       On error, an error code is reported to the error handler.
     */
    shared_ptr<CSingleSourceDSP> CreateSingleSourceDSP();
    
    /** \brief Removes one audio source for spatialization
     *	\param [in] source shared pointer of audio source to remove
	 *   \eh On success, RESULT_OK is reported to the error handler.
	 *       On error, an error code is reported to the error handler.
     */
    void RemoveSingleSourceDSP(shared_ptr<CSingleSourceDSP> source);

private:
	// Reset the convolution buffer of each source	
	void ResetConvolutionBuffers();

	// Calculate the new coordinates from the source to the listener
	void CalculateSourceCoordinates();

	void RemoveAllSources();

	// Reset HRTF and BRIR when buffer size or HRTF resampling step changes	
	void CalculateHRTFandBRIR();

	// Reset HRTF, BRIR and ILD when sample rate changes
	void ResetHRTF_BRIR_ILD();

	///////////////
	// ATTRIBUTES
	///////////////	
	shared_ptr<CListener > listener;					// Listener attributes	
    vector<shared_ptr<CEnvironment>> environments;		// Environment attributes 															
	vector<shared_ptr<CSingleSourceDSP>> audioSources;	// List of audio sources 
	
	Common::TAudioStateStruct audioState;				// Global audio state
	Common::CMagnitudes magnitudes;						// Physical magnitudes
	int HRTF_resamplingStep;							// HRTF resampling step in degrees, in order to interpolate the HRTF table from database	
		
    friend class CEnvironment;							// Friend class definition
	friend class CListener;								// Friend class definition
};
 
}
#endif