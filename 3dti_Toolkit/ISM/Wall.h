/**
* \class Wall
*
* \brief Declaration of Wall interface. This class defines a wall as a set of vertex which has to be declared anticlockwise 
		 as seen from inside the room. Walls are the key component to compute images.
* \date	July 2021
*
* \authors F. Arebola-Pérez and A. Reyes-Lecuona, members of the 3DI-DIANA Research Group (University of Malaga)
* \b Contact: A. Reyes-Lecuona as head of 3DI-DIANA Research Group (University of Malaga): areyes@uma.es
*
* \b Contributions: (additional authors/contributors can be added here)
*
* \b Project: SAVLab (Spatial Audio Virtual Laboratory) ||
* \b Website:
*
* \b Copyright: University of Malaga - 2021
*
* \b Licence: GPLv3
*
* \b Acknowledgement: This project has received funding from Spanish Ministerio de Ciencia e Innovación under the SAVLab project (PID2019-107854GB-I00)
*
*/
#pragma once
#include <vector>
#include <Common/Vector3.h>

#ifndef THRESHOLD_BORDER
#define THRESHOLD_BORDER 0.3f
#endif

#ifndef NUM_BAND_ABSORTION
#define NUM_BAND_ABSORTION 9
#endif

#ifndef FIRST_ABSORTION_BAND
#define FIRST_ABSORTION_BAND 62.5f
#endif

namespace ISM
{

	class Wall
	{
	public:
		////////////
		// Methods
		////////////

		/** \brief Constructor
		*	\details Defauld values: wall is active and purely reflective (absortion coefficients = 0.0)
		*/
		Wall();

		/** \brief Insert a new corner (vertex) in the wall by coordinates
		*	\details Walls are defined as a series of corners (vertices), whcih should be declared in anticlockwise order as seen from inside the
					 room. The wall is asumed to be a convex polygon and the last edge is defined between the last and the first declared vertices.
					 When more then 3 corners are inserted, it is checked wether the new corner is in the same plane as the others or not. If not,
					 it is projected to the plane to ensure that all the corners belong to the same plane.
		*	\param [in] X coordinate of the corner to be inserted (m).
		*	\param [in] Y coordinate of the corner to be inserted (m).
		*	\param [in] Z coordinate of the corner to be inserted (m).
		*	\param [out] CORRECT: 1 if the new corner is in the same plane of the others. 0 if not and a projection to the plane is carried out.
		*/
		int insertCorner(float x, float y, float z);

		/** \brief Insert a new corner (vertex) in the wall by a 3D vector
		*	\details Walls are defined as a series of corners (vertices), whcih should be declared in anticlockwise order as seen from inside the
					 room. The wall is asumed to be a convex polygon and the last edge is defined between the last and the first declared vertices.
					 When more then 3 corners are inserted, it is checked wether the new corner is in the same plane as the others or not. If not,
					 it is projected to the plane to ensure that all the corners belong to the same plane.
		*	\param [in] Corner: vector containing the new corner to be inserted (expresed in m).
		*	\param [out] CORRECT: 1 if the new corner is in the same plane of the others. 0 if not and a projection to the plane is carried out.
		*/
		int insertCorner(Common::CVector3 _corner);

		/** \brief Returns the corners of the wall
		*	\param [out] Corners: vector containing the set of corners of the wall in teh same order as they are defined.
		*/
		std::vector<Common::CVector3> getCorners();

		/** \brief set the absortion coeficient (frequency independent) of the wall
		*   \param [in] Absortion: absortion coeficient of the wall (expressed as a number between 0 (no absortion) and 1 (total absortion).
		*/
		void setAbsortion(float _absortion);
		
		/** \brief set the absortion coeficients for each band of the wall
		*	\param [in] Absortion: Vector with absortion coeficients of the wall (expressed as a number between 0 (no absortion) and 1 (total absortion).
		*/
		void setAbsortion(std::vector<float> _absortionPerBand);
				
		/** \brief Returns the vector with absortion coeficients of the wall. 
		*	\param [out] Absortion: absortion of the wall. Vector with the absorption coefficients of each band.
		*/
		std::vector<float> getAbsortionB();

		/** \brief Returns the normal vector to the wall. If the wall is properly defined, it points towards inside the room.
		*	\param [out] Normal: normal vector to the wall.
		*/
		Common::CVector3 getNormal();

		/** \brief Returns the center of the wall.
		*	\param [out] Center: central point of the wall.
		*/
		Common::CVector3 getCenter();

		/** \brief Returns the distance of a given point to the wall's plane.
		*	\param [in] Point: point for whhich the distance to the wall's plane will be calculated (m).
		*	\param [out] Distance: shorterst distance to teh wall's plane (m).
		*/
		float getDistanceFromPoint(Common::CVector3 point);

		/** \brief Returns the minimum distance between two walls.
		*	\details Computes the distance between each corner of the given wall and each corner of this wall and returns the minimum of these distances
					 This method is used to determine whether an image room can contain an image source closer than a given distance to any possible location
					 in the original room. If the minimum distance from the wall in the original room which starts the branch of images and the image wall 
					 which produces a new image room is greater than d, it is not possible to find a location in the new image room closar than d to any 
					 location in the original room
		*	\param [in] wall: wall to compute the distance to this one.
		*	\param [out] Distance: shorterst distance to teh wall's plane (m).
		*/
		float getMinimumDistanceFromWall(ISM::Wall wall);

		/** \brief Returns the location of the image of a given point reflected in the wall's plane.
		*	\param [in] Point: original point for which the image reflected in the wall will be calculated.
		*	\param [out] Image: location of the image point.
		*/
		Common::CVector3 getImagePoint(Common::CVector3 point);

		/** \brief Returns an image wall of another given wall reflected in this wall's plane
		*	\param [in] Wall: original wall.
		*	\param [out] ImageWall: image wall, result of reflection of the original wall.
		*/
		Wall getImageWall(Wall _wall);

		/** \brief Returns the poin projected in the wall's plane of a given point.
		*	\param [in] X coordinate of the point to be projected.
		*	\param [in] Y coordinate of the point to be projected.
		*	\param [in] Z coordinate of the point to be projected.
		*	\param [out] Projection: porjected point in the woall's plane.
		*/
		Common::CVector3 getPointProjection(float x, float y, float z);

		/** \brief Returns the poin projected in the wall's plane of a given point.
		*	\param [in] Point: point to be projected.
		*	\param [out] Projection: porjected point in the woall's plane.
		*/
		Common::CVector3 getPointProjection(Common::CVector3 point);

		/** \brief Returns the poin where a given line intersects the wall's plane.
		*	\details Given a line defined with two points, this method computes its intersection qithg the wall's plane and returns
					 that intersection point.
		*	\param [in] Point1: one of the points to define the line.
		*	\param [in] Point2: the other point to define the line.
		*	\param [out] Intersection: point of intersection of teh given line and the wall's plane.
		*/
		Common::CVector3 getIntersectionPointWithLine(Common::CVector3 point1, Common::CVector3 point2);

		/** \brief Checks wether a given point is inside the wall or not.
		*	\details Given a point, this method returns a positive value -that is the distance from the point to the closest edge of the wall-
					when the point is in the wall's plane and within the limits defined by the wall's corners. If the point is not int wall's plane
					or if the point is outside the polygon defined by the wall's corners, it returns a negative value.
		*	\param [in] Point: point to be checked.
		*	\param [out] Result: 0 --> Point is not in the wall's plane or distanceToBorder > THRESHOLD_BORDER
		                         1 --> Point is inside the wall 
									     visibility (sharpness) = 1.0 if distanceToBorder > THRESHOLD_BORDER
									     visibility (sharpness) is between [1.0 ,  0.5) if distanceToBorder < THRESHOLD_BORDER
								 2 --> Point is coming out of the wall
								         visibility (sharpness) is between (0.5 ,  0] if distanceToBorder <= THRESHOLD_BORDER
		*/
		int checkPointInsideWall(Common::CVector3 point, float &distanceNearestEdge, float &sharpness);

		/** \brief Returns the distance to the nearest edge of the wall.
		*	\details
		*	\param [in] Point: point to be checked.
		*	\param [out] Result: distance to the nearest edge.
		*/
		float calculateDistanceNearestEdge(Common::CVector3 point);

		/** \brief Returns the distance between a 3D point and a line in a 3D plane.
		*	\details
		*	\param [in] Point: 3D point, 3D point_1 of line,  3D point_2 of line.
		*	\param [out] Result: distance to the nearest edge.
		*/
		float distancePointToLine(Common::CVector3 point, Common::CVector3 pointLine1, Common::CVector3 pointLine2);

		/** \brief Enable the wall.
		*	\details Every wall can be active (it reflects) or not (i does not reflect anything, so it is as it does not exist.
					 This methof sets the wall as active.
		*/
		void enable() { active = true; }

		/** \brief Disable the wall.
		*	\details Every wall can be active (it reflects) or not (i does not reflect anything, so it is as it does not exist.
				 This method sets the wall as incative.
		*/
		void disable() { active = false; }

		/** \brief Returns wether the wall is active or not.
		*	\details Every wall can be active (it reflects) or not (i does not reflect anything, so it is as it does not exist.
				 This method returs wether the wall is active or not.
		*/
		bool isActive() { return active; }


	private:

		////////////
		// Methods
		////////////

		/** \brief calculates the general (cartesian) equation of the plane containing the wall. Parameters are stored in private attributes
		*/
		void calculate_ABCD();

		////////////
		// Attributes
		////////////
		std::vector<Common::CVector3> polygon;	// corners of the wall
		std::vector<float> absortionBands;      // absortion coeficients (absorved energy / incident energy) for each octave Band
		bool active;						//sets wether the wall is active or not (if false, the wall is transparent)

		float A, B, C, D;						// General Plane Eq.: Ax + By + Cz + D = 0

	};

}//namespace ISM