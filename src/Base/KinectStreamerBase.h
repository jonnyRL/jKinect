// Declares of KinectStreamerBase class

#ifndef KINECTSTREAMERBASE_H
#define KINECTSTREAMERBASE_H


#define WIN32_LEAN_AND_MEAN
#define INC_OLE2

#include <windows.h>

//Many errors appear when excluding ObjBase.h
#include <ObjBase.h>

#include "NuiApi.h"

namespace jrl
{
	class KinectStreamerBase
	{

	public:

		//Variables

		//HWND        m_hWnd;
		HINSTANCE   m_hInstance;

		///Sets the tilt of the device. This funciton
		///must be called during initialization and never during operation.
		///@param[in] angle The desired tilt angle for the device.
		inline void setSetpointAngle(LONG angle){
			m_kinectSetpointAngle = angle;
		}

		///Reads the device angle.
		inline const LONG getSetpointAngle() const {
			return m_kinectSetpointAngle;
		}

		KinectStreamerBase();
		virtual ~KinectStreamerBase();

		HRESULT init(int deviceIndex = 0); //Nui_Init
		void unInit(); //Nui_uninit

		///@todo Implement error code printing
		//std::ostringstream printResult(HRESULT result);

		OLECHAR* getDeviceConnectionID();
		OLECHAR* getDeviceUniqueID();

		void printConnectionID();

		void setDeviceStatusChangeCallback(NuiStatusProc callback, void* pUserData);

		static HRESULT getSensorCount(int& sensorCount);

		static char* sensorStatusToString(HRESULT sensorStatus);

	protected:
		//Setpoint angle
		LONG m_kinectSetpointAngle;

		//Kinect Status
		int m_activeNumOfStreams;

		LONG m_deviceAngle;

		// Current kinect
		INuiSensor* m_pNuiSensor;
		BSTR m_instanceId;

		HANDLE m_hThNuiProcess;
		HANDLE m_hEvNuiProcessStop;

		virtual HRESULT subsystemInit()=0;
		virtual void subsystemUnInit()=0;

		void Nui_Zero();

		
		LONG getDeviceAngle();
		void setDeviceAngle(LONG angle);

		static DWORD WINAPI Nui_ProcessThread(LPVOID pParam);
		virtual DWORD WINAPI subsystemThreadFunction() = 0;

	};
}


#endif