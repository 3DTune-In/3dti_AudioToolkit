/**
* \class CQuaternion
*
* \brief Declaration of CQuaternion class interface. 
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

#ifndef _CQUATERNION_H_
#define _CQUATERNION_H_

#include <Common/Vector3.h>
#include <iostream>

namespace Common {

	/** \details This class declares the necessary algorithms and vars for Quaternions (representation of orientation).
	*/
	class CQuaternion
	{
		// METHODS
	public:

		static const CQuaternion ZERO;                         ///< Basic predefined ZERO quaternion (0, 0, 0, 0)
		static const CQuaternion UNIT;                         ///< Basic predefined UNIT quaternion (1, 0, 0, 0)

	//
	// Constructors/Destructors
	//

		/** \brief Default constructor
		*	\details By default, sets a \link UNIT \endlink quaternion
		*   \eh Nothing is reported to the error handler.
		*/
		CQuaternion();

		/** \brief Constructor from components
		*	\param [in] _w w component (scalar part)
		*	\param [in] _x x component (vector part)
		*	\param [in] _y y component (vector part)
		*	\param [in] _z z component (vector part)
		*   \eh Nothing is reported to the error handler.
		*/
		CQuaternion(float _w, float _x, float _y, float _z);

		/** \brief Constructor from components
		*	\param [in] _w w component (scalar part)
		*	\param [in] _v vector part
		*   \eh Nothing is reported to the error handler.
		*/
		CQuaternion(float _w, CVector3 _v);

		/** \brief Constructor from vector
		*	\details Builds a quaternion without scalar part
		*	\param [in] _vector vector components (x, y, z)
		*   \eh Nothing is reported to the error handler.
		*/
		CQuaternion(CVector3 _vector);

		/** \brief Constructor from scalar
		*	\details Builds a quaternion without vector part
		*	\param [in] _scalar scalar component (w)
		*   \eh Nothing is reported to the error handler.
		*/
		CQuaternion(float _scalar);

		//
		// Conversions (We can implement more if we need them)
		//

			/** \brief Get a quaternion from an axis and angle representation
			*	\param [in] _axis rotation axis
			*	\param [in] _angle amount or angle of rotation, in radians
			*	\retval quaternion quaternion equivalent to the given axis-angle representation
			*   \eh Nothing is reported to the error handler.
			*/
		static CQuaternion FromAxisAngle(CVector3 _axis, float _angle);

		/** \brief Get axis and angle representation from a quaternion
		*	\param [out] _axis rotation axis
		*	\param [out] _angle amount or angle of rotation, in radians
		*   \eh Warnings may be reported to the error handler
		*/
		void ToAxisAngle(CVector3& _axis, float& _angle);

		/** \brief Get a quaternion from a roll-pitch-yaw representation
		*   \details This representation corresponds to intrinsic Tait-Bryan angles with sequence: yaw-pitch-roll
		*	\param [in] _roll roll angle in radians
		*	\param [in] _pitch pitch angle in radians
		*	\param [in] _yaw yaw angle in radians
		*	\retval quaternion quaternion equivalent to the given roll-pitch-yaw representation
		*   \eh Nothing is reported to the error handler.
		*/
		static CQuaternion FromYawPitchRoll(float _yaw, float _pitch, float _roll);

		/** \brief Get roll-pitch-yaw representation from a quaternion
		*   \details This representation corresponds to intrinsic Tait-Bryan angles with sequence: yaw-pitch-roll
		*	\param [out] roll roll angle in radians
		*	\param [out] pitch pitch angle in radians
		*	\param [out] yaw yaw angle in radians
		*   \eh Nothing is reported to the error handler.
		*/
		void ToYawPitchRoll(float& yaw, float& pitch, float& roll);

		//
		// Basic operations
		//

			/** \brief Rotate quaternion with another quaternion
			*	\details Equivalent to quaternion product
			*	\param [in] _rightHand other quaternion
			*   \eh Nothing is reported to the error handler.
			*/
		void Rotate(CQuaternion _rightHand);

		/** \brief Rotate a vector with another quaternion
		*	\param [in] _vector vector to rotate
		*	\retval rotated rotated vector
		*   \eh Nothing is reported to the error handler.
		*/
		const CVector3 RotateVector(CVector3 _vector) const;

		//
		// Operands
		//

			/** \brief Quaternion product
			*	\details Quaternion product is not commutative. Used for rotating.
			*/
		const CQuaternion operator* (const CQuaternion _rightHand) const;

		/** \brief Get the quaternion inverse
		*	\retval inverse inverse of quaternion
		*   \throws May throw warnings to debugger
		*   \eh Warnings may be reported to the error handler
		*/
		const CQuaternion Inverse() const;

		/** \brief Get the quaternion norm
		*	\retval norm norm of quaternion
		*   \eh Nothing is reported to the error handler.
		*/
		float Norm();

		/** \brief Get the squared quaternion norm
		*	\details To avoid computing square roots
		*	\retval sqrnorm squared norm of quaternion
		*   \eh Nothing is reported to the error handler.
		*/
		const float SqrNorm() const;

		/** \brief Get the pitch angle in radians
		*	\retval pitch pitch angle in radians
		*   \eh Nothing is reported to the error handler.
		*/
		float GetPitch();

		/** \brief Get the roll angle in radians
		*	\retval roll roll angle in radians
		*   \eh Nothing is reported to the error handler.
		*/
		float GetRoll();

		// ATTRIBUTES
	public:
		float w;    ///< w component of the quaternion (scalar part)
		float x;    ///< x component of the quaternion (vector part)
		float y;    ///< y component of the quaternion (vector part)
		float z;    ///< z component of the quaternion (vector part)
	};

	/** \brief Formatted stream output of quaternions for debugging
	*/
	inline std::ostream & operator<<(std::ostream & out, const CQuaternion & q)
	{
		out << "<" << q.w << ", (" << q.x << ", " << q.y << ", " << q.z << ")>";
		return out;
	}
}//end namespace Common
#endif