/**
* \class Room
*
* \brief Declaration of Room interface. This class is basically a container of walls
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
#include "Wall.h"

namespace ISM
{
	struct RoomGeometry
	{
		std::vector<Common::CVector3> corners;
		std::vector<std::vector<int>> walls;
	};

	class Room
	{
	public:
		////////////
		// Methods
		////////////

		/** \brief Initializes the object with a shoebox room
		*	\details creates six walls conforming a shoebox room with 0,0,0 at the center. It must be used right after
		*			 creating the empty object.
		*	\param [in] length: extension of the room along the X axis.
		*	\param [in] width: extension of the room along the Y axis.
		*	\param [in] height: extension of the room along the Z axis
		*/
		void setupShoeBox(float length, float width, float height);

		/** \brief Initializes the object with a shoebox room
		*	\details creates a room with arbitrary geometry by means of defining all its corners and the walls as polygons with those corners
		*	\param [in] roomGeometry: struct containing all the vertices and walls
		*/
		void setupRoomGeometry(RoomGeometry roomGeometry);

		/** \brief insert a new wall in the room
		*	\details Instead of using the setup method, this method can be used to create any arbitrary room. It should be
					 called once per wall to be inserted, after creating a new empty room.
		*	\param [in] Wall to be inserted.
		*/
		void insertWall(Wall newWall);

		/** \brief Makes one of the room's walls active
		*	\details Sets the i-th wall of the room as active and therefore reflective.
		*	\param [in] index of the wall to be active.
		*/
		void enableWall(int wallIndex);

		/** \brief Makes one of the room's walls transparent
		*	\details Sets the i-th wall of the room as not active and therefore transparent.
		*	\param [in] index of the wall to be active.
		*/
		void disableWall(int wallIndex);

		/** \brief sets the absortion coeficient (frequency independent) of one wall
		*	\details Sets the absortion coeficient (absorved energy / incident energy) of the 
		*            i-th wall of the room.
		*	\param [in] index of the wall.
		*	\param [in] absortion coeficient (frequency independent)
		*/
		void setWallAbsortion(int wallIndex, float absortion);

		/** \brief sets the absortion coeficient (frequency dependent) of one wall
		*	\details Overloads the previous one. Sets the absortion coeficient (absorved energy / incident energy) of 
		*            each of the nine bands for the i-th wall of the room.
		*	\param [in] index of the wall.
		*	\param [in] absortion coeficients for each band (frequency dependent)
		*/
		void setWallAbsortion(int wallIndex, std::vector<float> absortionPerBand);

		/** \brief Returns a vector of walls containing all the walls of the room.
		*	\param [out] Walls: vector of walls with all the walls of the room.
		*/
		std::vector<Wall> getWalls();

		/** \brief Returns a vector of image rooms
		*	\details creates an image (specular) room for each wall of this room and returns a vector contoining them.
		*	\param [out] ImageRooms: vector containing all image rooms of this room.
		*/
		std::vector<Room> getImageRooms();

		/** \brief Checks wether a 3D point is inside the room or not.
		*	\details Returns the result of checking wether a 3D point is inside the room or not and the distance to teh nearest wall 
		*            which is positive if the point is inside the room and negative if it is outside. This method assumes that the room is convex
					 and that all the walls are properly defined declaring their corners clockwise as seen from inside the room.
		*	\param [in] point: 3D point to be checked.
		*	\param [out] distance to nearest wall passed by reference
		*	\param [out] Result: returned boolean indicating if the point is inside the room (true) or not (false)
		*/
		bool checkPointInsideRoom(Common::CVector3 point, float &distanceNearestWall);

		/** \brief Returns the center of the room.
		*	\details The center is calculated as the average of the centers of the walls
		*	\param [out] center: point (CVector3) which is the center of the room.
		*/
		Common::CVector3 getCenter();

	private:
		////////////
		// Attributes
		////////////

		bool shoeBox = false;				//Flag indicating if the room was set up as a shoebox
		std::vector<Wall> walls;            //Vector with all the walls of the room
	};

}//namespace ISM