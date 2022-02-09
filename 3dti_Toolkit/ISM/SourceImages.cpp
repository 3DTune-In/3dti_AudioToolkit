#include "SourceImages.h"
#include <cmath>

//#ifndef THRESHOLD_BORDER
//#define THRESHOLD_BORDER 0.2f
//#endif

namespace ISM
{

	void SourceImages::setLocation(Common::CVector3 _location)
	{
		sourceLocation = _location;
		updateImages();
	}

	Common::CVector3 SourceImages::getLocation()
	{
		return sourceLocation;
	}

	std::vector<SourceImages> SourceImages::getImages()
	{
		return images;
	}

	void SourceImages::getImageLocations(std::vector<Common::CVector3> &imageSourceList,
		int reflectionOrder)
	{
		if (reflectionOrder > 0)
		{
			reflectionOrder--;
			for (int i = 0; i < images.size(); i++)
			{
				if (images.at(i).reflectionWall.isActive())
				{
					imageSourceList.push_back(images.at(i).getLocation());
					if (reflectionOrder > 0)
					{
						images.at(i).getImageLocations(imageSourceList, reflectionOrder);
					}
				}
			}
		}

	}


	void SourceImages::getImageData(std::vector<ImageSourceData> &imageSourceDataList, Common::CVector3 listenerLocation, int reflectionOrder)
	{
		if (reflectionOrder > 0)
		{
			reflectionOrder--;
			for (int i = 0; i < images.size(); i++)

			{
				if (images.at(i).reflectionWall.isActive())//////////////////////////////////////////////////////////////////
				{
					ImageSourceData temp;
					temp.location = images.at(i).getLocation();
					temp.reflectionWalls = images.at(i).reflectionWalls;
					temp.visible = true;	//We hypothesise that it is visible and in case on founding a wall where it is not, this will become false
					temp.visibility = 1.0;	//We hypothesise that it is fully visible. Otherwise, this will become lower
					temp.reflection = 1.0;	//We start asuming pure reflective walls 
					temp.reflectionBands.empty(); 	//We start asuming pure reflective walls 
					for (int k = 0; k < NUM_BAND_ABSORTION; k++)
						temp.reflectionBands.push_back(1.0);	//We start asuming pure reflective walls 

					for (int j = 0; j < temp.reflectionWalls.size(); j++)
					{
						Common::CVector3 reflectionPoint = temp.reflectionWalls.at(j).getIntersectionPointWithLine(images.at(i).getLocation(), listenerLocation);
						float distanceToBorder, visibility;
						
						temp.reflectionWalls.at(j).checkPointInsideWall(reflectionPoint, distanceToBorder, visibility);
						/*float visibility = 0.5 + distanceToBorder / (THRESHOLD_BORDER * 2.0);  // >1 if further inside than VISIBILITY_MARGIN and <-1 if further outside than VISIBILITY_MARGIN
						if (visibility > 1) visibility = 1;
						if (visibility < 0) visibility = 0;
						*/
						temp.visibility *= visibility;
						temp.visible &= (visibility > 0);

						//reflection as scalar value
						temp.reflection *= sqrt(1 - temp.reflectionWalls.at(j).getAbsortion()); 
						// /*
						//reflection as vector
						std::vector<float> absortionBands = temp.reflectionWalls.at(j).getAbsortionB();
						if (temp.reflectionBands.size() == absortionBands.size()) {
							for (int k = 0; k < absortionBands.size(); k++)
								temp.reflectionBands[k] *= sqrt(1 - absortionBands[k]);
						}
						else {
							// error
							for (int k = 0; k < temp.reflectionBands.size(); k++)
								temp.reflectionBands[k] *= sqrt(1 - absortionBands[k]);
						}
                        //*/
					}
					temp.visibility = pow(temp.visibility, (1 / (float)temp.reflectionWalls.size()));
					imageSourceDataList.push_back(temp);
					if (reflectionOrder > 0)
					{
						images.at(i).getImageData(imageSourceDataList, listenerLocation, reflectionOrder);
					}
				}
			}
		}

	}


	void SourceImages::setReflectionWall(Wall _reflectionWall)
	{
		reflectionWall = _reflectionWall;
	}

	void SourceImages::setReflectionWalls(std::vector<Wall> _reflectionWalls)
	{
		reflectionWalls = _reflectionWalls;
	}

	Wall SourceImages::getReflectionWall()
	{
		return reflectionWall;
	}

	void SourceImages::createImages(Room _room, Common::CVector3 listenerLocation, int reflectionOrder)
	{
		images.clear();
		createImages(_room, listenerLocation, reflectionOrder, reflectionWalls);
	}

	void SourceImages::createImages(Room _room, Common::CVector3 listenerLocation, int reflectionOrder, std::vector<Wall> reflectionWalls)
	{
		reflectionOrder--;
		std::vector<Wall> walls = _room.getWalls();
		for (int i = 0; i < walls.size(); i++)
		{
			if (walls.at(i).isActive())
			{
				SourceImages tempSourceImage;
				Common::CVector3 tempImageLocation = walls[i].getImagePoint(sourceLocation);

				// if the image is closer to the listener than the previous original, that reflection is not real and should not be included
				// this is equivalent to determine wether source and listener are on the same side of the wall or not
				if ((listenerLocation - sourceLocation).GetDistance() < (listenerLocation - tempImageLocation).GetDistance())
				{
					tempSourceImage.setLocation(tempImageLocation);
					tempSourceImage.setReflectionWall(walls.at(i));
					reflectionWalls.push_back(walls.at(i));
					tempSourceImage.reflectionWalls = reflectionWalls;

					if (reflectionOrder > 0)
					{
						// We need to calculate the image room before asking for all the new images
						Room tempRoom;
						for (int j = 0; j < walls.size(); j++)
						{
							Wall tempWall = walls.at(i).getImageWall(walls.at(j));
							tempRoom.insertWall(tempWall);
						}
						tempSourceImage.createImages(tempRoom, listenerLocation, reflectionOrder, reflectionWalls);
						surroundingRoom = tempRoom;
					}

					images.push_back(tempSourceImage);
					reflectionWalls.pop_back();
				}
			}
		}
	}

	void SourceImages::updateImages()
	{
		for (int i = 0; i < images.size(); i++)
		{
			//FIXME: When some images disappear or reappear, this has to be done differently
			images[i].setLocation(images.at(i).getReflectionWall().getImagePoint(sourceLocation));
		}
	}


}//namespace ISM
