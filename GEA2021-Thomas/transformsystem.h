#ifndef TRANSFORMSYSTEM_H
#define TRANSFORMSYSTEM_H
#include "vector3d.h"
class GameObject;



 /** @brief Tranforms system for moving, scaling and rotating with UI


    Inspired by teamTOG's method of scaling and rotating

    There was some strange morphing and streaching when scaling, when the object is tilted
    (not rotation = (0,0,0)). I needed setScale and setRotation functionallity.
    I took inspiraton from teamTOG's method of scaling and rotating and modified it to fit my code.

   @author Inspiration: Gudbrand Dynna https://github.com/Hedmark-University-College-SPIM/GEA2021/tree/team-TOG-insurance
 */
class TransformSystem
{
public:
    TransformSystem();

    static TransformSystem* getInstance();

    static void setPosition(GameObject* obj, gsl::Vector3D moveToPos);
    static void setScale(GameObject* obj, gsl::Vector3D scaleToValue);
    static void setRotation(GameObject* obj, gsl::Vector3D rotateToValue);
private:
    static void updateMatrix(GameObject* obj);
    static TransformSystem* mInstance;

    bool rotateOnce{true};
};

#endif // TRANSFORMSYSTEM_H
