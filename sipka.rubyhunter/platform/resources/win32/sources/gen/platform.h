#ifndef RHFW_PLATFORM_H_
#define RHFW_PLATFORM_H_

#define RHFW_PLATFORM_WIN32

#define WINDOWS_EXECPTION_HELPER_INCLUDE <win32platform/ExceptionHelper.h>

#define PLATFORM_BYTEORDER_HEADER <win32platform/byteorder.h>

#define FILE_SEPARATOR "\\"
#define FILE_SEPARATOR_CHAR '\\'
namespace rhfw {
typedef wchar_t FilePathChar;
} // namespace rhfw

#define ASSETFILEDESCRIPTOR_EXACT_CLASS_INCLUDE <win32platform/asset/Win32AssetDescriptor.h>
#define ASSETFILEDESCRIPTOR_EXACT_CLASS_TYPE Win32AssetDescriptor
#define MUTEX_EXACT_CLASS_INCLUDE <win32platform/threading/Mutex.h>
#define MUTEX_EXACT_CLASS_TYPE Win32Mutex
#define SEMAPHORE_EXACT_CLASS_INCLUDE <win32platform/threading/Semaphore.h>
#define SEMAPHORE_EXACT_CLASS_TYPE Win32Semaphore
#define STORAGEDIRECTORYDESCRIPTOR_EXACT_CLASS_INCLUDE <win32platform/storage/Win32StorageDirectoryDescriptor.h>
#define STORAGEDIRECTORYDESCRIPTOR_EXACT_CLASS_TYPE Win32StorageDirectoryDescriptor
#define STORAGEFILEDESCRIPTOR_EXACT_CLASS_INCLUDE <win32platform/storage/Win32StorageFileDescriptor.h>
#define STORAGEFILEDESCRIPTOR_EXACT_CLASS_TYPE Win32StorageFileDescriptor
#define THREAD_EXACT_CLASS_INCLUDE <win32platform/threading/Thread.h>
#define THREAD_EXACT_CLASS_TYPE Win32Thread
#define WINDOW_EXACT_CLASS_INCLUDE <win32platform/window/Win32Window.h>
#define WINDOW_EXACT_CLASS_TYPE Win32Window

#define NETWORKADAPTER_EXACT_CLASS_INCLUDE <winsock/network/WinSockNetworkAdapter.h>
#define NETWORKADAPTER_EXACT_CLASS_TYPE WinSockNetworkAdapter

#define UDPIPV4SOCKET_EXACT_CLASS_INCLUDE <winsock/network/ipv4/udp/WinSockUDPIPv4Socket.h>
#define UDPIPV4SOCKET_EXACT_CLASS_TYPE WinSockUDPIPv4Socket

#define TCPIPV4SOCKET_EXACT_CLASS_INCLUDE <winsock/network/ipv4/tcp/WinSockTCPIPv4Socket.h>
#define TCPIPV4SOCKET_EXACT_CLASS_TYPE WinSockTCPIPv4Socket
#define TCPIPV4CONNECTION_EXACT_CLASS_INCLUDE <winsock/network/ipv4/tcp/WinSockTCPIPv4Connection.h>
#define TCPIPV4CONNECTION_EXACT_CLASS_TYPE WinSockTCPIPv4Connection

#define RANDOMCONTEXT_EXACT_CLASS_INCLUDE <win32platform/random/Win32RandomContext.h>
#define RANDOMCONTEXT_EXACT_CLASS_TYPE Win32RandomContext

#define SENSORMANAGER_EXACT_CLASS_INCLUDE <win32platform/sensor/Win32SensorManager.h>
#define SENSORMANAGER_EXACT_CLASS_TYPE Win32SensorManager
#define ATTITUDESENSOR_EXACT_CLASS_INCLUDE <win32platform/sensor/Win32AttitudeSensor.h>
#define ATTITUDESENSOR_EXACT_CLASS_TYPE Win32AttitudeSensor
#define ACCELEROMETERSENSOR_EXACT_CLASS_INCLUDE <win32platform/sensor/Win32AccelerometerSensor.h>
#define ACCELEROMETERSENSOR_EXACT_CLASS_TYPE Win32AccelerometerSensor

#define CAMERAMANAGER_EXACT_CLASS_INCLUDE <win32platform/camera/Win32CameraManager.h>
#define CAMERAMANAGER_EXACT_CLASS_TYPE Win32CameraManager
#define CAMERA_EXACT_CLASS_INCLUDE <win32platform/camera/Win32Camera.h>
#define CAMERA_EXACT_CLASS_TYPE Win32Camera
#define CAMERAINFO_EXACT_CLASS_INCLUDE <win32platform/camera/Win32CameraInfo.h>
#define CAMERAINFO_EXACT_CLASS_TYPE Win32CameraInfo
#define CAMERATEXTUREINPUTSOURCE_EXACT_CLASS_INCLUDE <win32platform/camera/Win32CameraTextureInputSource.h>
#define CAMERATEXTUREINPUTSOURCE_EXACT_CLASS_TYPE Win32CameraTextureInputSource

#endif /* RHFW_PLATFORM_H_ */
