#ifndef RHFW_PLATFORM_H_
#define RHFW_PLATFORM_H_

#define RHFW_PLATFORM_LINUX

#define OPENAL_HEADER_FILE <gen/OpenALCommonHeader.h>

#define PLATFORM_BYTEORDER_HEADER <linuxplatform/byteorder.h>

#define FILE_SEPARATOR "/"
#define FILE_SEPARATOR_CHAR '/'
namespace rhfw {
typedef char FilePathChar;
} // namespace rhfw

#define ASSETFILEDESCRIPTOR_EXACT_CLASS_INCLUDE <linuxplatform/asset/LinuxAssetDescriptor.h>
#define ASSETFILEDESCRIPTOR_EXACT_CLASS_TYPE LinuxAssetDescriptor
#define MUTEX_EXACT_CLASS_INCLUDE <linuxplatform/threading/Mutex.h>
#define MUTEX_EXACT_CLASS_TYPE LinuxMutex
#define SEMAPHORE_EXACT_CLASS_INCLUDE <linuxplatform/threading/Semaphore.h>
#define SEMAPHORE_EXACT_CLASS_TYPE LinuxSemaphore
#define STORAGEDIRECTORYDESCRIPTOR_EXACT_CLASS_INCLUDE <linuxplatform/storage/LinuxStorageDirectoryDescriptor.h>
#define STORAGEDIRECTORYDESCRIPTOR_EXACT_CLASS_TYPE LinuxStorageDirectoryDescriptor
#define STORAGEFILEDESCRIPTOR_EXACT_CLASS_INCLUDE <linuxplatform/storage/LinuxStorageFileDescriptor.h>
#define STORAGEFILEDESCRIPTOR_EXACT_CLASS_TYPE LinuxStorageFileDescriptor
#define THREAD_EXACT_CLASS_INCLUDE <linuxplatform/threading/Thread.h>
#define THREAD_EXACT_CLASS_TYPE LinuxThread
#define WINDOW_EXACT_CLASS_INCLUDE <linuxplatform/window/LinuxWindow.h>
#define WINDOW_EXACT_CLASS_TYPE LinuxWindow

#define NETWORKADAPTER_EXACT_CLASS_INCLUDE <linuxplatform/network/LinuxNetworkAdapter.h>
#define NETWORKADAPTER_EXACT_CLASS_TYPE LinuxNetworkAdapter

#define UDPIPV4SOCKET_EXACT_CLASS_INCLUDE <linuxplatform/network/ipv4/udp/LinuxUDPIPv4Socket.h>
#define UDPIPV4SOCKET_EXACT_CLASS_TYPE LinuxUDPIPv4Socket

#define TCPIPV4SOCKET_EXACT_CLASS_INCLUDE <linuxplatform/network/ipv4/tcp/LinuxTCPIPv4Socket.h>
#define TCPIPV4SOCKET_EXACT_CLASS_TYPE LinuxTCPIPv4Socket
#define TCPIPV4CONNECTION_EXACT_CLASS_INCLUDE <linuxplatform/network/ipv4/tcp/LinuxTCPIPv4Connection.h>
#define TCPIPV4CONNECTION_EXACT_CLASS_TYPE LinuxTCPIPv4Connection

#define RANDOMCONTEXT_EXACT_CLASS_INCLUDE <linuxplatform/random/LinuxRandomContext.h>
#define RANDOMCONTEXT_EXACT_CLASS_TYPE LinuxRandomContext

#define SENSORMANAGER_EXACT_CLASS_INCLUDE <linuxplatform/sensor/LinuxSensorManager.h>
#define SENSORMANAGER_EXACT_CLASS_TYPE LinuxSensorManager
#define ATTITUDESENSOR_EXACT_CLASS_INCLUDE <linuxplatform/sensor/LinuxAttitudeSensor.h>
#define ATTITUDESENSOR_EXACT_CLASS_TYPE LinuxAttitudeSensor
#define ACCELEROMETERSENSOR_EXACT_CLASS_INCLUDE <linuxplatform/sensor/LinuxAccelerometerSensor.h>
#define ACCELEROMETERSENSOR_EXACT_CLASS_TYPE LinuxAccelerometerSensor

#define CAMERAMANAGER_EXACT_CLASS_INCLUDE <linuxplatform/camera/LinuxCameraManager.h>
#define CAMERAMANAGER_EXACT_CLASS_TYPE LinuxCameraManager
#define CAMERA_EXACT_CLASS_INCLUDE <linuxplatform/camera/LinuxCamera.h>
#define CAMERA_EXACT_CLASS_TYPE LinuxCamera
#define CAMERAINFO_EXACT_CLASS_INCLUDE <linuxplatform/camera/LinuxCameraInfo.h>
#define CAMERAINFO_EXACT_CLASS_TYPE LinuxCameraInfo
#define CAMERATEXTUREINPUTSOURCE_EXACT_CLASS_INCLUDE <linuxplatform/camera/LinuxCameraTextureInputSource.h>
#define CAMERATEXTUREINPUTSOURCE_EXACT_CLASS_TYPE LinuxCameraTextureInputSource

#endif /* RHFW_PLATFORM_H_ */
