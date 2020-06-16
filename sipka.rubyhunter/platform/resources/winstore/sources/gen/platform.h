#ifndef RHFW_PLATFORM_H_
#define RHFW_PLATFORM_H_

#define RHFW_PLATFORM_WINDOWSSTORE

#define WINDOWS_EXECPTION_HELPER_INCLUDE <winstoreplatform/ExceptionHelper.h>

#define PLATFORM_BYTEORDER_HEADER <winstoreplatform/byteorder.h>

#define FILE_SEPARATOR "\\"
#define FILE_SEPARATOR_CHAR '\\'
namespace rhfw {
typedef wchar_t FilePathChar;
} // namespace rhfw

#define ASSETFILEDESCRIPTOR_EXACT_CLASS_INCLUDE <winstoreplatform/asset/WinstoreAssetDescriptor.h>
#define ASSETFILEDESCRIPTOR_EXACT_CLASS_TYPE WinstoreAssetDescriptor
#define MUTEX_EXACT_CLASS_INCLUDE <winstoreplatform/threading/Mutex.h>
#define MUTEX_EXACT_CLASS_TYPE WinstoreMutex
#define SEMAPHORE_EXACT_CLASS_INCLUDE <winstoreplatform/threading/Semaphore.h>
#define SEMAPHORE_EXACT_CLASS_TYPE WinstoreSemaphore
#define STORAGEDIRECTORYDESCRIPTOR_EXACT_CLASS_INCLUDE <winstoreplatform/storage/WinstoreStorageDirectoryDescriptor.h>
#define STORAGEDIRECTORYDESCRIPTOR_EXACT_CLASS_TYPE WinstoreStorageDirectoryDescriptor
#define STORAGEFILEDESCRIPTOR_EXACT_CLASS_INCLUDE <winstoreplatform/storage/WinstoreStorageFileDescriptor.h>
#define STORAGEFILEDESCRIPTOR_EXACT_CLASS_TYPE WinstoreStorageFileDescriptor
#define THREAD_EXACT_CLASS_INCLUDE <winstoreplatform/threading/Thread.h>
#define THREAD_EXACT_CLASS_TYPE WinstoreThread
#define WINDOW_EXACT_CLASS_INCLUDE <winstoreplatform/WinstoreWindow.h>
#define WINDOW_EXACT_CLASS_TYPE WinstoreWindow

#define NETWORKADAPTER_EXACT_CLASS_INCLUDE <winsock/network/WinSockNetworkAdapter.h>
#define NETWORKADAPTER_EXACT_CLASS_TYPE WinSockNetworkAdapter

#define UDPIPV4SOCKET_EXACT_CLASS_INCLUDE <winsock/network/ipv4/udp/WinSockUDPIPv4Socket.h>
#define UDPIPV4SOCKET_EXACT_CLASS_TYPE WinSockUDPIPv4Socket

#define TCPIPV4SOCKET_EXACT_CLASS_INCLUDE <winsock/network/ipv4/tcp/WinSockTCPIPv4Socket.h>
#define TCPIPV4SOCKET_EXACT_CLASS_TYPE WinSockTCPIPv4Socket
#define TCPIPV4CONNECTION_EXACT_CLASS_INCLUDE <winsock/network/ipv4/tcp/WinSockTCPIPv4Connection.h>
#define TCPIPV4CONNECTION_EXACT_CLASS_TYPE WinSockTCPIPv4Connection

#define RANDOMCONTEXT_EXACT_CLASS_INCLUDE <winstoreplatform/random/WinstoreRandomContext.h>
#define RANDOMCONTEXT_EXACT_CLASS_TYPE WinstoreRandomContext

#define SENSORMANAGER_EXACT_CLASS_INCLUDE <winstoreplatform/sensor/WinstoreSensorManager.h>
#define SENSORMANAGER_EXACT_CLASS_TYPE WinstoreSensorManager
#define ATTITUDESENSOR_EXACT_CLASS_INCLUDE <winstoreplatform/sensor/WinstoreAttitudeSensor.h>
#define ATTITUDESENSOR_EXACT_CLASS_TYPE WinstoreAttitudeSensor
#define ACCELEROMETERSENSOR_EXACT_CLASS_INCLUDE <winstoreplatform/sensor/WinstoreAccelerometerSensor.h>
#define ACCELEROMETERSENSOR_EXACT_CLASS_TYPE WinstoreAccelerometerSensor

#define CAMERAMANAGER_EXACT_CLASS_INCLUDE <winstoreplatform/camera/WinstoreCameraManager.h>
#define CAMERAMANAGER_EXACT_CLASS_TYPE WinstoreCameraManager
#define CAMERA_EXACT_CLASS_INCLUDE <winstoreplatform/camera/WinstoreCamera.h>
#define CAMERA_EXACT_CLASS_TYPE WinstoreCamera
#define CAMERAINFO_EXACT_CLASS_INCLUDE <winstoreplatform/camera/WinstoreCameraInfo.h>
#define CAMERAINFO_EXACT_CLASS_TYPE WinstoreCameraInfo
#define CAMERATEXTUREINPUTSOURCE_EXACT_CLASS_INCLUDE <winstoreplatform/camera/WinstoreCameraTextureInputSource.h>
#define CAMERATEXTUREINPUTSOURCE_EXACT_CLASS_TYPE WinstoreCameraTextureInputSource

#endif /* RHFW_PLATFORM_H_ */
