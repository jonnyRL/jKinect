#ifndef KINECTCOLORSUBSYS_H
#define KINECTCOLORSUBSYS_H

#include "windows.h"
#include "NuiApi.h"
//#include "KinectStreamerResources.h"
//#define KSR KinectStreamerResources

namespace jrl
{
	class KinectColorSubSys
	{

	public:

		enum KINECT_IMAGE_TYPE{
			KINECT_IMAGE_TYPE_COLOR,
			KINECT_IMAGE_TYPE_COLOR_INFRARED, //Note IR pixel is 16 bits
			KINECT_IMAGE_TYPE_COLOR_RAW_BAYER,
			KINECT_IMAGE_TYPE_COLOR_RAW_YUV,
			KINECT_IMAGE_TYPE_COLOR_YUV
			//  NUI_IMAGE_TYPE_DEPTH,
			//  NUI_IMAGE_TYPE_DEPTH_AND_PLAYER_INDEX
		};
		enum KINECT_COLOR_RESOLUTION{
			//KINECT_DEPTH_RESOLUTION_INVALID = -1,
			//KINECT_COLOR_RESOLUTION_80x60 = 0,
			//KINECT_COLOR_RESOLUTION_320x240 = 1,
			KINECT_COLOR_RESOLUTION_640x480 = 2,
			KINECT_COLOR_RESOLUTION_1280x960 = 3,
		};

		//Getters/Setters
		inline const int getFrameWidth() const{
			return frameWidth;
		}
		inline const int getFrameHeight() const{
			return frameHeight;
		}
		inline const unsigned int getWidthByHeight() const{
			return widthByHeight;
		}

		inline const unsigned int getTotalColorBytes() const{
			return totalColorBytes;
		}

		inline unsigned char* const getColorBuffer() const
		{
			return m_pLockedColorBuffer;
		}
		inline HANDLE getEventHandle() const
		{
			return m_hNextColorFrameEvent;
		}

		KinectColorSubSys();
		~KinectColorSubSys();

		void setImageType(KINECT_IMAGE_TYPE type);
		void setResolution(KINECT_COLOR_RESOLUTION res);

		HRESULT subsystemInit(INuiSensor* kSensor);
		void subsystemUnInit();

		void gotColorAlert();

		HRESULT colorSpaceToDepthSpace(LONG depthX, LONG depthY, USHORT depthVal, LONG *xTarget, LONG *yTarget);
		HRESULT colorSpaceToDepthSpaceIndex(LONG depthX, LONG depthY, USHORT depthVal, LONG* colorSetIndex);
		unsigned char* colorSpacePixelToDepthSpace(LONG depthX, LONG depthY, USHORT depthVal);
		unsigned char* colorSpacePixelToDepthSpace(unsigned char* src, LONG depthX, LONG depthY, USHORT depthVal);

	private:

		friend class KinectColorStreamer;
		friend class KinectDepthAndColorStreamer;

		NUI_IMAGE_TYPE colorImageType;
		NUI_IMAGE_RESOLUTION colorResolution;
		NUI_IMAGE_RESOLUTION depthResolution;

		int bytesPerPixel;

		INuiSensor* sensor;
		//Frame size
		unsigned int frameWidth;
		unsigned int frameHeight;
		unsigned int widthByHeight;
		unsigned int totalColorBytes;

		

		//Color Buffer Vars
		unsigned char*	m_pFreeColorBuffer;
		//Color Buffer Vars
		unsigned char* m_pLockedColorBuffer;
		//unsigned char m_colorBuffer[2][KSR::TOTALIMAGEBYTES];

		bool m_newColorDataAvailable;

		HANDLE        m_hNextColorFrameEvent;
		HANDLE        m_pVideoStreamHandle;

		void streamerInit();
		void streamerUnInit();

		bool newColorData();

		void setDepthResolutionForSpaceTransformation(NUI_IMAGE_RESOLUTION res);

		DWORD WINAPI subsystemThreadFunction();
	};
}

#endif