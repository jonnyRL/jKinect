#include "KinectSkeletalStreamer.h"

namespace jrl
{
	KinectSkeletalStreamer::KinectSkeletalStreamer()
	{
	}

	KinectSkeletalStreamer::~KinectSkeletalStreamer()
	{
	}

	HRESULT KinectSkeletalStreamer::subsystemInit()
	{
		HRESULT hr;
		//DWORD nuiFlags = NUI_INITIALIZE_FLAG_USES_DEPTH_AND_PLAYER_INDEX | NUI_INITIALIZE_FLAG_USES_SKELETON |  NUI_INITIALIZE_FLAG_USES_COLOR;
		//hr = m_pNuiSensor->NuiInitialize( nuiFlags );
		//DWORD nuiFlags = NUI_INITIALIZE_FLAG_USES_DEPTH_AND_PLAYER_INDEX  |  NUI_INITIALIZE_FLAG_USES_SKELETON;
		DWORD nuiFlags = NUI_INITIALIZE_FLAG_USES_SKELETON;
		hr = m_pNuiSensor->NuiInitialize( nuiFlags );

		if ( FAILED( hr ) ){
			if ( E_NUI_DEVICE_IN_USE == hr ){
				///@ todo Add message: Device in use.
			}else{
			}
			return hr;
		}

		//hr = depthSubSys.subsystemInit(m_pNuiSensor);
		hr = skeletalSubSys.subsystemInit(m_pNuiSensor);
		
		return hr;
	}

	void KinectSkeletalStreamer::subsystemUnInit()
	{
		skeletalSubSys.subsystemUnInit();
	}
	//-------------------------------------------------------------------
	///Thread function
	///@return The color-only thread function.
	//DWORD WINAPI KinectSkeletalStreamer::subsystemThreadFunction()
	//{
	//	const int numEvents = 3;
	//	HANDLE hEvents[numEvents] = { m_hEvNuiProcessStop, depthSubSys.getEventHandle(), skeletalSubSys.getEventHandle()};
	//	int    nEventIdx;

	//	// Main thread loop
	//	bool continueProcessing = true;
	//	while ( continueProcessing ){
	//		// Wait for any of the events to be signalled
	//		nEventIdx = WaitForMultipleObjects( numEvents, hEvents, FALSE, 100 );
	//		switch ( nEventIdx ){
	//		case WAIT_TIMEOUT:
	//			continue;
	//			// If the stop event, stop looping and exit
	//		case WAIT_OBJECT_0:
	//			continueProcessing = false;
	//			continue;

	//		case WAIT_OBJECT_0 + 1:
	//			depthSubSys.gotDepthAlert();
	//			break;

	//		case WAIT_OBJECT_0 + 2:
	//			skeletalSubSys.gotSkeletonAlert();
	//			break;
	//		}
	//	}
	//	return 0;
	//}

	DWORD WINAPI KinectSkeletalStreamer::subsystemThreadFunction()
	{
		const int numEvents = 2;
		HANDLE hEvents[numEvents] = { m_hEvNuiProcessStop, skeletalSubSys.getEventHandle()};
		int    nEventIdx;

		// Main thread loop
		bool continueProcessing = true;
		while ( continueProcessing ){
			// Wait for any of the events to be signalled
			nEventIdx = WaitForMultipleObjects( numEvents, hEvents, FALSE, 100 );
			switch ( nEventIdx ){
			case WAIT_TIMEOUT:
				continue;
				// If the stop event, stop looping and exit
			case WAIT_OBJECT_0:
				continueProcessing = false;
				continue;

			case WAIT_OBJECT_0 + 1:
				skeletalSubSys.gotSkeletonAlert();
				break;
			}
		}
		return 0;
	}

}