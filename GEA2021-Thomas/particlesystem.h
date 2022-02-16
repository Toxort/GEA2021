#ifndef PARTICLESYSTEM_H
#define PARTICLESYSTEM_H

#include "vector3d.h"
#include "gameobject.h"
#include "particle.h"
/** @brief Particle system for blood visual effect

  Simple particle system

  Resource use for making this: ThinMatrix https://www.youtube.com/watch?v=6PkjU9LaDTQ

  @todo Instansing, transparrant textures and billboard shader.
 */
class ParticleSystem
{
public:
    ParticleSystem();
    /** @brief Holds all the Particles
     */
    static std::vector<Particle*> particles;

    /** @brief Updates all the paricles in the paricles array

      updates if the paricle should be deleted

      @see Particle::update()
     */
    static void update();

    /** Add Particle to the particles array

       @param particle
     */
    static void addParticle(Particle* particle);
};

#endif // PARTICLESYSTEM_H
