#include "SourceImages.h"
#include "ISM.h"
#include <cmath>
#include <Common/BiquadFilter.h>

//#ifndef THRESHOLD_BORDER
//#define THRESHOLD_BORDER 0.2f
//#endif

namespace ISM
{
	SourceImages::SourceImages(ISM::CISM* _ownerISM) : 
		ownerISM{_ownerISM}, 
		eq{_ownerISM->GetSampleRate()}
	{
		sourceLocation = Common::CVector3(0, 0, 0);
		visibility = 1.0;
		visible = true;
			
	}

	void SourceImages::setLocation(Common::CVector3 _location)
	{
		sourceLocation = _location;
		updateImages();
	}

	Common::CVector3 SourceImages::getLocation()
	{
		return sourceLocation;
	}

	std::vector<weak_ptr <SourceImages>> SourceImages::getImages()
	{
		vector<weak_ptr<SourceImages>> result;
		for (auto i = 0; i < images.size(); ++i) {
			result.push_back(weak_ptr<SourceImages>(images[i]));
		}
		return result;
	}

	void SourceImages::getImageLocations(std::vector<Common::CVector3> &imageSourceList)
	{
			for (int i = 0; i < images.size(); i++)
			{
				if (images.at(i)->reflectionWalls.back().isActive())
				{
					imageSourceList.push_back(images.at(i)->getLocation());
					images.at(i)->getImageLocations(imageSourceList);
				}
			}
	}


	void SourceImages::getImageData(std::vector<ImageSourceData> &imageSourceDataList)
	{
		for (int i = 0; i < images.size(); i++)
		{
			ImageSourceData temp;
			temp.location = images.at(i)->getLocation();
			temp.reflectionWalls = images.at(i)->reflectionWalls;
			temp.reflectionBands = images.at(i)->reflectionBands;
			temp.visibility = images.at(i)->visibility;
			temp.visible = images.at(i)->visible;
			imageSourceDataList.push_back(temp);  //Once created, the image source data is added to the list

			images.at(i)->getImageData(imageSourceDataList); //recurse to the next level
		}
	}

	//ARCADIO: is this needed? it seems it is not used
	void SourceImages::setReflectionWalls(std::vector<Wall> _reflectionWalls)
	{
		reflectionWalls = _reflectionWalls;
	}

	Wall SourceImages::getReflectionWall()
	{
		return reflectionWalls.back();
	}

	void SourceImages::createImages(Room _room, int reflectionOrder)
	{
		images.clear();		
		createImages(_room, reflectionOrder, reflectionWalls);		
		updateImages();
	}

	void SourceImages::createImages(Room _room, int reflectionOrder, std::vector<Wall> reflectionWalls)
	{
		Common::CVector3 roomCenter = ownerISM->getRoom().getCenter();
		Common::CTransform listenerTransform = ownerISM->GetListener()->GetListenerTransform();
		Common::CVector3 listenerLocation = listenerTransform.GetPosition();

		/// Test all requirements to create a new image source

		// The reflection order is decremented at each level of the recursive tree
		if (reflectionOrder > 0) //if the reflection order is already 0 no more images should be created. We are at the leaves of the tree
		{
			reflectionOrder--;
			std::vector<Wall> walls = _room.getWalls();
			for (int i = 0; i < walls.size(); i++)  //for each wall an image is created
			{
				if (walls.at(i).isActive()) //if the wall is not active, its image is not created
				{
					Common::CVector3 tempImageLocation = walls[i].getImagePoint(sourceLocation);

					// if the image is closer to the room center than the previous original, that reflection is not real and should not be included
					// this is equivalent to determine wether source and room center are on the same side of the wall or not
					if (((roomCenter - sourceLocation).GetDistance() < (roomCenter - tempImageLocation).GetDistance()))
					{
						// If the image room where the candidate image source is far from the original room, so that any location in it is further than
						// the maximum distance to any location in the original room, then the recursive tree has to stop growing

						//if there are no reflection walls, the minimum distance is 0 and it is not necessary to calculate it. This way, -
						//we avoid to get wall from the reflectionWalls vector, which is empty.
						float roomsDistance = 0.0;
						if(reflectionWalls.size()>0) 
						{
							// the distance criterion can be static or dynamic
							if (ownerISM->staticDistanceCriterion == false)
								roomsDistance = walls.at(i).getMinimumDistanceFromWall(reflectionWalls.front());
							else
							    roomsDistance = (listenerLocation - tempImageLocation).GetDistance();
						}

						// the distance criterion can be static or dynamic
						float maxDistanceImageSources;
						if (ownerISM->staticDistanceCriterion == false)
							maxDistanceImageSources = ownerISM->getMaxDistanceImageSources();
						else
							maxDistanceImageSources = ownerISM->getMaxDistanceImageSources() + ownerISM->transitionMeters * 0.5;

						if (roomsDistance <= maxDistanceImageSources)
						{
							//The new candidate meets all requirements and will be a source image. It is therefor finally completed
							shared_ptr<SourceImages> tempSourceImage = make_shared< SourceImages>(ownerISM);
							std::vector<float> reflectionBands(NUM_BAND_ABSORTION, 1.0);
							tempSourceImage->reflectionBands = reflectionBands;
							tempSourceImage->setLocation(tempImageLocation);
							reflectionWalls.push_back(walls.at(i));
							tempSourceImage->reflectionWalls = reflectionWalls;

							tempSourceImage->eq.RemoveFilters();  //Remove all previously created filters

							// Calculate the reflection coefficients from the absortion coefficients of the reflection walls
							for (int n = 0; n < NUM_BAND_ABSORTION; n++)
							{
								for (int j = 0; j < reflectionWalls.size(); j++)
								{
									tempSourceImage->reflectionBands[n] *= sqrt(1 - reflectionWalls.at(j).getAbsortionB().at(n));
								}
							}

							// Set the reflection coefficients to the equalizer
							tempSourceImage->eq.SetCommandGains(ownerISM->GetSampleRate(),tempSourceImage->reflectionBands);

							
							if (reflectionOrder > 0)  //Still higher order reflections: we need to create images of the image just created
							{
								// We need to calculate the image room before asking for all the new images
								Room tempRoom;
								for (int j = 0; j < walls.size(); j++)
								{
									Wall tempWall = walls.at(i).getImageWall(walls.at(j));
									tempRoom.insertWall(tempWall);
								}
								tempSourceImage->createImages(tempRoom, reflectionOrder, reflectionWalls);
							}
							images.push_back(tempSourceImage);
							reflectionWalls.pop_back();
						}
					}
				}
			}
		}
	}

	void SourceImages::updateImages()
	{	
		Common::CTransform listenerTransform = ownerISM->GetListener()->GetListenerTransform();
		Common::CVector3 listenerLocation = listenerTransform.GetPosition();

		float distanceMargin = ownerISM->transitionMeters;
		distanceMargin *= 1; // METERS_OF_MARGIN;
				
		for (int i = 0; i < images.size(); i++)
		{
			images[i]->setLocation(images.at(i)->getReflectionWall().getImagePoint(sourceLocation));
		}

		//Check visibility through all reflection walls and compute a visibility coeficient
		visibility = 1.0;	//We hypothesise that it is fully visible. Otherwise, this will become lower
		visible = true;

		Common::CVector3 roomCenter = ownerISM->getRoom().getCenter();
	
		float distanceImageToLisener, distAux1;
		distanceImageToLisener = (listenerLocation - sourceLocation).GetDistance();
		float maxDistanceSourcesToListener = ownerISM->getMaxDistanceImageSources();
			
		if (distanceImageToLisener > (maxDistanceSourcesToListener + distanceMargin * 0.5))
		{
			visible = false; 
			visibility = 0.0;
		}
		else 
		{
			for (int j = 0; j < reflectionWalls.size(); j++)
			{
				Common::CVector3 reflectionPoint = reflectionWalls.at(j).getIntersectionPointWithLine(sourceLocation, listenerLocation);
				float distanceToBorder, wallVisibility;
				reflectionWalls.at(j).checkPointInsideWall(reflectionPoint, distanceToBorder, wallVisibility);
				visibility *= wallVisibility;
				visible &= (wallVisibility > 0);
			}
			visibility = pow(visibility, (1 / (float)reflectionWalls.size())); 
			if (distanceImageToLisener > (maxDistanceSourcesToListener - distanceMargin * 0.5))
			{
				visibility *= 0.5 + 0.5 * cos(PI * (distanceImageToLisener - (maxDistanceSourcesToListener - distanceMargin * 0.5)) / distanceMargin);
			}
		}
	}


	void SourceImages::processAbsortion(CMonoBuffer<float> inBuffer, std::vector<CMonoBuffer<float>> &imageBuffers, Common::CVector3 listenerLocation)
	{
		for (int i = 0; i < images.size();i++)  //process buffers for each of the image sources, adding the result to the output vector of buffers
		{
			
			CMonoBuffer<float> tempBuffer(inBuffer.size(), 0.0);

			if (images.at(i)->visibility > 0.00001)
			   images.at(i)->eq.Process(inBuffer, tempBuffer);
			imageBuffers.push_back(tempBuffer);
			images.at(i)->processAbsortion(inBuffer, imageBuffers, listenerLocation);
		}
	}


}//namespace ISM
