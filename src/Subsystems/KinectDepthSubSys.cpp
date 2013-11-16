/**
\class jrl::KinectDepthSubSys

A class that includes functions to access the Kinect's depth subsystem.

*/

#include "KinectDepthSubSys.h"
#include <iostream>

namespace jrl
{

	//lookups for color tinting based on player index
	const int KinectDepthSubSys::m_IntensityShiftByPlayerR[] = { 1, 2, 0, 2, 0, 0, 2, 0 };
	const int KinectDepthSubSys::m_IntensityShiftByPlayerG[] = { 1, 2, 2, 0, 2, 0, 0, 1 };
	const int KinectDepthSubSys::m_IntensityShiftByPlayerB[] = { 1, 0, 2, 2, 0, 2, 0, 2 };

	///The default constructor.
	KinectDepthSubSys::KinectDepthSubSys(){
		setResolution(KINECT_DEPTH_RESOLUTION_640x480);
		streamerInit();
	}

	///The default destructor.
	KinectDepthSubSys::~KinectDepthSubSys()
	{
		streamerUnInit();
	}

	///Sets the resolution of the depth image to be used.
	///@param[in] res The selected resolution.
	void KinectDepthSubSys::setResolution(KINECT_DEPTH_RESOLUTION res){
		switch(res){
		case KINECT_DEPTH_RESOLUTION_80x60:
			depthResolution = NUI_IMAGE_RESOLUTION_80x60;
			frameWidth = 80;
			frameHeight = 60;
			break;
		case KINECT_DEPTH_RESOLUTION_320x240:
			depthResolution = NUI_IMAGE_RESOLUTION_320x240;
			frameWidth = 320;
			frameHeight = 240;
			break;
		case KINECT_DEPTH_RESOLUTION_640x480:
			depthResolution = NUI_IMAGE_RESOLUTION_640x480;
			frameWidth = 640;
			frameHeight = 480;
			break;
		default:
			depthResolution = NUI_IMAGE_RESOLUTION_INVALID;
			break;
		}
		bytesPerPixel = 2;
		widthByHeight = frameWidth * frameHeight;
		totalDepthBytes = widthByHeight * bytesPerPixel;
		m_pLockedRawDepthBuffer = new USHORT[widthByHeight];
	}

	//-------------------------------------------------------------------
	///Activates or deactivates near mode.
	///@note Only use this function during sytem initialization.
	///@param[in] mode True for near mode, false for far mode. 
	void KinectDepthSubSys::setNearMode(bool mode){
		updateDepthStreamFlag(NUI_IMAGE_STREAM_FLAG_ENABLE_NEAR_MODE, mode);
	}

	///Initializes the class variables.
	void KinectDepthSubSys::streamerInit()
	{
		//Init mutex
		//m_depthStreamEnabled = depthStream;
		//m_rawDepthStreamEnabled = rawDepthStream;

		//m_nearThreshold = 400;
		//m_farThreshold=2000;

		//Raw Depth Vars

		m_pFreeRawDepthBuffer = &m_depthOutputBuffer[0][0];
	
		//m_pLockedRawDepthBuffer = &m_depthOutputBuffer[1][0];
		

		//Image Depth Vars
		//m_pFreeDepthBuffer = &m_depthBuffer[0][0];
		//m_pLockedDepthBuffer = &m_depthBuffer[1][0];
		m_newDepthDataAvailable = false;

	}

	///Clears the class variables.
	void KinectDepthSubSys::streamerUnInit()
	{
		m_hNextDepthFrameEvent = NULL;
		m_pDepthStreamHandle = NULL;
		m_DepthFramesTotal = 0;
		m_LastDepthFPStime = 0;
		m_LastDepthFramesTotal = 0;
		//m_pDrawDepth = NULL;
		//m_pDrawColor = NULL;
		//ZeroMemory(m_SkeletonIds,sizeof(m_SkeletonIds));
		//ZeroMemory(m_TrackedSkeletonIds,sizeof(m_SkeletonIds));
		m_DepthStreamFlags = 0;

		delete[] m_pLockedRawDepthBuffer;
	}

	//-------------------------------------------------------------------
	///Switches the depth buffers.
	///@note Function is not being currently used
	void KinectDepthSubSys::switchDepthBuffers(){
		//Remember initial locked buffer is set to index 1
		static int rawBufferSwitch = 1; 
		m_pFreeRawDepthBuffer = &m_depthOutputBuffer[rawBufferSwitch][0];
		m_newDepthDataAvailable = true;
		rawBufferSwitch=(rawBufferSwitch+1)&1;
		m_pLockedRawDepthBuffer = &m_depthOutputBuffer[rawBufferSwitch][0];
	}

	//-------------------------------------------------------------------
	///Reads the "new depth data" flag.
	///The flag is reset to false each time it is read so it works as a one-shot trigger.
	///@return True for new data, false if flag has been read before new data arrives.
	bool KinectDepthSubSys::newDepthDataAvailable(){
		bool available = m_newDepthDataAvailable;
		m_newDepthDataAvailable = false;
		return available;
	}

	///Transforms a pixel in depth space to coordinates in skeleton space
	///@todo Parallelize function. Library source code is available.
	///@param[in] depthX The x index of the pixel.
	///@param[in] depthY The y index of the pixel.
	///@param[in] depthVal The depth value of the pixel as obtained from the depth buffer (shifted 3 bits left).
	///@return A vector with the x, y and z coordinates in skeleton space (meters).
	Vector4 KinectDepthSubSys::depthSpaceToSkelSpace(LONG depthX, LONG depthY, USHORT depthVal)
	{
		return NuiTransformDepthImageToSkeleton(depthX, depthY, depthVal, depthResolution);
	}

	//-------------------------------------------------------------------
	///Updates a value of the stream flags.
	///@param[in] flag The flag to update.
	///@param[in] value The value of the flag to update.
	void KinectDepthSubSys::updateDepthStreamFlag(DWORD flag, bool value )
	{
		DWORD newFlags = m_DepthStreamFlags;

		if (value){
			newFlags |= flag;
		}else{
			newFlags &= ~flag;
		}

		if (NULL != sensor && newFlags != m_DepthStreamFlags){
			m_DepthStreamFlags = newFlags;
			sensor->NuiImageStreamSetImageFrameFlags( m_pDepthStreamHandle, m_DepthStreamFlags );
		}
	}

	///Initializes the color subsystem.
	///@param[in] A pointer to the sensor.
	///@return The status of the operation. A value greater than zero if "SUCCEEDED" other value if "FAILED".
	HRESULT KinectDepthSubSys::subsystemInit(INuiSensor* kSensor)
	{
		//frameWidth = KSR::KINECT_WIDTH_H;
		//frameHeight = KSR::KINECT_HEIGHT_H;
		//widthByHeight = frameWidth * frameHeight;

		sensor=kSensor;
		//Depth Stream
		HRESULT hr;
		m_DepthStreamFlags = NUI_IMAGE_STREAM_FLAG_DISTINCT_OVERFLOW_DEPTH_VALUES;
	//	if(m_depthStreamEnabled){
		m_hNextDepthFrameEvent = CreateEvent( NULL, TRUE, FALSE, NULL );

		hr = sensor->NuiImageStreamOpen( //Near Mode Enabling Settings
			HasSkeletalEngine(sensor) ? NUI_IMAGE_TYPE_DEPTH_AND_PLAYER_INDEX : NUI_IMAGE_TYPE_DEPTH,
			/*NUI_IMAGE_RESOLUTION_320x240,*/
			depthResolution,
			m_DepthStreamFlags,
			2,
			m_hNextDepthFrameEvent,
			&m_pDepthStreamHandle );
		
			//++m_activeNumOfStreams;
	//	}
		return hr;
	}

	///Frees the resources used by the subsystem.
	void KinectDepthSubSys::subsystemUnInit()
	{
		if ( m_hNextDepthFrameEvent && ( m_hNextDepthFrameEvent != INVALID_HANDLE_VALUE ) )
		{
			CloseHandle( m_hNextDepthFrameEvent );
			m_hNextDepthFrameEvent = NULL;
		}
	}

	//-------------------------------------------------------------------
	/// Handles new depth data.
	void KinectDepthSubSys::gotDepthAlert()
	{
		
		//raii_kinImageResources dRes(m_pNuiSensor, m_pDepthStreamHandle);
		//if ( FAILED(dRes.hr) )
		//{
		//	return;
		//}
		//raii_pINuiFrameTexture.pTexture->LockRect( 0, &LockedRect, NULL, 0 );
		//if ( 0 != dRes.LockedRect.Pitch ){


		NUI_IMAGE_FRAME imageFrame;

		HRESULT hr = sensor->NuiImageStreamGetNextFrame(
			m_pDepthStreamHandle,
			0,
			&imageFrame );
		
		if ( FAILED( hr ) )
		{
			return;
		}

		INuiFrameTexture * pTexture = imageFrame.pFrameTexture;
		NUI_LOCKED_RECT LockedRect;
		pTexture->LockRect( 0, &LockedRect, NULL, 0 );
		if ( 0 != LockedRect.Pitch ){
			//lastDroppedDepthFrameAvailable = false;
			//---DWORD frameWidth, frameHeight;
			//NuiImageResolutionToSize( dRes.imageFrame.eResolution, frameWidth, frameHeight );		
			//---NuiImageResolutionToSize( imageFrame.eResolution, frameWidth, frameHeight );

			// end pixel is start + width*height - 1
			//assert( frameWidth * frameHeight <= ARRAYSIZE(m_rgbWk) );

			////Transfer Raw Data to Buffer
			//USHORT* pBufferRun = reinterpret_cast<USHORT*>(dRes.LockedRect.pBits);


			//---USHORT * pBufferRun = (USHORT *)LockedRect.pBits;
			//---USHORT* pBufferEnd = pBufferRun + (frameWidth * frameHeight);
			//---USHORT* targetBuffer = m_pLockedRawDepthBuffer; 
			memcpy(m_pLockedRawDepthBuffer, LockedRect.pBits, widthByHeight*sizeof(USHORT));
			//--targetBuffer;
			//--pBufferRun;
			//while(pBufferRun<pBufferEnd){
			//	//Get image data and discard last three bits to keep only depth data
			//	*(++targetBuffer) = (*(++pBufferRun))>>3;
			//	//*(++targetBuffer) = (*(++pBufferRun))>>3;
			//}		
			
		}else{
			OutputDebugString( L"Buffer length of received texture is bogus\r\n" );
		}
		pTexture->UnlockRect( 0 );
		m_newDepthDataAvailable = true;
		sensor->NuiImageStreamReleaseFrame( m_pDepthStreamHandle, &imageFrame );
	}

	//int* createDepthFrame_depthOnly();

	//-------------------------------------------------------------------
	///Transform 13-bit depth information into an 8-bit intensity appropriate
	///for display.
	///@param[in] s The pixel data to transform.
	///@param[out] target The placeholder for the transformed pixel. 
	void KinectDepthSubSys::Nui_ShortToQuad_Depth( USHORT s, unsigned char *target )
	{
		USHORT RealDepth = NuiDepthPixelToDepth(s);
		USHORT Player    = NuiDepthPixelToPlayerIndex(s);

		// transform 13-bit depth information into an 8-bit intensity appropriate
		// for display (we disregard information in most significant bit)
		//BYTE intensity = (BYTE)~(RealDepth >> 4);

		RealDepth = (RealDepth>>1)&255;
		BYTE intensity = BYTE(RealDepth);
		// tint the intensity by dividing by per-player values
		*target = intensity >> m_IntensityShiftByPlayerR[Player];
		*(++target) = intensity >> m_IntensityShiftByPlayerG[Player];
		*(++target) = intensity >> m_IntensityShiftByPlayerB[Player];
		*(++target)|=255;
	}




}