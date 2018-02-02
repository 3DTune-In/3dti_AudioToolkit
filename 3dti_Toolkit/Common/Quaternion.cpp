/*
	* \class CQuaternion
	*
	* \brief Definition of Quaternions (representation of orientation).
	*
	* This class define the necessary algorithms and vars for Quaternions (representation of orientation).
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
	// ToDo: + Take into account conventions for circular motion
#include <Common/Quaternion.h>
#include <Common/ErrorHandler.h>
#include <cmath>

namespace Common {

	//////////////////////////////////////////////
	// BASIC PREDEFINED QUATERNIONS

	const CQuaternion CQuaternion::ZERO(0.0f, 0.0f, 0.0f, 0.0f);
	const CQuaternion CQuaternion::UNIT(1.0f, 0.0f, 0.0f, 0.0f);

	//////////////////////////////////////////////
	// CONSTRUCTORS/DESTRUCTORS

		// Default: unit quaternion
	CQuaternion::CQuaternion()
	{
		*this = CQuaternion::UNIT;
	}

	//////////////////////////////////////////////

	CQuaternion::CQuaternion(float _w, float _x, float _y, float _z)
	{
		w = _w;
		x = _x;
		y = _y;
		z = _z;
	}

	//////////////////////////////////////////////

	CQuaternion::CQuaternion(float _w, CVector3 _v)
	{
		w = _w;
		x = _v.x;
		y = _v.y;
		z = _v.z;
	}

	//////////////////////////////////////////////

		// From a vector (quaternion without scalar component)
	CQuaternion::CQuaternion(CVector3 _vector)
	{
		w = 0.0f;
		x = _vector.x;
		y = _vector.y;
		z = _vector.z;
	}

	//////////////////////////////////////////////

		// From a scalar (quaternion without vectorial component)
	CQuaternion::CQuaternion(float _scalar)
	{
		w = _scalar;
		x = y = z = 0.0f;
	}

	//////////////////////////////////////////////
	// BASIC OPERATIONS

		// Rotate a quaternion with another quaternion
	void CQuaternion::Rotate(CQuaternion _rightHand)
	{
		// Error handler:
		//SET_RESULT(RESULT_OK, "Quaternion rotated succesfully");

		CQuaternion thisCopy = *this;
		*this = _rightHand*thisCopy;
	}

	//////////////////////////////////////////////

		// Rotate a vector with this quaternion
	const CVector3 CQuaternion::RotateVector(CVector3 _vector) const
	{
		// Error handler:
		// Trust in Inverse for setting result

		// Convert vector into quaternion, forcing quaternion axis convention
		CQuaternion vectorQuaternion = CQuaternion(_vector);

		// Left product
		CQuaternion leftProduct = *this*vectorQuaternion;

		// Right product
		CQuaternion rightProduct = leftProduct*Inverse();

		// Convert result quaternion into vector
		CVector3 result = CVector3(rightProduct.x, rightProduct.y, rightProduct.z);

		return result;
	}

	//////////////////////////////////////////////
	// CONVERSIONS

		// Get a quaternion from axis and angle
	CQuaternion CQuaternion::FromAxisAngle(CVector3 _axis, float _angle)
	{
		// Error handler:
		//SET_RESULT(RESULT_OK, "Conversion from axis-angle to quaternion was succesfull");

		float newW, newX, newY, newZ;

		float halfAngle = _angle*0.5f;
		float halfSin = std::sin(halfAngle);
		float halfCos = std::cos(halfAngle);

		newW = halfCos;
		newX = halfSin*_axis.x;
		newY = halfSin*_axis.y;
		newZ = halfSin*_axis.z;

		return CQuaternion(newW, newX, newY, newZ);
	}

	//////////////////////////////////////////////

		// Get axis and angle from a quaternion
	void CQuaternion::ToAxisAngle(CVector3& _axis, float& _angle)
	{
		float sqrLength = x*x + y*y + z*z;
		if (sqrLength > 0.0f)
		{
			// Error handler:
			//SET_RESULT(RESULT_OK, "Conversion from quaternion to axis-angle was succesfull");

			float invLength = 1.0f / std::sqrt(sqrLength);
			_axis.SetCoordinates(x*invLength, y*invLength, z*invLength);
			_angle = 2.0f*std::acos(w);
		}
		else
		{
			// Error handler:
			SET_RESULT(RESULT_WARNING, "Converting to axis/angle from zero quaternion returns an arbitrary axis");

			_axis.SetCoordinates(1.0f, 0.0f, 0.0f); // Any axis is valid for a 0 angle
			_angle = 0.0f;
		}
	}

	//////////////////////////////////////////////

		// Get a quaternion from roll-pitch-yaw
	CQuaternion CQuaternion::FromYawPitchRoll(float _yaw, float _pitch, float _roll)
	{
		double t0 = std::cos(_yaw * 0.5f);
		double t1 = std::sin(_yaw * 0.5f);
		double t2 = std::cos(_roll * 0.5f);
		double t3 = std::sin(_roll * 0.5f);
		double t4 = std::cos(_pitch * 0.5f);
		double t5 = std::sin(_pitch * 0.5f);

		float newW = t0 * t2 * t4 + t1 * t3 * t5;
		float newForward = t0 * t3 * t4 - t1 * t2 * t5;
		float newRight = t0 * t2 * t5 + t1 * t3 * t4;
		float newDown = t1 * t2 * t4 - t0 * t3 * t5;

		// Create vector part of quaternion, for convention-independent operations
		CVector3 vectorPart = CVector3::ZERO;
		vectorPart.SetAxis(UP_AXIS, -newDown);
		vectorPart.SetAxis(RIGHT_AXIS, newRight);
		vectorPart.SetAxis(FORWARD_AXIS, newForward);
		return CQuaternion(newW, vectorPart);
	}

	//////////////////////////////////////////////

		// Get roll-pitch-yaw representation from a quaternion
	void CQuaternion::ToYawPitchRoll(float& yaw, float& pitch, float& roll)
	{
		// Get vector part of quaternion and extract up, forward and right axis values
		CVector3 vectorPart = CVector3(x, y, z);
		float up = vectorPart.GetAxis(UP_AXIS);
		float right = vectorPart.GetAxis(RIGHT_AXIS);
		float forward = vectorPart.GetAxis(FORWARD_AXIS);
		float down = -up;

		// roll (forward-axis rotation)
		double t0 = 2.0f * (w * forward + right * down);
		double t1 = 1.0f - 2.0f * (forward * forward + right * right);
		roll = std::atan2(t0, t1);

		// pitch (right-axis rotation)
		double t2 = 2.0f * (w * right - down * forward);
		t2 = t2 > 1.0f ? 1.0f : t2;
		t2 = t2 < -1.0f ? -1.0f : t2;
		pitch = std::asin(t2);

		// yaw (up-axis rotation)
		double t3 = 2.0f * (w * down + forward * right);
		double t4 = 1.0f - 2.0f * (right * right + down * down);
		yaw = std::atan2(t3, t4);
	}

	//////////////////////////////////////////////
	// OPERANDS

		// Quaternion product (not commutative). Use this for rotating!
	const CQuaternion CQuaternion::operator* (const CQuaternion _rightHand) const
	{
		// Error handler:
		//SET_RESULT(RESULT_OK, "Quaternion product operation succesfull.");

		float newW = w*_rightHand.w - x*_rightHand.x - y*_rightHand.y - z*_rightHand.z;
		float newX = w*_rightHand.x + x*_rightHand.w + y*_rightHand.z - z*_rightHand.y;
		float newY = w*_rightHand.y + y*_rightHand.w + z*_rightHand.x - x*_rightHand.z;
		float newZ = w*_rightHand.z + z*_rightHand.w + x*_rightHand.y - y*_rightHand.x;
		return CQuaternion(newW, newX, newY, newZ);
	}

	//////////////////////////////////////////////

		// Get the inverse of this quaternion
	const CQuaternion CQuaternion::Inverse() const
	{
		// Error handler:
		float norm = SqrNorm();		// Not completely sure that we can use SqrNorm instead of Norm...
		if (norm == 0.0f)
		{
			SET_RESULT(RESULT_WARNING, "Computing inverse of quaternion with zero norm (returns ZERO quaternion)");
			return CQuaternion::ZERO;
		}
		//else		
		//	SET_RESULT(RESULT_OK, "Inverse of quaternion was computed succesfully");

		float invNorm = 1.0f / norm;

		float newW = w*invNorm;
		float newX = -x*invNorm;
		float newY = -y*invNorm;
		float newZ = -z*invNorm;

		return CQuaternion(newW, newX, newY, newZ);
	}

	//////////////////////////////////////////////

		// Get the norm of this quaternion
	float CQuaternion::Norm()
	{
		// Error handler: 
		// Trust in SqrNorm for setting result

		return std::sqrt(SqrNorm());
	}

	//////////////////////////////////////////////

		// Get the squared norm
	const float CQuaternion::SqrNorm() const
	{
		// Error handler:
		//SET_RESULT(RESULT_OK, "Norm computed succesfully for quaternion");

		return w*w + x*x + y*y + z*z;
	}

	//////////////////////////////////////////////

	// Get the pitch angle
	float CQuaternion::GetPitch()
	{
		return std::asin(-2.0*(x*z - w*y));
	}
	//////////////////////////////////////////////

	// Get the roll angle
	float CQuaternion::GetRoll()
	{
		return std::atan2(2.0*(x*y + w*z), w*w + x*x - y*y - z*z);
	}
}//end namespace Common