////////////////////////////////////////////////
// Project: 3D-Tune-In
// Package: 3DTI Toolkit
/*
	* \class CVector3
	*
	* \brief Definition of a class for 3D Vectors.
	*
	* This class define the vars and methods for 3D Vectors.
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

	// ToDo:
//		+ Make sure conventions are taken into account
//		+ Simplify implementation of predefined rotation axis!!!

#define _USE_MATH_DEFINES // TODO: Test in windows! Might also be problematic for other platforms??
#include <cmath>
#include <Common/Vector3.h>
#include <Common/ErrorHandler.h>
#include <Common/Conventions.h>

constexpr float _2PI = 2.0f*M_PI;

namespace Common {

	//////////////////////////////////////////////
	// BASIC PREDEFINED VECTORS

	const CVector3 CVector3::ZERO(0.0f, 0.0f, 0.0f);

	//////////////////////////////////////////////
	// Predefined rotation axis for rotating in basic directions, using angle-axis rotation

#if AZIMUTH_MOTION == ANTICLOCKWISE
#if UP_AXIS == AXIS_Y
	//const CVector3 CVector3::TO_LEFT(0.0f, 1.0f, 0.0f);
	//const CVector3 CVector3::TO_RIGHT(0.0f, -1.0f, 0.0f);
	const CVector3 CVector3::TO_LEFT(0.0f, -1.0f, 0.0f);
	const CVector3 CVector3::TO_RIGHT(0.0f, 1.0f, 0.0f);
#elif UP_AXIS == AXIS_X
	const CVector3 CVector3::TO_LEFT(1.0f, 0.0f, 0.0f);
	const CVector3 CVector3::TO_RIGHT(-1.0f, 0.0f, 0.0f);
#elif UP_AXIS == AXIS_Z
	const CVector3 CVector3::TO_LEFT(0.0f, 0.0f, 1.0f);
	const CVector3 CVector3::TO_RIGHT(0.0f, 0.0f, -1.0f);
#elif UP_AXIS == AXIS_MINUS_Z
	const CVector3 CVector3::TO_LEFT(0.0f, 0.0f, -1.0f);
	const CVector3 CVector3::TO_RIGHT(0.0f, 0.0f, 1.0f);
#endif
	// TO DO: cases for -X and -Y
#elif AZIMUTH_MOTION == CLOCKWISE
#if UP_AXIS == AXIS_Y
	const CVector3 CVector3::TO_LEFT(0.0f, -1.0f, 0.0f);
	const CVector3 CVector3::TO_RIGHT(0.0f, 1.0f, 0.0f);
#elif UP_AXIS == AXIS_X
	const CVector3 CVector3::TO_LEFT(-1.0f, 0.0f, 0.0f);
	const CVector3 CVector3::TO_RIGHT(1.0f, 0.0f, 0.0f);
#elif UP_AXIS == AXIS_Z
	const CVector3 CVector3::TO_LEFT(0.0f, 0.0f, -1.0f);
	const CVector3 CVector3::TO_RIGHT(0.0f, 0.0f, 1.0f);
#endif
	// TO DO: more cases
#endif

#if ELEVATION_MOTION == ANTICLOCKWISE
#if RIGHT_AXIS == AXIS_X
	const CVector3 CVector3::TO_UP(1.0f, 0.0f, 0.0f);
	const CVector3 CVector3::TO_DOWN(-1.0f, 0.0f, 0.0f);
#elif RIGHT_AXIS == AXIS_Y
	const CVector3 CVector3::TO_UP(0.0f, 1.0f, 0.0f);
	const CVector3 CVector3::TO_DOWN(0.0f, -1.0f, 0.0f);
#elif RIGHT_AXIS == AXIS_Z
	const CVector3 CVector3::TO_UP(0.0f, 0.0f, 1.0f);
	const CVector3 CVector3::TO_DOWN(0.0f, 0.0f, -1.0f);
#elif RIGHT_AXIS == AXIS_MINUS_Y
	const CVector3 CVector3::TO_UP(0.0f, -1.0f, 0.0f);
	const CVector3 CVector3::TO_DOWN(0.0f, 1.0f, 0.0f);
#endif
#elif ELEVATION_MOTION == CLOCKWISE
#if RIGHT_AXIS == AXIS_X
	const CVector3 CVector3::TO_UP(-1.0f, 0.0f, 0.0f);
	const CVector3 CVector3::TO_DOWN(1.0f, 0.0f, 0.0f);
#elif RIGHT_AXIS == AXIS_Y
	const CVector3 CVector3::TO_UP(0.0f, -1.0f, 0.0f);
	const CVector3 CVector3::TO_DOWN(0.0f, 1.0f, 0.0f);
#elif RIGHT_AXIS == AXIS_Z
	const CVector3 CVector3::TO_UP(0.0f, 0.0f, -1.0f);
	const CVector3 CVector3::TO_DOWN(0.0f, 0.0f, 1.0f);
#endif
#endif


#if AZIMUTH_MOTION == ANTICLOCKWISE
#if FORWARD_AXIS == AXIS_MINUS_Y
	const CVector3 CVector3::TO_ROLL_LEFT(0.0f, -1.0f, 0.0f);
	const CVector3 CVector3::TO_ROLL_RIGHT(0.0f, 1.0f, 0.0f);
#elif FORWARD_AXIS == AXIS_X
	const CVector3 CVector3::TO_ROLL_LEFT(1.0f, 0.0f, 0.0f);
	const CVector3 CVector3::TO_ROLL_RIGHT(-1.0f, 0.0f, 0.0f);
#endif
	// TO DO: more cases 
#elif AZIMUTH_MOTION == CLOCKWISE
	// TO DO: more cases
#endif

	double SafeAcos(double x)
	{
		if (x < -1.0) x = -1.0;
		else if (x > 1.0) x = 1.0;
		return std::acos(x);
	}

	//////////////////////////////////////////////
	// CONSTRUCTORS/DESTRUCTORS

	CVector3::CVector3()
	{
		*this = CVector3::ZERO;
	}

	////////////////////////////////////////////////

	CVector3::CVector3(float _xyzArray[3])
	{
		x = _xyzArray[0];
		y = _xyzArray[1];
		z = _xyzArray[2];
	}

	////////////////////////////////////////////////

	CVector3::CVector3(float _x, float _y, float _z)
	{
		x = _x;
		y = _y;
		z = _z;
	}

	//////////////////////////////////////////////
	// GET METHODS

		// Get distance (vector modulus) in whatever units you are using
	const float CVector3::GetDistance() const
	{
		// Error handler: trust in GetSqrDistance
		// Sqrt may set errno if the argument is negative, but we have full control on this argument and we know that it can never be negative		
		return std::sqrt(GetSqrDistance());
	}

	//////////////////////////////////////////////

	// Get square of distance (avoid computing square roots)
	const float CVector3::GetSqrDistance() const
	{
		// Error handler:
		//SET_RESULT(RESULT_OK, "Distance computed succesfully.");

		//return (x*x + y*y + z*z);
		return (x*x + y*y + z*z);
	}

	//////////////////////////////////////////////

	// Get elevation in radians, according to the selected axis convention. Currently uses LISTEN database convention for elevation angles: full circle starting with 0º in front towards up. 
	const float CVector3::GetElevationRadians() const
	{
		// Error handler:
		float distance = GetDistance();
		if (distance == 0.0f)
		{
			SET_RESULT(RESULT_ERROR_DIVBYZERO, "Distance from source to listener is zero");
			return 0.0f;
		}
		//else
		//	SET_RESULT(RESULT_OK, "Elevation computed from vector succesfully");	// No more possible errors. 

		// 0=front; 90=up; -90=down
		//float cosAngle = *upAxis / GetDistance(); // Error check: division by zero
		//float angle = SafeAcos(cosAngle);
		//return (M_PI / 2.0f) - angle;

		// 0=front; 90=up; 270=down (LISTEN)
		float cosAngle = GetAxis(UP_AXIS) / distance;
		float angle = SafeAcos(cosAngle);
		float adjustedAngle = (M_PI * 2.5f) - angle;

		// Check limits (always return 0 instead of 2PI)
		if (adjustedAngle >= _2PI)
			adjustedAngle = std::fmod(adjustedAngle, (float)_2PI);

		return adjustedAngle;
	}

	//////////////////////////////////////////////

	// Get azimuth in radians, according to the selected axis convention. Currently uses LISTEN database convention for azimuth angles: anti-clockwise full circle starting with 0º in front.
	const float CVector3::GetAzimuthRadians() const
	{
		// Error handler:
		float rightAxis = GetAxis(RIGHT_AXIS);
		float forwardAxis = GetAxis(FORWARD_AXIS);
		if ((rightAxis == 0.0f) && (forwardAxis == 0.0f))
		{
			SET_RESULT(RESULT_ERROR_INVALID_PARAM, "Azimuth cannot be computed for a (0,0,z) vector. 0.0 is returned");
			return 0.0f;
		}

		// front=0; left=-90; right=90
		//return atan2(*rightAxis, *forwardAxis);		

		//front=0; left=90; right=270 (LISTEN)
		float angle = std::atan2(GetAxis(RIGHT_AXIS), GetAxis(FORWARD_AXIS));
		float adjustedAngle = std::fmod((float)(_2PI - angle), (float)_2PI);

		// Check limits (always return 0 instead of 2PI)
		if (adjustedAngle >= _2PI)
			adjustedAngle = std::fmod(adjustedAngle, (float)_2PI);

		return adjustedAngle;
	}

	//////////////////////////////////////////////

	// Get elevation in degrees, according to the selected axis convention. Currently uses LISTEN database convention for elevation angles: full circle starting with 0º in front towards up. 
	const float CVector3::GetElevationDegrees() const
	{
		// Error handler:
		// Trust in GetElevationRadians for setting result

		return GetElevationRadians() * (180.0f / M_PI);
	}

	//////////////////////////////////////////////

	// Get azimuth in degress, according to the selected axis convention. Currently uses LISTEN database convention for azimuth angles: anti-clockwise full circle starting with 0º in front. 
	const float CVector3::GetAzimuthDegrees() const
	{
		// Error handler:
		// Trust in GetAzimuthRadians for setting result

		return GetAzimuthRadians() * (180.0f / M_PI);
	}

	//////////////////////////////////////////////

	// Set the x,y,z coordinates from azimuth, elevation and distance. Currently uses LISTEN database convention for azimuth angles : anti - clockwise full circle starting with 0º in front.
	void CVector3::SetFromAED(float azimuth, float elevation, float distance)
	{
		azimuth = azimuth   * (M_PI / 180.0f);
		elevation = elevation * (M_PI / 180.0f);

		float up = std::sin(elevation);

		float pd = std::cos(elevation); // pd -> projected distance (on the horizontal plane).

		float right = -pd * std::sin(azimuth);   // minus sign to fit the LISTEN database convention
		float forward = pd * std::cos(azimuth);

		SetAxis(UP_AXIS, up      * distance);
		SetAxis(RIGHT_AXIS, right   * distance);
		SetAxis(FORWARD_AXIS, forward * distance);
	}

	//////////////////////////////////////////////

	// Get the interaural azimut angle in radians, according to the selected axis convention. Currently uses LISTEN database convention for elevation angles: full circle starting with 0º in front towards up.  
	const float CVector3::GetInterauralAzimuthRadians() const
	{
		float distance = GetDistance();
		if (distance == 0.0f)
		{
			SET_RESULT(RESULT_ERROR_DIVBYZERO, "Distance from source to listener is zero" );
			return 0.0f;
		}
		//else
		//	SET_RESULT(RESULT_OK, "Interaural azimuth computed from vector succesfully");	// No more possible errors. 

		float f = GetAxis(FORWARD_AXIS);
		float u = GetAxis(UP_AXIS);
		float r = GetAxis(RIGHT_AXIS);
		float angle = SafeAcos(std::sqrt(f*f + u*u) / distance);

		return r > 0 ? angle : -angle;
	}

	//////////////////////////////////////////////

	// Get the angle that this vector keeps with the forward axis
	const float CVector3::GetAngleToForwardAxisDegrees() const
	{
		return GetAngleToForwardAxisRadians() * (180.0f / M_PI);
	}

	//////////////////////////////////////////////

	// Get the angle that this vector keeps with the forward axis
	const float CVector3::GetAngleToForwardAxisRadians() const
	{
		float distance = GetDistance();
		if (distance == 0.0f)
		{
			SET_RESULT(RESULT_ERROR_DIVBYZERO, "Distance from source to listener is zero");
			return 0.0f;
		}

		float f = GetAxis(FORWARD_AXIS);
		float angle = SafeAcos(f / distance);

		return f >= 0 ? angle : angle + M_PI * 0.5;
	}

	//////////////////////////////////////////////

	// Get the interaural elevation angle in radians, according to the selected axis convention. Currently uses LISTEN database convention for elevation angles: full circle starting with 0º in front towards up.  
	const float CVector3::GetInterauralElevationRadians() const
	{
		float distance = GetDistance();
		if (distance == 0.0f)
		{
			SET_RESULT(RESULT_ERROR_DIVBYZERO, "Distance from source to listener is zero");
			return 0.0f;
		}
		//else
		//	SET_RESULT(RESULT_OK, "Interaural elevation computed from vector succesfully");	// No more possible errors. 

		float angle = std::atan2(GetAxis(UP_AXIS), GetAxis(FORWARD_AXIS));
		return angle >= 0 ? angle : angle + 2.0 * M_PI;
	}
	//////////////////////////////////////////////
	const float CVector3::GetInterauralAzimuthDegrees() const
	{
		// Error handler:
		// Trust in GetInterauralAzimutRadians for setting result
		return GetInterauralAzimuthRadians() * (180.0f / M_PI);
	}
	//////////////////////////////////////////////
	const float CVector3::GetInterauralElevationDegrees() const
	{
		// Error handler:
		// Trust in GetInterauralElevationRadians for setting result
		return GetInterauralElevationRadians() * (180.0f / M_PI);
	}

	//////////////////////////////////////////////
	// SET METHODS

	void CVector3::SetCoordinates(float _x, float _y, float _z)
	{
		x = _x;
		y = _y;
		z = _z;
	}

	//////////////////////////////////////////////

		//void CVector3::SetAxisConvention(CAxisConvention _axisConvention)
		//{
		//	axisConvention = _axisConvention;
		//}

	//////////////////////////////////////////////

	const float CVector3::GetAxis(TAxis _axis) const
	{
		// Error handler:
		//SET_RESULT(RESULT_OK, "Succesfully got axis from convention.");

		switch (_axis)
		{
			case AXIS_X: return x; break;
			case AXIS_Y: return y; break;
			case AXIS_Z: return z; break;
			case AXIS_MINUS_X: return -x; break;
			case AXIS_MINUS_Y: return -y; break;
			case AXIS_MINUS_Z: return -z; break;
			default: SET_RESULT(RESULT_ERROR_CASENOTDEFINED, "Trying to get an axis which name is not defined");  return 0.0f;
		}
	}

	//////////////////////////////////////////////

	void CVector3::SetAxis(TAxis _axis, float value)
	{
		// Error handler:
		//SET_RESULT(RESULT_OK, "Succesfully set axis from convention.");

		switch (_axis)
		{
		case AXIS_X: x = value; break;
		case AXIS_Y: y = value; break;
		case AXIS_Z: z = value; break;
		case AXIS_MINUS_X: x = -value;	break;
		case AXIS_MINUS_Y: y = -value;	break;
		case AXIS_MINUS_Z: z = -value;	break;
		default: SET_RESULT(RESULT_ERROR_CASENOTDEFINED, "Trying to set an axis which name is not defined");
		}
	}

	//////////////////////////////////////////////
	// BASIC VECTOR OPERANDS

	CVector3 CVector3::operator-(CVector3 const _rightHand)
	{
		return CVector3(x - _rightHand.x, y - _rightHand.y, z - _rightHand.z);
	}

	//////////////////////////////////////////////

	const CVector3 CVector3::operator+(CVector3 const _rightHand) const
	{
		return CVector3(x + _rightHand.x, y + _rightHand.y, z + _rightHand.z);
	}

	//////////////////////////////////////////////

	float CVector3::DotProduct(CVector3 _rightHand)
	{
		// Error handler:
		//SET_RESULT(RESULT_OK, "Dot product computed succesfully");

		return (x*_rightHand.x + y*_rightHand.y + z*_rightHand.z);
	}

	//////////////////////////////////////////////

	CVector3 CVector3::CrossProduct(CVector3 _rightHand)
	{
		// Error handler:
		//SET_RESULT(RESULT_OK, "Cross product computed sucessfully");

		CVector3 result;

		result.x = y*_rightHand.z - z*_rightHand.y;
		result.y = z*_rightHand.x - x*_rightHand.z;
		result.z = x*_rightHand.y - y*_rightHand.x;

		return result;
	}
}//end namespace Common
