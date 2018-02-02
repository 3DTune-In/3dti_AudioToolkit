/**
* \class CTransform
*
* \brief  Declaration of CTransform class interface
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

#ifndef _CTRANSFORM_H_
#define _CTRANSFORM_H_

#include <Common/Vector3.h>
#include <Common/Quaternion.h>
#include <Common/Conventions.h>

namespace Common {

	/** \details This class holds data and algorithms for rigid transformations (position and orientation).
	*/
	class CTransform
	{
		// METHODS
	public:

		//
		// Constructors/Destructors
		//

			/** \brief Default constructor
			*	\details By default, sets position to (0,0,0) and orientation towards the forward vector (front).
			*   \eh Nothing is reported to the error handler.
			*/
		CTransform();

		//
		// Get methods
		//

			/** \brief Get a vector from "this" to target in "this" reference frame
			*	\param [in] target target transform
			*	\retval vector vector from this transform to target transform, in the reference frame of this transform
			*   \throws May throw warnings to debugger
			*   \eh Nothing is reported to the error handler.
			*/
		const CVector3 GetVectorTo(CTransform target) const;

		/** \brief Get the position component
		*	\retval position vector containing the position
		*   \eh Nothing is reported to the error handler.
		*/
		CVector3 GetPosition();

		/** \brief Get the orientation component
		*	\retval orientation quaternion containing the orientation
		*   \eh Nothing is reported to the error handler.
		*/
		CQuaternion GetOrientation();

		//
		// Set methods
		//

		/** \brief Set the position component
		*	\param [in] _position vector containing the position
		*   \eh Nothing is reported to the error handler.
		*/
		void SetPosition(CVector3 _position);

		/** \brief Set the orientation component
		*	\param [in] _orientation quaternion containing the orientation
		*   \eh Nothing is reported to the error handler.
		*/
		void SetOrientation(CQuaternion _orientation);

		//
		// Transform methods
		//

		/** \brief Applies a translation to the current position
		*	\param [in] _translation vector of translation
		*   \eh Nothing is reported to the error handler.
		*/
		void Translate(CVector3 _translation);

		/** \brief Applies a rotation to the current orientation
		*	\details Rotation of a given angle with respect to a given axis
		*	\param [in] _axis axis of rotation
		*	\param [in] _angle amount or angle of rotation, in radians
		*   \eh Nothing is reported to the error handler.
		*/
		void Rotate(CVector3 _axis, float _angle);

		/** Returns a new transform with a local translation applied to the position
		*	\param [in] _translation vector of translation
		*	\retval translation position obtained after local translation
		*   \eh Nothing is reported to the error handler.
		*/
		const CTransform GetLocalTranslation(CVector3 _translation) const;

		// ATTRIBUTES
	private:
		CVector3 position;
		CQuaternion orientation;
	};
}//end namespace Common
#endif
