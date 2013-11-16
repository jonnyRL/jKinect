#include "KinectSkeletalSubSys.h"
#include <iostream>

namespace jrl
{
	const COLORREF KinectSkeletalSubSys::m_JointColorTable[NUI_SKELETON_POSITION_COUNT] = 
	{
		RGB(169, 176, 155), // NUI_SKELETON_POSITION_HIP_CENTER
		RGB(169, 176, 155), // NUI_SKELETON_POSITION_SPINE
		RGB(168, 230, 29),  // NUI_SKELETON_POSITION_SHOULDER_CENTER
		RGB(200, 0,   0),   // NUI_SKELETON_POSITION_HEAD
		RGB(79,  84,  33),  // NUI_SKELETON_POSITION_SHOULDER_LEFT
		RGB(84,  33,  42),  // NUI_SKELETON_POSITION_ELBOW_LEFT
		RGB(255, 126, 0),   // NUI_SKELETON_POSITION_WRIST_LEFT
		RGB(215,  86, 0),   // NUI_SKELETON_POSITION_HAND_LEFT
		RGB(33,  79,  84),  // NUI_SKELETON_POSITION_SHOULDER_RIGHT
		RGB(33,  33,  84),  // NUI_SKELETON_POSITION_ELBOW_RIGHT
		RGB(77,  109, 243), // NUI_SKELETON_POSITION_WRIST_RIGHT
		RGB(37,   69, 243), // NUI_SKELETON_POSITION_HAND_RIGHT
		RGB(77,  109, 243), // NUI_SKELETON_POSITION_HIP_LEFT
		RGB(69,  33,  84),  // NUI_SKELETON_POSITION_KNEE_LEFT
		RGB(229, 170, 122), // NUI_SKELETON_POSITION_ANKLE_LEFT
		RGB(255, 126, 0),   // NUI_SKELETON_POSITION_FOOT_LEFT
		RGB(181, 165, 213), // NUI_SKELETON_POSITION_HIP_RIGHT
		RGB(71, 222,  76),  // NUI_SKELETON_POSITION_KNEE_RIGHT
		RGB(245, 228, 156), // NUI_SKELETON_POSITION_ANKLE_RIGHT
		RGB(77,  109, 243)  // NUI_SKELETON_POSITION_FOOT_RIGHT
	};

	const COLORREF KinectSkeletalSubSys::m_SkeletonColors[NUI_SKELETON_COUNT] =
	{
		RGB( 255, 0, 0),
		RGB( 0, 255, 0 ),
		RGB( 64, 255, 255 ),
		RGB( 255, 255, 64 ),
		RGB( 255, 64, 255 ),
		RGB( 128, 128, 255 )
	};

	const NUI_TRANSFORM_SMOOTH_PARAMETERS KinectSkeletalSubSys::m_verySmoothParams =
	{0.7f, 0.3f, 1.0f, 1.0f, 1.0f};

	const NUI_TRANSFORM_SMOOTH_PARAMETERS KinectSkeletalSubSys::m_somewhatLatentParams =
	{0.5f, 0.1f, 0.5f, 0.1f, 0.1f};

	const NUI_TRANSFORM_SMOOTH_PARAMETERS KinectSkeletalSubSys::m_defaultParams =
	{0.5f, 0.5f, 0.5f, 0.05f, 0.04f};

	///Constructor
	KinectSkeletalSubSys::KinectSkeletalSubSys()
	{
		streamerInit();
	}


	///Destructor
	KinectSkeletalSubSys::~KinectSkeletalSubSys()
	{
		streamerUnInit();
	}

	//-------------------------------------------------------------------
	/// Handle new skeleton data
	void KinectSkeletalSubSys::gotSkeletonAlert()
	{
		NUI_SKELETON_FRAME SkeletonFrame = {0};

		m_numSkeletonsFound = 0;
		//static int m_activePlayerID = 255; //Assuming no player will have ID 255
		//int lastSkeletonTracked;
		if ( SUCCEEDED(sensor->NuiSkeletonGetNextFrame( 0, &SkeletonFrame )) ){
	
			for (int i = 0 ; i < NUI_SKELETON_COUNT ; ++i ){
				//m_presentPlayers[i]=false;
				//if(SkeletonFrame.SkeletonData[i].eTrackingState == NUI_SKELETON_TRACKED ||
				//  (SkeletonFrame.SkeletonData[i].eTrackingState == NUI_SKELETON_POSITION_ONLY && m_bAppTracking)){
				if(SkeletonFrame.SkeletonData[i].eTrackingState == NUI_SKELETON_TRACKED){
					//m_presentPlayers[i]=true;
					m_activePlayerID = i;
					++m_numSkeletonsFound;
					//std::cout<<"Skeleton\n";
					break; //StopAfterFirstSkeleton is found
				}
			}
		}
		// no skeletons!
		if( !m_numSkeletonsFound ){ //Change var for PresentPlayers, has more info.
			//std::cout<<"No Skeleton\n"<<std::endl;
			return;
		} 

		HRESULT hr = sensor->NuiTransformSmooth(&SkeletonFrame,&m_verySmoothParams);

		//HRESULT hr = m_pNuiSensor->NuiTransformSmooth(&SkeletonFrame,&m_somewhatLatentParams);

		if ( FAILED(hr) ){
			return;
		}

		*m_pLockedSkeletonBuffer = SkeletonFrame;
		
		static int bufferSwitch = 1; //Remember initial locked buffer is set to index 1
		m_pFreeSkeletonBuffer = &m_skeletonBuffer[bufferSwitch];
		bufferSwitch=(bufferSwitch+1)&1;
		m_pLockedSkeletonBuffer = &m_skeletonBuffer[bufferSwitch];
		

	}

	///Supply the event initialization function to the base class.
	HRESULT KinectSkeletalSubSys::subsystemInit(INuiSensor* kSensor)
	{
		sensor = kSensor;

		HRESULT hr;
		//Skeletal Stream
		//if(m_skeletalStreamEnabled){
		m_hNextSkeletonEvent = CreateEvent( NULL, TRUE, FALSE, NULL );

		//Skeleton Engine Initialization
		if ( HasSkeletalEngine( sensor ) ){
			//initialize Skeleton Tracking
			hr = sensor->NuiSkeletonTrackingEnable(
				m_hNextSkeletonEvent,
				0
				);
			//TODO: Check settings work when near mode is switched
			//hr = m_pNuiSensor->NuiSkeletonTrackingEnable(
			//	m_hNextSkeletonEvent,
			//	NUI_SKELETON_TRACKING_FLAG_ENABLE_IN_NEAR_RANGE |	
			//	NUI_SKELETON_TRACKING_FLAG_ENABLE_SEATED_SUPPORT
			//	);
		}
		//++m_activeNumOfStreams;
		//}
		return hr;
	}

	///Supply additional termination procedures to the base class.
	void KinectSkeletalSubSys::subsystemUnInit()
	{
		if ( m_hNextSkeletonEvent && ( m_hNextSkeletonEvent != INVALID_HANDLE_VALUE ) )
		{
			CloseHandle( m_hNextSkeletonEvent );
			m_hNextSkeletonEvent = NULL;
		}
	}

	///Initialize local variables.
	void KinectSkeletalSubSys::streamerInit()
	{
		//m_skeletalStreamEnabled = skeletalStream;
		//Skeleton Vars
		m_pFreeSkeletonBuffer = &m_skeletonBuffer[0]; //Set the initial Skeleton Buffers to point to
		m_pLockedSkeletonBuffer = &m_skeletonBuffer[1];
		m_newSkeletalDataAvailable = false;
		m_activePlayerID=0;
	}

	///Clear local handles and variables
	void KinectSkeletalSubSys::streamerUnInit()
	{
		m_hNextSkeletonEvent = NULL;
		m_LastSkeletonFoundTime = 0;
	}

	//-------------------------------------------------------------------
	///Read the new skeletal data flag.
	///The flag is reset to false each time it is read so it works as a one-shot trigger.
	///@return True for new data, false if flag has been read before new data arrives.
	bool KinectSkeletalSubSys::newSkeletalData(){
		bool available = m_newSkeletalDataAvailable;
		m_newSkeletalDataAvailable = false;
		return available;
	}

}