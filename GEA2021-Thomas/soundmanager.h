#ifndef SOUNDMANAGER_H
#define SOUNDMANAGER_H

#ifdef _WIN32
#include <al.h>
#include <alc.h>
#endif
#ifdef __APPLE__
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#endif

#include <string>
#include <vector>
#include "soundsource.h"
#include "vector3d.h"

/// @brief Class handling OpenAL setup and management of sound sources.
/**
    Singleton class that creates and destroys sound sources.
    @author Ole Flaten https://github.com/Hedmark-University-College-SPIM/GEA2021/tree/ole_experiments
**/
class SoundManager
{
public:
    static SoundManager* getInstance()          ///< Get pointer to singleton instance.
    {
        if (!mInstance)
        {
            mInstance = new SoundManager();
            mInstance->init();
        }
        return mInstance;
    }

    /// Initialises OpenAL.
    bool init();
    /// Cleans up and shuts down OpenAL.
    void cleanUp();

    /// Creates a new SoundSource with the given parameters.
    /**
        \param The name of the sound. (Not in use.)
        \param The source position as Vector3.
        \param File path relative to execution directory.
        \param Boolean to see if sound should loop or not.
    **/
    SoundSource* createSource(std::string name, gsl::Vector3D pos, std::string filePath = "", bool loop = false, float gain = 1.0);
    void updateListener(gsl::Vector3D pos, gsl::Vector3D vel, gsl::Vector3D dir, gsl::Vector3D up);

private:
    SoundManager();                         ///< Private constructor.
    SoundManager(SoundManager const&);      ///< Private copy constructor.
    void operator=(SoundManager const&);    ///< Private class assignment operator.
    static SoundManager* mInstance;         ///< Singleton instance pointer.
    /// Debug function.
    /**
        Checks for and outputs OpenAL errors.
    **/
    bool checkError();

    ALCdevice* mDevice;                 ///< Pointer to the ALC Device.
    ALCcontext* mContext;               ///< Pointer to the ALC Context.
};

#endif
