#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include "components.h"
#include <qstring.h>

/**  @brief Entity
  Simple entity which holds has pointers to all its components.
  @warning This is a verry simplified ECS and is further discussed in the video and/or document.
 */
class GameObject
{
public:
    std::string name{0};
    std::string filepath{"-"};
    int id{0};

    TransformComponent* mTransformComp{nullptr};
    MeshComponent* mMeshComp{nullptr};
    MaterialComponent* mMaterialComp{nullptr};
    CollisionComponent* mCollisionComp{nullptr};
    MeshComponent* mCollisionLines{nullptr};
    AIComponent* mAIComponent{nullptr};
    SoundSourceComponent* mSoundSourceComp{nullptr};
    BallPhysicsComponent* mBallPhysicsComp{nullptr};
};

#endif // GAMEOBJECT_H
