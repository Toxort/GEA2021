#ifndef COLLISIONSYSTEM_H
#define COLLISIONSYSTEM_H
#include <vector>
#include "vertex.h"


class Components;
class GameObject;

/**
  @brief System for collision detection

(incomplete)
 */
class CollisionSystem
{
public:
    CollisionSystem();

    /** @brief AABB collision detection

      Checks if the player will collied based on the position it would have in the next frame.
      If the players hitbox(in the next frame) overlaps with the collision object(static) in all the axis
      the function will return true.


       @param player
       @param objects
       @param playerNextPos
       @return player collided or not
     */
    static bool isColliding(GameObject * player, std::vector<GameObject*> objects, QVector3D playerNextPos);
};

#endif // COLLISIONSYSTEM_H
