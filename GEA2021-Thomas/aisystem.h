#ifndef AISYSTEM_H
#define AISYSTEM_H
#include "gameobject.h"
#include <vector>
/**
  @brief System to update enemy movement and behavior when hit
 */
class AISystem
{
public:
    AISystem();
    /** @brief Simple fuctnion for behavior when hit

       @param obj - Object which is hit
       @return - Enemy alive status
     */
    static bool onHit(GameObject* obj);
    /** @brief Simple Enemy movement

      All objects with AI component will move towards the player. And make a noice when it starts moving.
      It starts moving withing a 10m.

       @param player
       @param objects
     */
    static void updateMovement(GameObject* player, std::vector<GameObject*> objects);

private:

    static float randomNumber(int min, int max, bool negative = false);
};

#endif // AISYSTEM_H
