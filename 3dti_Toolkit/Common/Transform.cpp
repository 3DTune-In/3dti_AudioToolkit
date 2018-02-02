/*
	* \class CTransform
	*
	* \brief Definition of rigid transformations (position and orientation).
	*
	* This class define the necessary algorithms for rigid transformations (position and orientation).
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
	
#include <Common/Transform.h>
#include <Common/ErrorHandler.h>

namespace Common {

	//////////////////////////////////////////////
	// CONSTRUCTORS/DESTRUCTORS

	CTransform::CTransform()
	{
		position = CVector3::ZERO;
		orientation = CQuaternion::UNIT;
	}

	//////////////////////////////////////////////
	// GET METHODS

		// Returns a vector from "this" to target in "this" reference frame 
	const CVector3 CTransform::GetVectorTo(CTransform target) const
	{
		// Error handler:
		// Trust in RotateVector for setting result

		// Get position of target in global reference frame
		CVector3 targetPositionGlobal = target.GetPosition();

		// Translate target until "this" reference frame has its origin in the global origin
		targetPositionGlobal = targetPositionGlobal - position;

		// Find new coordinates in "this" frame (rotated with respect to global)
		CVector3 targetPositionThis = orientation.Inverse().RotateVector(targetPositionGlobal);

		return targetPositionThis;
	}

	//////////////////////////////////////////////

	CVector3 CTransform::GetPosition()
	{
		return position;
	}

	//////////////////////////////////////////////

	CQuaternion CTransform::GetOrientation()
	{
		return orientation;
	}

	//////////////////////////////////////////////
	// SET METHODS

	void CTransform::SetPosition(CVector3 _position)
	{
		position = _position;
	}

	//////////////////////////////////////////////

	void CTransform::SetOrientation(CQuaternion _orientation)
	{
		orientation = _orientation;
	}

	//////////////////////////////////////////////
	// TRANSFORM METHODS

	void CTransform::Translate(CVector3 _translation)
	{
		position = position + _translation;
	}

	//////////////////////////////////////////////

		// Rotate from axis and angle. This method can be overloaded depending on needs
	void CTransform::Rotate(CVector3 _axis, float _angle)
	{
		// Error handler:
		// Trust in FromAxisAngle and Rotate for setting result

		CQuaternion rotation = CQuaternion::FromAxisAngle(_axis, _angle);
		orientation.Rotate(rotation);
	}

	//////////////////////////////////////////////

		// Returns a new transform with a local translation applied to the position
	const CTransform CTransform::GetLocalTranslation(CVector3 _translation) const
	{
		// Error handler: trust in called methods for setting result

		CTransform result;
		CVector3 newGlobalPosition = orientation.RotateVector(_translation) + position;
		result.SetPosition(newGlobalPosition);
		result.SetOrientation(orientation);
		return result;
	}
}//end namespace Common