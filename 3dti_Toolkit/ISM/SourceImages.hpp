#ifndef _SOURCE_IMAGES_HPP_
#define _SOURCE_IMAGES_HPP_

#include "Room.h"
#include "ISM.hpp"
#include "ISMParameters.hpp"
#include <Common/Vector3.h>
#include <Common/CascadeGraphicEq9OctaveBands.h>

namespace ISM
{	
	//Struct to store all the data of the image sources
	struct ImageSourceData
	{
		Common::CVector3 location;						//Location of the image source
		bool visible;									//If the source is visible it should be rendered
		float visibility;								//1 if visible, 0 if not, something in the middle if in the transition, where the transition is +/-VISIBILITY_MARGIN width
		std::vector<Wall> reflectionWalls;				//list of walls where the source has reflected (last reflection first)
		std::vector<float> reflectionBands;             //coeficients, for each octave Band, to be applied to simulate walls' absortion
	};

	class SourceImages {
	public:
		SourceImages(std::shared_ptr<CISMParameters> _ISMParameters)
			: visible {false}
			, visibility{ 0 }
			, sourceLocation{ Common::CVector3(0,0,0) }
			, eq{ (float)_ISMParameters->sampleRate}
		{								
			ISMParameters = _ISMParameters;			
		}
		

		/** \brief Returns the location of the original source
		*   \param [out] Location: Current location for the original source.
		*/
		Common::CVector3 getImageLocation()		
		{
			return sourceLocation;
		}

		/** \brief Returns the first order reflections of the original source
		*   \param [out] Images: vector with the first order reflection images.
		*/
		/*std::vector<weak_ptr <SourceImages2>> getImages()
		{
			vector<weak_ptr<SourceImages2>> result;
			for (auto i = 0; i < imagesTree.size(); ++i) {
				result.push_back(weak_ptr<SourceImages2>(imagesTree[i]));
			}
			return result;
		}*/

		/** \brief Returns the locations of all images but the original source
		*   \details this method recurively goes through the image tree to collect all the image locations
		*   \param [out] imageSourceList: vector containing all image locations.
		*/
		void getImageLocations(std::vector<Common::CVector3>& imageSourceList)		
		{					
			for (auto & image : imagesTree) {
				if (image->getMyReflectionWall().isActive()) {
					imageSourceList.push_back(image->getImageLocation());
					image->getImageLocations(imageSourceList);
				}
			}
			
		}
	
		/** \brief Returns data of all image sources
		*	\details This method returns the location of all image sources and wether they are visible or not, not including the
			original source (direct path).
		*	\param [out] imageSourceDataList: Vector containing the data of the image sources
		*/
		void getImageSourcesData(std::vector<ImageSourceData>& imageSourceDataList)		
		{			
			for (auto& image : imagesTree) {
				ImageSourceData temp;
				temp.location = image->getImageLocation();
				temp.reflectionWalls = image->reflectionWallsPath;
				temp.reflectionBands = image->reflectionBands;
				temp.visibility = image->visibility;
				temp.visible = image->visible;
				imageSourceDataList.push_back(temp);  //Once created, the image source data is added to the list
				image->getImageSourcesData(imageSourceDataList); //recurse to the next level
			}
		}

		/**
		 * @brief Creates all image sources up to a given order
		 * @details Creates a recursive tree of imagesources using all active walls up to the reflection order depth. This methos should be
					called every time the room geometry changes (walls are set as active or inactive), the reflection order changes
					or time parameters change (max distance, transition meters)
		 * @param _room Real (original) room geometry
		 * @param _order recursion depth
		 * @param _sourceLocation Current location of the real (original) source
		 */
		void createImagesTree(const Room& _room, const int& _order, const Common::CVector3& _sourceLocation) {
			imagesTree.clear();
						
			std::vector<Wall> path;			
			try {
				path.reserve(_order);
			}
			catch (const std::bad_alloc& e) {
				// Capture the exception if memory allocation fails
				// The operating system could not reserve the requested space				
			}			
			std::vector<float> absorptionCoefficients = std::vector<float>(NUM_BAND_ABSORTION, 1.0f);

			sourceLocation = _sourceLocation;
			createImagesTree(_room, _order, path, absorptionCoefficients);
			
			// TODO LIBERAR MEMORIA RESERVADA previamente EN PATH 
			// mediante std::vector<Wall>(path).swap(path);
			
			UpdateImagesTreeVisibilities();			
		}
						
		/** \brief updates imege source location, reflection and visibility
		*	\details Updates the recursive image source tree with the source locations and computes refelction coefficients and visibility
					 to be applied when process
		*/
		void UpdateSourceLocation(const Common::CVector3& _sourceLocation)
		{												
			for (auto& image : imagesTree) {
				image->SetSourceLocation(image->getMyReflectionWall().getImagePoint(_sourceLocation));
			}
			UpdateImageVisibility();			
		}

		/**
		 * @brief Update visibility for all images in the tree
		 */
		void UpdateImagesTreeVisibilities() {
			for (auto& image : imagesTree) {
				image->UpdateImagesTreeVisibilities();
			}		
			UpdateImageVisibility();
		}		

		/*void UpdateImagesTreeWallsAbsorptionCoefficients() {						
			for (auto& image : imagesTree) {
				image->UpdateImagesTreeWallsAbsorptionCoefficients();
			}
			UpdateImageWallsAbsorptionCoefficients();
		}*/


		/** \brief Adds wall absortion to the sound
		*	\details Recursively process all source images providing an independent buffer for each of them with the original sound filtered
					 by the wall absortions. For non visible sources the output buffer contains zeros
		*	\param [in] inBuffer: original buffer used for all images
		*   \param [out] imageBuffers: vector of buffers with the sound filtered (One buffer per image)
		*	\param [in] listenerLocation: needed to know visibility of sources
		*/
		void processAbsortion(CMonoBuffer<float> inBuffer, std::vector<CMonoBuffer<float>>& imageBuffers, Common::CVector3 listenerLocation)
		{
			for (int i = 0; i < imagesTree.size(); i++)  //process buffers for each of the image sources, adding the result to the output vector of buffers
			{

				CMonoBuffer<float> tempBuffer(inBuffer.size(), 0.0);

				if (imagesTree.at(i)->visibility > 0.00001)
					imagesTree.at(i)->eq.Process(inBuffer, tempBuffer);
				imageBuffers.push_back(tempBuffer);
				imagesTree.at(i)->processAbsortion(inBuffer, imageBuffers, listenerLocation);
			}
		}

		void Reset() {
			imagesTree.clear();
			reflectionWallsPath.clear();
			reflectionBands.clear();
			sourceLocation = Common::CVector3(0, 0, 0);
			visibility = 1.0;
			visible = true;
			eq = Common::CascadeGraphicEq9OctaveBands(ISMParameters->sampleRate);
		}

	private:

		/**
		 * @brief Check visibility through all reflection walls and compute a visibility coeficient
		 */
		void UpdateImageVisibility() {
			visibility = 1.0;
			visible = true;
			
			float distanceImageToListener = (ISMParameters->listenerLocation - sourceLocation).GetDistance();
			float upperBorder = ISMParameters->maxDistanceSourcesToListener + 0.5f * ISMParameters->transitionMeters;
			float lowerBorder = ISMParameters->maxDistanceSourcesToListener - 0.5f * ISMParameters->transitionMeters;
			
			if (distanceImageToListener > upperBorder)
			{
				visible = false;
				visibility = 0.0;
				return;
			}
			
			if (!reflectionWallsPath.empty()) {
				for (auto& wall : reflectionWallsPath) {
					float distanceToBorder;
					float wallVisibility;
					Common::CVector3 reflectionPoint = wall.getIntersectionPointWithLine(sourceLocation, ISMParameters->listenerLocation);
					wall.checkPointInsideWall(reflectionPoint, distanceToBorder, wallVisibility);
					visibility *= wallVisibility;
					visible &= (wallVisibility > 0);
					
					if (visibility == 0.0f) break; // It is possible to do this more robustly by using an epsilon comparison
				}
				
				visibility = std::pow(visibility, (1 / (float)reflectionWallsPath.size()));
			}

			if (distanceImageToListener > lowerBorder) {
				visibility *= 0.5f + 0.5f * std::cos(PI * (distanceImageToListener - lowerBorder) / ISMParameters->transitionMeters);
			}

		}

		void UpdateImageWallsAbsorptionCoefficients() {
			std::vector<float> absorptionCoefficients = std::vector<float>(NUM_BAND_ABSORTION, 1.0f);
			for (auto& wall : reflectionWallsPath) {
				AddCoefficientsFromWall(absorptionCoefficients, wall); // Calculate EQ
			}
			reflectionBands.assign(absorptionCoefficients.begin(), absorptionCoefficients.end());
			eq.SetCommandGains(ISMParameters->sampleRate, absorptionCoefficients);
		}


		/** \brief changes the location of the original source
		*	\details Sets a new location for the original source and updates all images accordingly.
		*   \param [in] _location: new location for the original source.
		*/
		void SetSourceLocation(const Common::CVector3& _sourceLocation)
		{			
			sourceLocation = _sourceLocation;
			UpdateSourceLocation(_sourceLocation);
		}



		void createImagesTree(const Room& _currentRoom, int order, std::vector<Wall>& path, std::vector<float>& absorptionCoefficients) {
			if (order == 0) return;
			
			const auto& wallsList = _currentRoom.getWalls();
			for (auto& wall : wallsList) {
				
				if (!wall.isActive()) continue;

				const auto newImageLocation = wall.getImagePoint(sourceLocation);
				Common::CVector3 realRoomCenter = ISMParameters->room.getCenter();

				// Filter out reflections that are not physically possible				
				/* If the image is closer to the room center than the previous original, 
				/  that reflection is not real and should not be included.
				/  this is equivalent to determine wether source and room center are on 
				/  the same side of the wall or not.
				*/
				float distanceRoomRealSource = (realRoomCenter - sourceLocation).GetSqrDistance();
				float distanceRoomImageSource = (realRoomCenter - newImageLocation).GetSqrDistance();			
				if (is_greater_or_equal(distanceRoomRealSource, distanceRoomImageSource)) continue;
			
				// Filter out reflections that are too far away
				/* If the image is in a room that is too far away, do not create it.
				/  the distance criterion can be static or dynamic
				*/
				float roomsDistance = CalculateRoomsDistance(wall, path, newImageLocation, ISMParameters->listenerLocation);				
				float maxDistanceImageSources = CalculateMaxDistanceImageSources();				
				if (is_greater(roomsDistance, maxDistanceImageSources)) continue;
				
				path.push_back(wall);
						
				//std::vector<std::shared_ptr<Wall>> path2;
				//path2.push_back(std::make_shared<Wall>(wall));

				auto child = std::make_shared<SourceImages>(ISMParameters);
				child->sourceLocation = newImageLocation;				
				child->reflectionWallsPath.assign(path.begin(), path.end());
				
				AddCoefficientsFromWall(absorptionCoefficients, wall); // Calculate EQ
				child->reflectionBands.assign(absorptionCoefficients.begin(), absorptionCoefficients.end());				
				child->eq.SetCommandGains(ISMParameters->sampleRate, absorptionCoefficients);

				if (order > 1) {
					Room nextRoom;
					for (auto& wj : wallsList)
						nextRoom.insertWall(wall.getImageWall(wj));
					child->createImagesTree(nextRoom, order - 1, path, absorptionCoefficients);
				}				

				imagesTree.push_back(std::move(child));
				path.pop_back();
				RemoveCoefficientsFromWall(absorptionCoefficients, wall);				
			}
		}
				
		void AddCoefficientsFromWall(std::vector<float>& absorptionCoefficients, const Wall& wall) {
			for (int n = 0; n < absorptionCoefficients.size(); n++) {
				absorptionCoefficients[n] *= std::sqrt(1 - wall.getAbsortionB().at(n));
			}			
		}
		void RemoveCoefficientsFromWall(std::vector<float>& absorptionCoefficients, const Wall& wall) {
			for (int n = 0; n < absorptionCoefficients.size(); n++) {
				absorptionCoefficients[n] /= std::sqrt(1 - wall.getAbsortionB().at(n));
			}
		}

		float CalculateRoomsDistance(const Wall& wall, const std::vector<Wall>& path, const Common::CVector3& imgPos, const Common::CVector3& listenerLocation) {
			float roomsDistance = 0.0;
			if (path.size() > 0)
			{
				// the distance criterion can be static or dynamic
				if (ISMParameters->staticDistanceCriterion == false)
					roomsDistance = wall.getMinimumDistanceFromWall(path.front());
				else					
					roomsDistance = (listenerLocation - imgPos).GetDistance();
			}
			return roomsDistance;
		}

		float CalculateMaxDistanceImageSources() {
			// the distance criterion can be static or dynamic
			float maxDistanceImageSources;
			if (ISMParameters->staticDistanceCriterion == false)
				maxDistanceImageSources = ISMParameters->maxDistanceSourcesToListener;
			else
				maxDistanceImageSources = ISMParameters->maxDistanceSourcesToListener + ISMParameters->transitionMeters * 0.5;
			return maxDistanceImageSources;
		}



		/** \brief Returns the  wall where the reflecion produced this image
		*   \param [out] Reflection wall.
		*/
		Wall getMyReflectionWall()
		{
			return reflectionWallsPath.back();
		}


		/**
	 * @brief Determines if the first value is strictly greater than the second, excluding cases where they are almost equal.
	 * @tparam T The type of the values to compare.
	 * @param a The first value to compare.
	 * @param b The second value to compare.
	 * @return True if 'a' is greater than 'b' and they are not almost equal; otherwise, false.
	 */
		template <typename T>
		bool is_greater(T a, T b) {
			return (a > b) && !almostEqual(a, b);
		}

		/**
		 * @brief Determines if the first value is greater than or approximately equal to the second value.
		 * @tparam T The type of the values to compare.
		 * @param a The first value to compare.
		 * @param b The second value to compare.
		 * @return True if the first value is greater than or approximately equal to the second value; otherwise, false.
		 */
		template <typename T>
		bool is_greater_or_equal(T a, T b) {
			return (a > b) || almostEqual(a, b);
		}
	
		/**
	 * @brief This method checks if two floating point numbers are almost equal, considering both absolute and relative tolerances.
	 * @details The method uses default tolerances based on the type of floating point number (float, double, long double).
	 * @tparam type T
	 * @param a First floating point number.
	 * @param b Second floating point number.
	 * @return true if the numbers are almost equal, false otherwise.
	 */
		template <typename T>
		bool almostEqual(T a, T b) {
			T relEpsilon;
			T absEpsilon;

			if constexpr (std::is_same_v<T, float>) {
				relEpsilon = 1e-5f;
				absEpsilon = 1e-8f;
			}
			else if constexpr (std::is_same_v<T, double>) {
				relEpsilon = 1e-12;
				absEpsilon = 1e-15;
			}
			else if constexpr (std::is_same_v<T, long double>) {
				relEpsilon = 1e-15L;
				absEpsilon = 1e-18L;
			}
			else {
				// generic fallback: use epsilon of the implementation
				relEpsilon = std::numeric_limits<T>::epsilon();
				absEpsilon = std::numeric_limits<T>::epsilon();
			}

			T diff = std::fabs(a - b);

			// Absolute comparison (for small values, close to 0)
			if (diff <= absEpsilon) {
				return true;
			}

			// Relative comparison (for large values)
			return diff <= relEpsilon * std::max(std::fabs(a), std::fabs(b));
		}

		////////////////
		/// Attributes
		////////////////		
				
				
	
		bool visible;									// false when visibility = 0, true otherwise
		float visibility;								// 1.0 if visible, 0.0 if not, something in the middle if the ray is close to the border of walls
		Common::CVector3 sourceLocation;				// Source location		
		std::vector<Wall> reflectionWallsPath;			// vector containing the walls where the sound has been reflected in inverse order (last reflection first)
		std::vector<std::shared_ptr<Wall>> reflectionWallsPath2;			// vector containing the walls where the sound has been reflected in inverse order (last reflection first)
		std::vector<float> reflectionBands;				// coeficients, for each octave Band, to be applied to simulate walls' absortion
		std::vector<std::shared_ptr<SourceImages>> imagesTree;	// recursive list of images			
		Common::CascadeGraphicEq9OctaveBands eq;		// Filter to simulate walls' absortion		

		std::shared_ptr<CISMParameters> ISMParameters;	// To access ISM parameters
		

};
}
#endif 