#ifndef GAME_SOUND_SOUNDMANAGER_H
#define GAME_SOUND_SOUNDMANAGER_H

#include <string>
#include <utility>
#include <deque>
#include <map>

#include <boost/shared_ptr.hpp>

#include <components/settings/settings.hpp>

#include "loudness.hpp"
#include "../mwbase/soundmanager.hpp"

namespace VFS
{
    class Manager;
}

namespace ESM
{
    struct Sound;
}

namespace MWSound
{
    class Sound_Output;
    struct Sound_Decoder;
    class Sound;
    class Sound_Buffer;

    enum Environment {
        Env_Normal,
        Env_Underwater
    };

    class SoundManager : public MWBase::SoundManager
    {
        const VFS::Manager* mVFS;

        std::auto_ptr<Sound_Output> mOutput;

        // Caches available music tracks by <playlist name, (sound files) >
        std::map<std::string, std::vector<std::string> > mMusicFiles;
        std::string mLastPlayedMusic; // The music file that was last played

        float mMasterVolume;
        float mSFXVolume;
        float mMusicVolume;
        float mVoiceVolume;
        float mFootstepsVolume;

        typedef std::deque<Sound_Buffer> SoundBufferList;
        // List of sound buffers, grown as needed. New enties are added to the
        // back, allowing existing Sound_Buffer references/pointers to remain
        // valid.
        SoundBufferList mSoundBuffers;
        size_t mBufferCacheMin;
        size_t mBufferCacheMax;
        size_t mBufferCacheSize;

        typedef std::map<std::string,Sound_Buffer*> NameBufferMap;
        NameBufferMap mBufferNameMap;

        typedef std::map<std::string,Sound_Loudness> NameLoudnessMap;
        NameLoudnessMap mVoiceLipBuffers;

        // NOTE: unused buffers are stored in front-newest order.
        typedef std::deque<Sound_Buffer*> SoundList;
        SoundList mUnusedBuffers;

        boost::shared_ptr<Sound> mMusic;
        std::string mCurrentPlaylist;

        typedef std::pair<MWBase::SoundPtr,Sound_Buffer*> SoundBufferRefPair;
        typedef std::vector<SoundBufferRefPair> SoundBufferRefPairList;
        typedef std::map<MWWorld::Ptr,SoundBufferRefPairList> SoundMap;
        SoundMap mActiveSounds;

        typedef std::pair<MWBase::SoundPtr,std::string> SoundNamePair;
        typedef std::map<MWWorld::Ptr,SoundNamePair> SaySoundMap;
        SaySoundMap mActiveSaySounds;

        MWBase::SoundPtr mUnderwaterSound;

        bool mListenerUnderwater;
        osg::Vec3f mListenerPos;
        osg::Vec3f mListenerDir;
        osg::Vec3f mListenerUp;

        int mPausedSoundTypes;

        Sound_Buffer *insertSound(const std::string &soundId, const ESM::Sound *sound);

        Sound_Buffer *lookupSound(const std::string &soundId) const;
        Sound_Buffer *loadSound(const std::string &soundId);

        // Ensures the loudness/"lip" data is loaded, and returns a decoder to
        // start streaming
        DecoderPtr loadVoice(const std::string &voicefile);

        void streamMusicFull(const std::string& filename);
        bool updateSound(MWBase::SoundPtr sound, const MWWorld::Ptr &ptr, float duration);
        void updateSounds(float duration);
        void updateRegionSound(float duration);

        float volumeFromType(PlayType type) const;

        SoundManager(const SoundManager &rhs);
        SoundManager& operator=(const SoundManager &rhs);

    protected:
        DecoderPtr getDecoder();
        friend class OpenAL_Output;

    public:
        SoundManager(const VFS::Manager* vfs, bool useSound);
        virtual ~SoundManager();

        virtual void processChangedSettings(const Settings::CategorySettingVector& settings);

        virtual void stopMusic();
        ///< Stops music if it's playing

        virtual void streamMusic(const std::string& filename);
        ///< Play a soundifle
        /// \param filename name of a sound file in "Music/" in the data directory.

        virtual void startRandomTitle();
        ///< Starts a random track from the current playlist

        virtual bool isMusicPlaying();
        ///< Returns true if music is playing

        virtual void playPlaylist(const std::string &playlist);
        ///< Start playing music from the selected folder
        /// \param name of the folder that contains the playlist

        virtual void say(const MWWorld::Ptr &reference, const std::string& filename);
        ///< Make an actor say some text.
        /// \param filename name of a sound file in "Sound/" in the data directory.

        virtual void say(const std::string& filename);
        ///< Say some text, without an actor ref
        /// \param filename name of a sound file in "Sound/" in the data directory.

        virtual bool sayDone(const MWWorld::Ptr &reference=MWWorld::Ptr()) const;
        ///< Is actor not speaking?

        virtual void stopSay(const MWWorld::Ptr &reference=MWWorld::Ptr());
        ///< Stop an actor speaking

        virtual float getSaySoundLoudness(const MWWorld::Ptr& reference) const;
        ///< Check the currently playing say sound for this actor
        /// and get an average loudness value (scale [0,1]) at the current time position.
        /// If the actor is not saying anything, returns 0.

        virtual MWBase::SoundPtr playTrack(const DecoderPtr& decoder, PlayType type);
        ///< Play a 2D audio track, using a custom decoder

        virtual MWBase::SoundPtr playSound(const std::string& soundId, float volume, float pitch, PlayType type=Play_TypeSfx, PlayMode mode=Play_Normal, float offset=0);
        ///< Play a sound, independently of 3D-position
        ///< @param offset value from [0,1], when to start playback. 0 is beginning, 1 is end.

        virtual MWBase::SoundPtr playSound3D(const MWWorld::Ptr &reference, const std::string& soundId,
                                             float volume, float pitch, PlayType type=Play_TypeSfx,
                                             PlayMode mode=Play_Normal, float offset=0);
        ///< Play a 3D sound attached to an MWWorld::Ptr. Will be updated automatically with the Ptr's position, unless Play_NoTrack is specified.
        ///< @param offset Value from [0,1] meaning from which fraction the sound the playback starts.

        virtual MWBase::SoundPtr playSound3D(const osg::Vec3f& initialPos, const std::string& soundId,
                                             float volume, float pitch, PlayType type, PlayMode mode, float offset=0);
        ///< Play a 3D sound at \a initialPos. If the sound should be moving, it must be updated using Sound::setPosition.

        ///< Play a sound from an object
        ///< @param offset value from [0,1], when to start playback. 0 is beginning, 1 is end.

        virtual void stopSound3D(const MWWorld::Ptr &reference, const std::string& soundId);
        ///< Stop the given object from playing the given sound,

        virtual void stopSound3D(const MWWorld::Ptr &reference);
        ///< Stop the given object from playing all sounds.

        virtual void stopSound(const MWWorld::CellStore *cell);
        ///< Stop all sounds for the given cell.

        virtual void stopSound(const std::string& soundId);
        ///< Stop a non-3d looping sound

        virtual void fadeOutSound3D(const MWWorld::Ptr &reference, const std::string& soundId, float duration);
        ///< Fade out given sound (that is already playing) of given object
        ///< @param reference Reference to object, whose sound is faded out
        ///< @param soundId ID of the sound to fade out.
        ///< @param duration Time until volume reaches 0.

        virtual bool getSoundPlaying(const MWWorld::Ptr &reference, const std::string& soundId) const;
        ///< Is the given sound currently playing on the given object?

        virtual void pauseSounds(int types=Play_TypeMask);
        ///< Pauses all currently playing sounds, including music.

        virtual void resumeSounds(int types=Play_TypeMask);
        ///< Resumes all previously paused sounds.

        virtual void update(float duration);

        virtual void setListenerPosDir(const osg::Vec3f &pos, const osg::Vec3f &dir, const osg::Vec3f &up);

        virtual void updatePtr (const MWWorld::Ptr& old, const MWWorld::Ptr& updated);

        virtual void clear();
    };
}

#endif
