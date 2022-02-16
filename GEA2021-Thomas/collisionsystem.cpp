#include "collisionsystem.h"
#include "components.h"
#include "gameobject.h"

#include <vector>

CollisionSystem::CollisionSystem()
{

}

bool CollisionSystem::isColliding(GameObject *player, std::vector<GameObject*> objects, QVector3D playerNextPos)
{
    QVector3D playerPos = playerNextPos;

    QVector3D playerMaxCorner = player->mCollisionComp->maxCorner;
    QVector3D playerMinCorner = player->mCollisionComp->minCorner;

    for(unsigned long long i{2}; i < objects.size() ; i++)
    {
        QVector3D otherPos = {objects[i]->mTransformComp->mMatrix.getPosition().x,
                              objects[i]->mTransformComp->mMatrix.getPosition().y,
                              objects[i]->mTransformComp->mMatrix.getPosition().z};
        QVector3D otherMaxCorner = objects[i]->mCollisionComp->maxCorner;
        QVector3D otherMinCorner = objects[i]->mCollisionComp->minCorner;

        bool xAxis = playerPos.x() + playerMinCorner.x() <= otherPos.x() + otherMaxCorner.x() &&
                otherPos.x() + otherMinCorner.x() < playerPos.x() + playerMaxCorner.x();
        bool zAxis = playerPos.z() + playerMinCorner.z() <= otherPos.z() + otherMaxCorner.z() &&
                otherPos.z() + otherMinCorner.z() < playerPos.z() + playerMaxCorner.z();
        bool yAxis =playerPos.y() + playerMinCorner.y() <= otherPos.y() + otherMaxCorner.y() &&
                otherPos.y() + otherMinCorner.y() <= playerPos.y() + playerMaxCorner.y();

        if(xAxis && zAxis && yAxis)
        {
            //qDebug() << i;
            return true;
        }
    }
    return false;
}




