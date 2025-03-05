/**
  \file G3D-gfx.lib/include/G3D-gfx/AudioDevice.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#pragma once
#define G3D_gfx_AudioDevice_h

#include "G3D-base/platform.h"
#include "G3D-base/Array.h"
#include "G3D-base/G3DString.h"
#include "G3D-base/Vector3.h"
#include "G3D-base/CoordinateFrame.h"

#ifndef G3D_NO_FMOD

// Forward declarations from FMOD's namespace needed for defining the
// AudioDevice classes. This avoids exposing FMOD to the programmer
// directly.
namespace FMOD {
    class System;
    class Sound;
    class Channel;
};

namespace G3D {

class Sound;
class AudioDevice;
class AudioChannel;
class Any;

/** A playing Sound.
    
    \sa G3D::Sound::play, G3D::SoundEntity, G3D::Entity::playSound   
*/
class AudioChannel {
protected:
    friend class AudioDevice;
    friend class Sound;

    FMOD::Channel*  m_fmodChannel;

    AudioChannel(FMOD::Channel* f) : m_fmodChannel(f) {}

    /** Delete resources */
    void cleanup();

public:

    /** Return true if the sound has finished playing and the channel is no longer needed. */
    bool done() const;

    bool paused() const;

    /** Stop the sound currently playing on this channel */
    void stop();

    void setPaused(bool paused);

    /** \param v on [0, 1] */
    void setVolume(float v);

    /** \param p -1.0 = left, 0.0 = center, 1.0 = right */
    void setPan(float p);

    /** Playback frequency in Hz */
    void setFrequency(float hz);

    /** In world space. The velocity is used for dopler; the sounds will not automatically
        move on their own but must be set each main loop iteration.*/
    void set3DAttributes(const Point3& wsPosition, const Vector3& wsVelocity);
};



/** 
 \brief Initializes the audio system.

 G3D automatically initializes and cleans up AudioDevice and invokes AudioDevice::update from
 RenderDevice::swapBuffers, so 
 this class is rarely accessed by programs explicitly.

 AudioDevice does not depend on the graphics API at present. However, it is in the gfx
 layer of G3D instead of the base layer because future implementations may use the 
 GPU for 3D sound simulation.
*/
class AudioDevice {
protected:
    friend class Sound;
    friend class AudioChannel;

    /** \brief Append-only dynamic array of weak pointers for objects to be shut down on AudioDevice::cleanup() */
    template<class T>
    class WeakCleanupArray {
    private:

        Array<weak_ptr<T> >         m_array;
        int                         m_rememberCallsSinceLastCheck;
    public:
        WeakCleanupArray() : m_rememberCallsSinceLastCheck(-2) {}

        shared_ptr<T> remember(const shared_ptr<T>& r) {
            ++m_rememberCallsSinceLastCheck;

            // Amortized O(1) cleanup of old pointer values 
            if (m_rememberCallsSinceLastCheck > m_array.length()) {
                for (int i = 0; i < m_array.length(); ++i) {
                    if (isNull(m_array[i].lock())) {
                        m_array.fastRemove(i);
                        --i;
                    }
                }
                m_rememberCallsSinceLastCheck = 0;
            }

            m_array.append(r);
            return r;
        }

        void cleanup() {
            for (int i = 0; i < m_array.length(); ++i) {
                const shared_ptr<T>& r = m_array[i].lock();
                if (notNull(r)) {
                    r->cleanup();
                }
            }
            m_array.clear();
        }
    };

    

    /** For cleaning up during shutdown */
    WeakCleanupArray<Sound>         m_soundArray;

    /** For cleaning up during shutdown */
    WeakCleanupArray<AudioChannel>  m_channelArray;

    bool                            m_enable;

public:

    FMOD::System*                   m_fmodSystem;

    enum {ANY_FREE = -1};

    static AudioDevice* instance;

    AudioDevice();

    ~AudioDevice();

    /** Invoke once per frame on the main thread to service the audio system. RenderDevice::swapBuffers automatically invokes this. */
    void update();

    /** \sa G3D::Scene::setActiveListener */
    void setListener3DAttributes(const CFrame& listenerFrame, const Vector3& listenerVelocity);

    /** The value from init() of the enableSound argument */
    bool enabled() const {
        return m_enable;
    }

    /** \param numVirtualChannels Number of channels to allocate.  There is no reason not to make this fairly
        large. The limit is 4093 and 1000 is the default inherited from FMOD.
        
        \param enableSound If false, then AudioDevice exists but no sounds will play and FMOD is not initialized.
        This is convenient for debugging a program that uses sound, so that Sound objects can still be instantiated
        but no disk access or sound-related performance delays will occur.

        \param bufferLength Length of DSP buffer to use. Affects latency and 1024 is the default from FMOD. FMOD
        claims that the default results in 21.33 ms of latency at 48 khz (1024 / 48000 * 1000 = 21.33). FMOD warns
        that bufferlength is generally best left alone.

        \param numBuffers Number of DSP buffers to use. Default from FMOD is 4. Similar to bufferLength, FMOD warns
        against changing this value.
        */
    void init(bool enableSound = true, int numVirtualChannels = 1000, unsigned int bufferLength = 1024, int numBuffers = 4);

    /** Destroys all Sounds and AudioChannels and shuts down the FMOD library. */
    void cleanup();
};


/** \brief Sound file loaded into memory that can be played on an AudioChannel.

    Analogous to a graphics texture...a (typically) read-only value that can be

    \sa AudioChannel
    
  */
class Sound : public ReferenceCountedObject {
protected:
    friend class AudioDevice;
    friend class AudioChannel;

    // For serialization only
    String          m_filename;
    bool            m_loop;
    bool            m_positional;

    String          m_name;

    FMOD::Sound*    m_fmodSound;

    Sound();

    /** Delete resources */
    void cleanup();

public:

    /** \param positional Set to true for 3D audio */
    static shared_ptr<Sound> create(const String& filename, bool loop = false, bool positional = false);

    /** Positional defaults to true for this constructor. */
    static shared_ptr<Sound> create(const Any& a);

    static const int16 DEFAULT_FREQUENCY = -1;

    const String& name() const {
        return m_name;
    }

    Any toAny() const;

    /** Returns the channel on which the sound is playing so that it can be terminated or
        adjusted. The caller is not required to retain the AudioChannel pointer to keep the 
        sound playing. 
        
        \sa G3D::Entity::play, G3D::SoundEntity, G3D::AudioChannel::stop
        */
    shared_ptr<AudioChannel> play(float initialVolume = 1.0f, float initialPan = 0.0f, float initialFrequency = DEFAULT_FREQUENCY, bool startPaused = false);

    /** Play a positional sound. The initialPosition and initialVelocity are ignored if the sound was not loaded positionally.
    
       \sa G3D::Entity::play, G3D::SoundEntity, G3D::AudioChannel::stop
      */
    shared_ptr<AudioChannel> play(const Point3& initialPosition, const Vector3& initialVelocity, float initialVolume = 1.0f, float initialFrequency = DEFAULT_FREQUENCY, bool startPaused = false);

    ~Sound();
};

} // namespace
#endif

