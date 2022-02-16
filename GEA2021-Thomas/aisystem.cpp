#include "aisystem.h"
#include "particle.h"
#include "particlesystem.h"
#include "vector3d.h"

#include<time.h>
#include<cstdlib>
#include <random>

AISystem::AISystem()
{

}

bool AISystem::onHit(GameObject *obj)
{
    if(obj->mAIComponent->hp <1)
    {
        obj->mSoundSourceComp->mSoundSource[1]->setPosition(obj->mTransformComp->mMatrix.getPosition());
        obj->mSoundSourceComp->mSoundSource[1]->play();
        return true;
//        on_actionDelete_Selected_triggered();
    }else
    {
        //Generate random number
        for(int i = 0; i < 10;i++)
            ParticleSystem::addParticle(new Particle(gsl::Vector3D(randomNumber(0,3,true),randomNumber(1,4),randomNumber(0,3,true)), obj->mTransformComp->mMatrix.getPosition()));

        obj->mSoundSourceComp->mSoundSource[0]->setPosition(obj->mTransformComp->mMatrix.getPosition());
        obj->mSoundSourceComp->mSoundSource[0]->play();

        obj->mAIComponent->hp--;
        return false;
    }
}

void AISystem::updateMovement(GameObject *player, std::vector<GameObject *> objects)
{

    for(int i = 0; i < objects.size(); i++)
    {
        if(objects.at(i)->mAIComponent == nullptr)
            continue;

        gsl::Vector3D playerPos = player->mTransformComp->mMatrix.getPosition();
        // raise the position so enemies go for the head
        playerPos = playerPos + gsl::Vector3D(0,2,0);

        gsl::Vector3D enemyPos = objects.at(i)->mTransformComp->mMatrix.getPosition();

        gsl::Vector3D distanceVec = playerPos-enemyPos;

        if(distanceVec.length() < 10)
        {
            if(objects.at(i)->mSoundSourceComp->mSoundSource[2] == nullptr)
                continue;

            float dt = 0.003f;

            objects.at(i)->mAIComponent->mPosition = enemyPos + distanceVec*dt;

            objects.at(i)->mTransformComp->mMatrix.setPosition(objects.at(i)->mAIComponent->mPosition.x, objects.at(i)->mAIComponent->mPosition.y, objects.at(i)->mAIComponent->mPosition.z);

            // to play sound once when its inside the radius
            if(objects.at(i)->mAIComponent->heheSoundOnce)
            {
                objects.at(i)->mSoundSourceComp->mSoundSource[2]->setPosition(objects.at(i)->mTransformComp->mMatrix.getPosition());
                objects.at(i)->mSoundSourceComp->mSoundSource[2]->play();
                objects.at(i)->mAIComponent->heheSoundOnce = false;
                continue;
            }
        }else
        objects.at(i)->mAIComponent->heheSoundOnce = true;
    }
}




float AISystem::randomNumber(int min, int max, bool negative)
{
    std::random_device rd; // obtain a random number from hardware
    std::mt19937 gen(rd()); // seed the generator
    std::uniform_int_distribution<> distr(min, max); // define the range
    float posrnd = distr(rd);
    float negrnd = -distr(rd);

    srand(time(0));
    int rand = 1-(std::rand() % 2);
    qDebug() << "Rand: " << rand;
    if(rand || !negative)
    {
        return posrnd;
    }else
        return negrnd;
}
