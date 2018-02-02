/**
* \class CVector3
*
* \brief  Declaration of CVector3 class interface.
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

#ifndef _CVECTOR3_H_
#define _CVECTOR3_H_

#include <Common/Conventions.h>
#include <iostream>
#include <string>

namespace Common {

	/** \details This class declares the vars and methods for handling 3D Vectors.
	*/
	class CVector3
	{
		// METHODS
	public:

		static const CVector3 ZERO;		///< Predefined ZERO vector (0, 0, 0)

	//
	// Predefined rotation axis for rotating in basic directions, using angle-axis rotation
	//

		static const CVector3 TO_LEFT;       ///< Predefined Left rotation axis, for angle-axis rotation
		static const CVector3 TO_RIGHT;      ///< Predefined Right rotation axis, for angle-axis rotation
		static const CVector3 TO_UP;         ///< Predefined Up rotation axis, for angle-axis rotation
		static const CVector3 TO_DOWN;       ///< Predefined Down rotation axis, for angle-axis rotation

		static const CVector3 TO_ROLL_LEFT;  ///< Predefined forward rotation axis, for angle-axis rotation
		static const CVector3 TO_ROLL_RIGHT; ///< Predefined backward rotation axis, for angle-axis rotation

	//
	// Constructors/Destructors
	//

		/** \brief Default constructor
		*	\details By default, sets \link ZERO \endlink vector
		*   \eh Nothing is reported to the error handler.
		*/
		CVector3();

		/** \brief Constructor from array
		*	\param [in] _xyzArray array with the 3 vector components (x, y, z)
		*   \eh Nothing is reported to the error handler.
		*/
		CVector3(float _xyzArray[3]);

		/** \brief Constructor from components
		*	\param [in] _x x vector component
		*	\param [in] _y y vector component
		*	\param [in] _z z vector component
		*   \eh Nothing is reported to the error handler.
		*/
		CVector3(float _x, float _y, float _z);

		//
		// Get methods
		//

		/** \brief Get distance (vector modulus)
		*	\retval distance vector modulus
		*   \eh Nothing is reported to the error handler.
		*/
		const float GetDistance() const;

		/** \brief Get squared distance
		*	\details To avoid computing square roots
		*	\retval distance squared vector modulus
		*   \eh Nothing is reported to the error handler.
		*/
		const float GetSqrDistance() const;

		/** \brief Get elevation in radians
		*	\details Elevation to which vector is pointing, in accordance with the selected axis convention. Currently uses LISTEN database convention for elevation angles: full circle starting with 0º in front towards up.
		*	\retval elevation elevation, in radians
		*   \eh On error, an error code is reported to the error handler.
		*/
		const float GetElevationRadians() const;

		/** \brief Get azimuth in radians
		*	\details Azimuth to which vector is pointing, in accordance with the selected axis convention. Currently uses LISTEN database convention for for azimuth angles: anti-clockwise full circle starting with 0º in front.
		*	\retval azimuth azimuth, in radians
		*   \eh Nothing is reported to the error handler.
		*/
		const float GetAzimuthRadians() const;

		/** \brief Get elevation in degrees
		*	\details Elevation to which vector is pointing, in accordance with the selected axis convention. Currently uses LISTEN database convention for elevation angles: full circle starting with 0º in front towards up.
		*	\retval elevation elevation, in degrees
		*   \eh Nothing is reported to the error handler.
		*/
		const float GetElevationDegrees() const;

		/** \brief Get azimuth in degrees
		*	\details Azimuth to which vector is pointing, in accordance with the selected axis convention. Currently uses LISTEN database convention for for azimuth angles: anti-clockwise full circle starting with 0º in front.
		*	\retval azimuth azimuth, in degrees
		*   \eh Nothing is reported to the error handler.
		*/
		const float GetAzimuthDegrees() const;

		/** \brief Set the x,y,z coordinates from azimuth, elevation and distance.
		*    Currently uses LISTEN database convention for azimuth angles : anti - clockwise full circle starting with 0 degrees in front.
		*   \eh Nothing is reported to the error handler.
		*/
		void SetFromAED(float azimuth_degrees, float elevation_degrees, float distance);

		/** \brief Get the interaural azimuth angle in radians, according to the selected axis convention.
		*	\details The interaural axis is located along the line that connects the two ears.
		*	The origin of coordinates is located between the two ears.
		*	The interaural azimuth angle of a vector (sometimes called interaural angle) is the angle between that vector and the sagittal (median) plane.
		*	\retval interauralAzimuth interaural azimuth in radians, from -PI/2 (left) to PI/2 degrees (right). 0 radians means the sagittal (median) plane		
		*   \eh On error, an error code is reported to the error handler.
		*/
		const float GetInterauralAzimuthRadians() const;

		/** \brief Get the interaural elevation angle in radians, according to the selected axis convention.
		*	\details The interaural axis is located along the line that connects the two ears.
		*	The origin of coordinates is located between the two ears.
		*	The interaural elevation angle of a vector (sometimes called polar angle) is the rotation angle around the intearural axis.
		*	\retval interauralElevation interaural elevation in radians, from 0 to 2*PI.
		*	0 radians means the forward axial (horizontal) semiplane.
		*   PI/2 means the upper coronal (frontal) semiplane.
		*   PI means the backward axial (horizontal) semiplane.
		*   3/2*PI means the lower coronal (frontal) semiplane.
		*   \eh On error, an error code is reported to the error handler.
		*/
		const float GetInterauralElevationRadians() const;

		/** \brief Get the interaural azimuth angle in degrees, according to the selected axis convention.
		*	\details The interaural axis is located along the line that connects the two ears.
		*	The origin of coordinates is located between the two ears.
		*	The interaural azimuth angle of a vector (sometimes called interaural angle) is the angle between that vector and the sagittal (median) plane.
		*	\retval interauralAzimuth interaural azimuth from -90 degrees (left) to +90 degrees (right). 0 degrees means the sagittal (median) plane
		*   \eh Nothing is reported to the error handler.
		*/
		const float GetInterauralAzimuthDegrees() const;

		/** \brief Get the interaural elevation angle in degrees, according to the selected axis convention.
		*	\details The interaural axis is located along the line that connects the two ears.
		*	The origin of coordinates is located between the two ears.
		*	The interaural elevation angle of a vector (sometimes called polar angle) is the rotation angle around the intearural axis.
		*	\retval interauralElevation interaural elevation in degrees, from 0 to 360.
		*	0 degrees means the forward axial (horizontal) semiplane.
		*   90 degrees means the upper coronal (frontal) semiplane.
		*   180 degrees means the backward axial (horizontal) semiplane.
		*   270 degrees means the lower coronal (frontal) semiplane.
		*   \eh Nothing is reported to the error handler.
		*/
		const float GetInterauralElevationDegrees() const;

		/** \brief Get the angle that this vector keeps with the forward axis.
		*	\details
		*	\retval This angle in degrees.
		*   \throws May throw errors to debugger
		*   \eh Nothing is reported to the error handler.
		*/
		const float GetAngleToForwardAxisDegrees() const;

		/** \brief Get the angle that this vector keeps with the forward axis.
		*	\details
		*	\retval This angle in radians.
		*   \eh On error, an error code is reported to the error handler.
		*/
		const float GetAngleToForwardAxisRadians() const;

		/** \brief Get the value of a given axis, in accordance with the axis convention
		*	\details This method is convention-safe
		*	\param [in] _axis which axis
		*	\retval value value of axis component
		*   \eh On error, an error code is reported to the error handler.
		*/
		const float GetAxis(TAxis _axis) const;

		/** \brief Set the value of a given axis, in accordance with the axis convention
		*	\details This method is convention-safe
		*	\param [in] _axis which axis
		*	\param [in] value value of axis component
		*   \eh On error, an error code is reported to the error handler.
		*/
		void SetAxis(TAxis _axis, float value);

		/** \brief Set the three components of the vector
		*	\param [in] _x x component
		*	\param [in] _y y component
		*	\param [in] _z z component
		*   \eh Nothing is reported to the error handler.
		*/
		void SetCoordinates(float _x, float _y, float _z);

		//
		// Basic vector operators
		//

		/** \brief Component-wise substraction
		*/
		CVector3 operator-(const CVector3 _rightHand);

		/** \brief Component-wise addition
		*/
		const CVector3 operator+(const CVector3 _rightHand) const;

		/** \brief Computes the vector dot product
		*	\param [in] _rightHand other vector
		*	\retval product dot product of this vector with other vector
		*   \eh Nothing is reported to the error handler.
		*/
		float DotProduct(CVector3 _rightHand);

		/** \brief Computes the vector cross product
		*	\param [in] _rightHand other vector
		*	\retval product cross product of this vector with other vector
		*   \eh Nothing is reported to the error handler.
		*/
		CVector3 CrossProduct(CVector3 _rightHand);

		// ATTRIBUTES
	public:
		float x;   ///< x component of the vector
		float y;   ///< y component of the vector
		float z;   ///< z component of the vector	
	};

	//

	/** \brief Formatted stream output of vectors for debugging */
	inline std::ostream & operator<<(std::ostream & out, const CVector3 & v)
	{
		out << "(" << v.x << ", " << v.y << ", " << v.z << ")";
		return out;
	}
}//end namespace Common
#endif