/* Implementation of 3D Tune In Classification Scale for Hearing loss characterization
* \date	November 2017
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

#ifndef _CLASSIFICATION_SCALE_HL_
#define _CLASSIFICATION_SCALE_HL_

#include <HAHLSimulation/HearingLossSim.h>

namespace HAHLSimulation {

	/** \brief Get all hearing loss values (dBHL) for an audiometry, depending on a specified preset of the HL classification scale	
	*	\param [in] curve letter specifying the curve from the HL classification scale ('A' to 'K')
	*	\param [in] extend of slope to be applied to the curveseverity from the HL classification scale (0 to 6)
	*	\param [in] severity of HL applied as an offset to the values returned in hl
	*	\param [out] hl audiometry with all hearing loss levels (dBHL) corresponding to the chosen HL classification scale (curve and scale)
	*   \eh Nothing is reported to the error handler.
	*/
	extern void GetClassificationScaleHL( char curve, int slope, int severity, TAudiometry &hl );

}// end namespace HAHLSimulation

#endif
