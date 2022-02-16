#ifndef CAMERA_H
#define CAMERA_H

#include "matrix4x4.h"
#include "vector3d.h"

/**
  This class still have some bugs. It mostly work, but when you rotate the camera 180 degrees
  the forward / backward is wrong, when steered with W and S.
 */
class Camera
{
public:
    Camera();

    void pitch(float degrees);
    void yaw(float degrees);
    void updateRightVector();
    void updateForwardVector();
    void update();
    void updateFirstPerson();
    void handleMouseMovement(float xoffset, float yoffset);

    gsl::Vector3D getFowrardVector();
    gsl::Vector3D getRightVector();

    gsl::Matrix4x4 mViewMatrix;
    gsl::Matrix4x4 mProjectionMatrix;

    void setPosition(const gsl::Vector3D &position);

    void setSpeed(float speed);
    void updateHeigth(float deltaHeigth);
    void moveForward(float speed);
    void moveRight(float delta);

    gsl::Vector3D position() const;
    gsl::Vector3D up() const;

    void setYaw(float newYaw);

    void setPitch(float newPitch);

private:
    // cam options
    float MoveSpeed;
    float MouseSens {0.05};

    gsl::Vector3D mForward{0.f, 0.f, 1.f};
    gsl::Vector3D mRight{1.f, 0.f, 0.f};
    gsl::Vector3D mUp{0.f, 1.f, 0.f};

    gsl::Vector3D mPosition{0.f, 0.f, 0.f};
    float mPitch{0.f};
    float mYaw{0.f};

    gsl::Matrix4x4 mYawMatrix;
    gsl::Matrix4x4 mPitchMatrix;

    float mSpeed{0.f}; //camera will move by this speed along the mForward vector
};

#endif // CAMERA_H
