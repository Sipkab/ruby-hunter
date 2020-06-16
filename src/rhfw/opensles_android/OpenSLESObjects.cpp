/*
 * Copyright (C) 2020 Bence Sipka
 *
 * This program is free software: you can redistribute it and/or modify 
 * it under the terms of the GNU General Public License as published by 
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
/*
 * OpenSLESObjects.cpp
 *
 *  Created on: 2016. marc. 4.
 *      Author: sipka
 */

#include <opensles_android/OpenSLESObjects.h>
#include <SLES/OpenSLES_Android.h>

namespace rhfw {
namespace audio {

#define DECLARE_CONVERSION(iid, type) \
	template<> OpenSLESObject::operator type() { \
		SLresult result;\
		type convres = nullptr; \
		result = (*object)->GetInterface(object, iid, &convres);\
		WARN_OPENSLES_ERROR(); \
		return convres;\
	}

DECLARE_CONVERSION(SL_IID_OBJECT, SLObjectItf)
DECLARE_CONVERSION(SL_IID_AUDIOIODEVICECAPABILITIES, SLAudioIODeviceCapabilitiesItf)
DECLARE_CONVERSION(SL_IID_LED, SLLEDArrayItf)
DECLARE_CONVERSION(SL_IID_VIBRA, SLVibraItf)
DECLARE_CONVERSION(SL_IID_METADATAEXTRACTION, SLMetadataExtractionItf)
DECLARE_CONVERSION(SL_IID_METADATATRAVERSAL, SLMetadataTraversalItf)
DECLARE_CONVERSION(SL_IID_DYNAMICSOURCE, SLDynamicSourceItf)
DECLARE_CONVERSION(SL_IID_OUTPUTMIX, SLOutputMixItf)
DECLARE_CONVERSION(SL_IID_PLAY, SLPlayItf)
DECLARE_CONVERSION(SL_IID_PREFETCHSTATUS, SLPrefetchStatusItf)
DECLARE_CONVERSION(SL_IID_PLAYBACKRATE, SLPlaybackRateItf)
DECLARE_CONVERSION(SL_IID_SEEK, SLSeekItf)
DECLARE_CONVERSION(SL_IID_RECORD, SLRecordItf)
DECLARE_CONVERSION(SL_IID_EQUALIZER, SLEqualizerItf)
DECLARE_CONVERSION(SL_IID_VOLUME, SLVolumeItf)
DECLARE_CONVERSION(SL_IID_DEVICEVOLUME, SLDeviceVolumeItf)
DECLARE_CONVERSION(SL_IID_BUFFERQUEUE, SLBufferQueueItf)
DECLARE_CONVERSION(SL_IID_PRESETREVERB, SLPresetReverbItf)
DECLARE_CONVERSION(SL_IID_ENVIRONMENTALREVERB, SLEnvironmentalReverbItf)
DECLARE_CONVERSION(SL_IID_EFFECTSEND, SLEffectSendItf)
DECLARE_CONVERSION(SL_IID_3DGROUPING, SL3DGroupingItf)
DECLARE_CONVERSION(SL_IID_3DCOMMIT, SL3DCommitItf)
DECLARE_CONVERSION(SL_IID_3DLOCATION, SL3DLocationItf)
DECLARE_CONVERSION(SL_IID_3DDOPPLER, SL3DDopplerItf)
DECLARE_CONVERSION(SL_IID_3DSOURCE, SL3DSourceItf)
DECLARE_CONVERSION(SL_IID_3DMACROSCOPIC, SL3DMacroscopicItf)
DECLARE_CONVERSION(SL_IID_MUTESOLO, SLMuteSoloItf)
DECLARE_CONVERSION(SL_IID_DYNAMICINTERFACEMANAGEMENT, SLDynamicInterfaceManagementItf)
DECLARE_CONVERSION(SL_IID_MIDIMESSAGE, SLMIDIMessageItf)
DECLARE_CONVERSION(SL_IID_MIDIMUTESOLO, SLMIDIMuteSoloItf)
DECLARE_CONVERSION(SL_IID_MIDITEMPO, SLMIDITempoItf)
DECLARE_CONVERSION(SL_IID_MIDITIME, SLMIDITimeItf)
DECLARE_CONVERSION(SL_IID_AUDIODECODERCAPABILITIES, SLAudioDecoderCapabilitiesItf)
DECLARE_CONVERSION(SL_IID_AUDIOENCODERCAPABILITIES, SLAudioEncoderCapabilitiesItf)
DECLARE_CONVERSION(SL_IID_AUDIOENCODER, SLAudioEncoderItf)
DECLARE_CONVERSION(SL_IID_BASSBOOST, SLBassBoostItf)
DECLARE_CONVERSION(SL_IID_PITCH, SLPitchItf)
DECLARE_CONVERSION(SL_IID_RATEPITCH, SLRatePitchItf)
DECLARE_CONVERSION(SL_IID_VIRTUALIZER, SLVirtualizerItf)
DECLARE_CONVERSION(SL_IID_VISUALIZATION, SLVisualizationItf)
DECLARE_CONVERSION(SL_IID_ENGINE, SLEngineItf)
DECLARE_CONVERSION(SL_IID_ENGINECAPABILITIES, SLEngineCapabilitiesItf)
DECLARE_CONVERSION(SL_IID_THREADSYNC, SLThreadSyncItf)

DECLARE_CONVERSION(SL_IID_ANDROIDEFFECT, SLAndroidEffectItf)
DECLARE_CONVERSION(SL_IID_ANDROIDEFFECTSEND, SLAndroidEffectSendItf)
DECLARE_CONVERSION(SL_IID_ANDROIDEFFECTCAPABILITIES, SLAndroidEffectCapabilitiesItf)
DECLARE_CONVERSION(SL_IID_ANDROIDCONFIGURATION, SLAndroidConfigurationItf)
DECLARE_CONVERSION(SL_IID_ANDROIDSIMPLEBUFFERQUEUE, SLAndroidSimpleBufferQueueItf)

SLresult OpenSLESObject::realize() {
	ASSERT(object != nullptr) << "Object to realize is nullptr";
	SLresult result;
	result = (*object)->Realize(object, SL_BOOLEAN_FALSE);
	return result;
}

}  // namespace audio
}  // namespace rhfw
