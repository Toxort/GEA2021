#include "particlesystem.h"
#include "vector"

std::vector<Particle*> ParticleSystem::particles{nullptr};

ParticleSystem::ParticleSystem()
{

}

void ParticleSystem::update()
{
    for(int i = 0; i < particles.size();i++)
    {
        if(particles.at(i)== nullptr)
            continue;

        bool isAlive = particles.at(i)->update();
        if(!isAlive)
        {
            particles.erase(particles.begin() + i);
        }
    }
}

void ParticleSystem::addParticle(Particle* particle)
{
    particles.push_back(particle);
}
