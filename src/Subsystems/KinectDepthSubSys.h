#ifndef KINECTDEPTHSUBSYS_H
#define KINECTDEPTHSUBSYS_H

#include "windows.h"
#include "NuiApi.h"
#include "NuiImageCamera.h"

//#include "KinectStreamerResources.h"

namespace jrl
{
	class KinectDepthSubSys
	{

	public:
		enum KINECT_DEPTH_RESOLUTION{
			//KINECT_DEPTH_RESOLUTION_INVALID = -1,
			KINECT_DEPTH_RESOLUTION_80x60 = 0,
			KINECT_DEPTH_RESOLUTION_320x240 = 1,
			KINECT_DEPTH_RESOLUTION_640x480 = 2
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

		inline const unsigned int getTotalDepthBytes() const{
			return totalDepthBytes;
		}

		inline void activateDepthFilter(bool activate){
			m_depthFiltersEnabled = activate;
		}
		inline const bool isDepthFilterActive() const{
			return m_depthFiltersEnabled;
		}

		inline USHORT* getDepthBuffer() const{
			return m_pLockedRawDepthBuffer;
		}

		KinectDepthSubSys();
		~KinectDepthSubSys();

		void setResolution(KINECT_DEPTH_RESOLUTION res);

		void setNearMode(bool mode);

		bool newDepthDataAvailable();

		Vector4 depthSpaceToSkelSpace(LONG depthX, LONG depthY, USHORT depthVal);

	private:

		friend class KinectDepthStreamer;
		friend class KinectDepthAndColorStreamer;

		static const int m_IntensityShiftByPlayerR[];
		static const int m_IntensityShiftByPlayerG[];
		static const int m_IntensityShiftByPlayerB[];

		NUI_IMAGE_RESOLUTION depthResolution;

		int bytesPerPixel;

		INuiSensor* sensor;

		INuiCoordinateMapper** mapperInterface;

		//FrameSize
		unsigned int frameWidth;
		unsigned int frameHeight;
		unsigned int widthByHeight;
		unsigned int totalDepthBytes;

		//Image Depth Buffer Vars
		//unsigned char*	m_pFreeDepthBuffer;

		//Raw Depth Vars
		USHORT* m_pFreeRawDepthBuffer;

		//int m_freeRawDepthBufferIndex;
		bool m_depthFiltersEnabled;

		//Threshold
		//int	m_nearThreshold;
		//int m_farThreshold;

		//unsigned char	m_depthBuffer[2][320*240*4];
		//unsigned char*		m_pFreeDepthBuffer;
		bool m_newDepthDataAvailable;
		//unsigned char*	m_pLockedDepthBuffer;
		//unsigned char m_depthBuffer[2][KSR::TOTALIMAGEBYTES/2];
		//unsigned char m_depthBuffer[2][480/2];

		//Raw Depth Vars
		USHORT	m_depthFilterBuffer[4][2][320 * 240]; //Buffers * (Depth + BitMask) * Resolution
		//char	depthBufferPlayerID[320 * 240];
		USHORT	m_depthOutputBuffer[2][320 * 240];
		USHORT* m_pLockedRawDepthBuffer;

		HANDLE        m_hNextDepthFrameEvent;
		HANDLE        m_pDepthStreamHandle;

		DWORD         m_DepthStreamFlags;

		int           m_DepthFramesTotal;
		DWORD         m_LastDepthFPStime;
		int           m_LastDepthFramesTotal;

		//Functions

		void streamerInit();
		void streamerUnInit();

		void switchDepthBuffers();

		void updateDepthStreamFlag(DWORD flag, bool value );

		inline HANDLE getEventHandle() const{
			return m_hNextDepthFrameEvent;
		}

		HRESULT subsystemInit(INuiSensor* kSensor);
		void subsystemUnInit();

		void gotDepthAlert();

		//int* createDepthFrame_depthOnly();

		void Nui_ShortToQuad_Depth( USHORT s, unsigned char *target );

		
	};
}

#endif