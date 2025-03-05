/**
  \file G3D-gfx.lib/source/AudioDevice.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/

#include "G3D-gfx/AudioDevice.h"
#include "G3D-base/stringutils.h"

#if (! defined(G3D_NO_FMOD))

#include "../../fmod/include/fmod.hpp"
#include "../../fmod/include/fmod_errors.h"

#include "G3D-base/FileSystem.h"
#include "G3D-base/Log.h"
#include "G3D-base/Any.h"

namespace G3D {

AudioDevice* AudioDevice::instance = nullptr;

static void ERRCHECK(FMOD_RESULT result) {
    alwaysAssertM(result == FMOD_OK, format("FMOD error! (%d) %s", result, FMOD_ErrorString(result)));
}
    
    
AudioDevice::AudioDevice() : m_enable(false), m_fmodSystem(nullptr) {
    instance = this;
}

    
void AudioDevice::setListener3DAttributes(const CFrame& listenerFrame, const Vector3& listenerVelocity) {
    if (! m_enable) return;

    const Vector3& forward = listenerFrame.lookVector();
    const Vector3& up = listenerFrame.upVector();
    const FMOD_RESULT result = m_fmodSystem->set3DListenerAttributes(0, (FMOD_VECTOR*)&listenerFrame.translation,
        (FMOD_VECTOR*)&listenerVelocity, (FMOD_VECTOR*)&forward, (FMOD_VECTOR*)&up);
    ERRCHECK(result);
}
    

void AudioDevice::init(bool enable, int numVirtualChannels, unsigned int bufferLength, int numBuffers) {
    m_enable = enable;
    if (enable) {
        alwaysAssertM(m_fmodSystem == nullptr, "Already initialized");
        FMOD_RESULT result = FMOD::System_Create(&m_fmodSystem);
        ERRCHECK(result);
            
        unsigned int version;
        result = m_fmodSystem->getVersion(&version);
        ERRCHECK(result);
            
        alwaysAssertM(version >= FMOD_VERSION, format("FMOD lib version %08x doesn't match header version %08x", version, FMOD_VERSION));

        m_fmodSystem->setDSPBufferSize(bufferLength, numBuffers);
            
        void* extradriverdata = nullptr;
        result = m_fmodSystem->init(numVirtualChannels, FMOD_INIT_NORMAL | FMOD_INIT_3D_RIGHTHANDED, extradriverdata);

        m_fmodSystem->set3DNumListeners(1);
        // scale factors: doppler, ditance, rolloff
        m_fmodSystem->set3DSettings(1.0f, 1.0f, 1.0f);

        ERRCHECK(result);
    } else {
        logPrintf("WARNING: AudioDevice is not enabled. Set G3DSpecification::audio = true before invoking initGLG3D() to enable audio.\n");
    }
        
}
    
    
void AudioDevice::cleanup() {
    if (notNull(m_fmodSystem)) {
        m_channelArray.cleanup();
        m_soundArray.cleanup();
            
        FMOD_RESULT result;
        result = m_fmodSystem->close();
        ERRCHECK(result);
        result = m_fmodSystem->release();
        ERRCHECK(result);
        m_fmodSystem = nullptr;
    }
}
    
    
AudioDevice::~AudioDevice() {
    cleanup();
}
    
    
void AudioDevice::update() {
    if (notNull(m_fmodSystem)) {
        const FMOD_RESULT result = m_fmodSystem->update();
        ERRCHECK(result);
    }
}
    
    
////////////////////////////////////////////////////////////////////////
    
void AudioChannel::stop() {
    if (notNull(m_fmodChannel)) {
        m_fmodChannel->stop();
        m_fmodChannel = nullptr;
    }
}


void AudioChannel::setVolume(float v) {
    if (notNull(m_fmodChannel)) m_fmodChannel->setVolume(v);
}
    
    
void AudioChannel::setPan(float p) {
    if (notNull(m_fmodChannel)) m_fmodChannel->setPan(p);
}
    
    
void AudioChannel::setFrequency(float hz) {
    if (notNull(m_fmodChannel)) m_fmodChannel->setFrequency(hz);
}
    

void AudioChannel::set3DAttributes(const Point3& wsPosition, const Vector3& wsVelocity) {
    if (notNull(m_fmodChannel)) m_fmodChannel->set3DAttributes((FMOD_VECTOR*)&wsPosition, (FMOD_VECTOR*)&wsVelocity);
}


void AudioChannel::cleanup() {
    m_fmodChannel = nullptr;
}
    
    
bool AudioChannel::paused() const {
    bool paused = false;
    if (notNull(m_fmodChannel)) m_fmodChannel->getPaused(&paused);
    return paused;
}
    

bool AudioChannel::done() const {
    if (isNull(m_fmodChannel)) {
        return true;
    } else {
        // This may be incorrect. The fmod documentation indicates 
        // that the channel can be reused, although it also says that
        // channels are reference counted and if re-used the handle
        // will be invalid...and we allocate a ridiculous number of channels
        // in this implementation to avoid running out due to simultaneous
        // sounds.
        bool playing = false;
        const FMOD_RESULT result = m_fmodChannel->isPlaying(&playing);
        ERRCHECK(result);
        return ! playing;
    }
}
    
void AudioChannel::setPaused(bool p) {
    if (notNull(m_fmodChannel)) {
        const FMOD_RESULT result = m_fmodChannel->setPaused(p);
        ERRCHECK(result);
    }
}
    
////////////////////////////////////////////////////////////////////////
    
    
Sound::Sound() : m_fmodSound(nullptr) {
}
   

Any Sound::toAny() const {
    Any a(Any::TABLE, "Sound");
    
    a["filename"] = m_filename;
    
    if (! m_positional) {
        a["positional"] = m_positional;
    }

    if (m_loop) {
        a["loop"] = m_loop;
    }

    return a;
}


shared_ptr<AudioChannel> Sound::play(const Point3& initialPosition, const Vector3& initialVelocity, float initialVolume, float initialFrequency, bool startPaused) {
    const shared_ptr<AudioChannel>& c = play(initialVolume, 0.0, initialFrequency, startPaused);
    if (notNull(c)) {
        c->set3DAttributes(initialPosition, initialVelocity);
    }
    return c;
}


shared_ptr<AudioChannel> Sound::play(float initialVolume, float initialPan, float initialFrequency, bool startPaused) {
    if (isNull(m_fmodSound)) { return nullptr; }
        
    static const bool START_PAUSED = true;
    FMOD::Channel* channel = nullptr;
    FMOD_RESULT result = AudioDevice::instance->m_fmodSystem->playSound(m_fmodSound, nullptr, START_PAUSED, &channel);
    ERRCHECK(result);
        
    const shared_ptr<AudioChannel>& ch = createShared<AudioChannel>(channel);
    ch->setVolume(initialVolume);
    ch->setPan(initialPan);
    if (initialFrequency > 0) { ch->setFrequency(initialFrequency); }
        
    ch->setPaused(startPaused);
        
    return AudioDevice::instance->m_channelArray.remember(ch);
}


shared_ptr<Sound> Sound::create(const Any& a) {
    AnyTableReader t(a);

    String filename;   
    bool positional = true;
    bool loop = false;

    t.getFilenameIfPresent("filename", filename);
    t.getIfPresent("loop", loop);
    t.getIfPresent("positional", positional);
    t.verifyDone();

    return create(filename, loop, positional);
}


shared_ptr<Sound> Sound::create(const String& filename, bool loop, bool positional) {
    alwaysAssertM(notNull(AudioDevice::instance), "AudioDevice not initialized");
    const shared_ptr<Sound>& s = createShared<Sound>();

    s->m_loop = loop;
    s->m_filename = filename;
    s->m_positional = positional;

    s->m_name = FilePath::baseExt(filename);
        
    FMOD_MODE mode = FMOD_DEFAULT;
        
    if (loop) {
        mode |= FMOD_LOOP_NORMAL;
    }

    if (positional) {
        mode |= FMOD_3D;
    }

        
    if (! FileSystem::exists(filename)) {
        throw filename + " not found in Sound::create";
    } else {
        FileSystem::markFileUsed(filename);
    }

    /*
    if ((FileSystem::size(filename) > 1e5) && endsWith(toLower(filename), ".mp3")) {
        // Force large files to load in streaming mode to save memory
        mode |= FMOD_CREATESTREAM;
    }
    */

    if (AudioDevice::instance->enabled()) {
        const FMOD_RESULT result = AudioDevice::instance->m_fmodSystem->createSound(filename.c_str(), mode, 0, &s->m_fmodSound);
        ERRCHECK(result);
    }
        
    return AudioDevice::instance->m_soundArray.remember(s);
}
    
    
Sound::~Sound() {
    cleanup();
}
    
    
void Sound::cleanup() {
    if (m_fmodSound) {
        m_fmodSound->release();
        m_fmodSound = nullptr;
    }
}

} // namespace G3D

#endif
