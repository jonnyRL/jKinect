#include "KinectColorStreamer.h"

namespace jrl
{
	KinectColorStreamer::KinectColorStreamer()
	{
	}

	KinectColorStreamer::~KinectColorStreamer()
	{
	}

	
	///Base class initialization extension.
	HRESULT KinectColorStreamer::subsystemInit()
	{
		HRESULT hr;

		DWORD nuiFlags = NUI_INITIALIZE_FLAG_USES_COLOR;
		hr = m_pNuiSensor->NuiInitialize( nuiFlags );

		if ( FAILED( hr ) ){
			if ( E_NUI_DEVICE_IN_USE == hr ){

			}else{
			}
			return hr;
		}

		hr = colorSubSys.subsystemInit(m_pNuiSensor);
		return hr;
	}

	///Base class uninitialization extension.
	void KinectColorStreamer::subsystemUnInit()
	{
		colorSubSys.subsystemUnInit();
	}


	//-------------------------------------------------------------------
	///Thread function
	///@return The color-only thread function.
	DWORD WINAPI KinectColorStreamer::subsystemThreadFunction()
	{
		const int numEvents = 2;
		HANDLE hEvents[numEvents] = { m_hEvNuiProcessStop, colorSubSys.getEventHandle() };
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
				colorSubSys.gotColorAlert();
				break;
			}
		}
		return 0;
	}
}