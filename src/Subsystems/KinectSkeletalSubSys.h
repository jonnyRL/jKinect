// Declares of KinectStreamerSkeletal class

#ifndef KINECTSKELETALSUBSYS_H
#define KINECTSKELETALSUBSYS_H

#include "windows.h"
#include "NuiApi.h"

//#include "KinectStreamerResources.h"
//#define KinectStreamerResources KSR

namespace jrl
{
	class KinectSkeletalSubSys
	{

	public:

		///Returns a reference to the skeletal data processed by the device.
		///@return The reference to the skeletal data.
		inline const NUI_SKELETON_DATA& getActiveSkeletonData() const
		{
			return m_pFreeSkeletonBuffer->SkeletonData[m_activePlayerID];
		}

		inline HANDLE getEventHandle() const
		{
			return m_hNextSkeletonEvent;
		}

		KinectSkeletalSubSys();
		~KinectSkeletalSubSys();

		HRESULT subsystemInit(INuiSensor* kSensor);
		void subsystemUnInit();

		void gotSkeletonAlert();

	private:
		//Kinect Constants
		static const COLORREF m_JointColorTable[NUI_SKELETON_POSITION_COUNT];
		static const COLORREF m_SkeletonColors[NUI_SKELETON_COUNT];

		static const NUI_TRANSFORM_SMOOTH_PARAMETERS m_verySmoothParams;
		static const NUI_TRANSFORM_SMOOTH_PARAMETERS m_somewhatLatentParams;
		static const NUI_TRANSFORM_SMOOTH_PARAMETERS m_defaultParams;

		INuiSensor* sensor;
		//Skeletons vars
		int m_numSkeletonsFound;
		float m_skeletonsPerSecond;

		NUI_SKELETON_FRAME* m_pFreeSkeletonBuffer;

		//Skeleton Vars
		NUI_SKELETON_FRAME m_skeletonBuffer[2];
		NUI_SKELETON_FRAME* m_pLockedSkeletonBuffer;
		bool m_newSkeletalDataAvailable;

		int m_activePlayerID;

		//Thread handling
		HANDLE        m_hNextSkeletonEvent;

		DWORD         m_LastSkeletonFoundTime;

		//bool          m_bAppTracking;


		void streamerInit();
		void streamerUnInit();

		bool newSkeletalData();

	};
}

#endif