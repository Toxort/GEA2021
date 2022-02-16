#include "camera.h"
#include <QDebug>

Camera::Camera()
{
    mViewMatrix.setToIdentity();
    mProjectionMatrix.setToIdentity();


    mYawMatrix.setToIdentity();
    mPitchMatrix.setToIdentity();
}

void Camera::pitch(float degrees)
{
    //  rotate around mRight
    mPitch -= degrees;
    updateForwardVector();
}

void Camera::yaw(float degrees)
{
    // rotate around mUp
    mYaw -= degrees;
    updateForwardVector();
}

void Camera::updateRightVector()
{
    mRight = mForward^mUp;
    mRight.normalize();

    // y should allways be 0
    mRight.y = 0;
    //qDebug() << "Right " << mRight;
}

void Camera::updateForwardVector()
{
    mRight = gsl::Vector3D(1.f, 0.f, 0.f);
    mRight.rotateY(mYaw);
    mRight.normalize();
    mUp = gsl::Vector3D(0.f, 1.f, 0.f);
    mUp.rotateX(mPitch);
    mUp.rotateY(mYaw);
    mUp.normalize();
    mForward = mUp^mRight;



    updateRightVector();
}

void Camera::update()
{
    mYawMatrix.setToIdentity();
    mPitchMatrix.setToIdentity();

    mPitchMatrix.rotateX(mPitch);
    mYawMatrix.rotateY(mYaw);

    mPosition -= mForward * mSpeed;

    mViewMatrix = mPitchMatrix* mYawMatrix;
    mViewMatrix.translate(-mPosition);
}

void Camera::updateFirstPerson()
{
    mYawMatrix.setToIdentity();
    mPitchMatrix.setToIdentity();

    mPitchMatrix.rotateX(mPitch);
    mYawMatrix.rotateY(mYaw);

    gsl::Vector3D fpsForardMovement {mForward.x, 0, mForward.z};
    mPosition -= fpsForardMovement * mSpeed*2;


    mViewMatrix = mPitchMatrix* mYawMatrix;
    mViewMatrix.translate(-mPosition);
}

void Camera::handleMouseMovement(float xoffset, float yoffset)
{

    xoffset *= MouseSens;
    yoffset *= MouseSens;

    mYaw   += xoffset;
    mPitch += yoffset;

    // make sure that when pitch is out of bounds, screen doesn't get flipped

        if (mPitch > 89.0f)
            mPitch = 89.0f;
        if (mPitch < -89.0f)
            mPitch = -89.0f;


    // update Front, Right and Up Vectors using the updated Euler angles
        updateForwardVector();
    //update();
}

gsl::Vector3D Camera::getFowrardVector()
{
    return mForward;
}

gsl::Vector3D Camera::getRightVector()
{
    return mRight;
}

void Camera::setPosition(const gsl::Vector3D &position)
{
    mPosition = position;
}

void Camera::setSpeed(float speed)
{
    mSpeed = speed;
}

void Camera::updateHeigth(float deltaHeigth)
{
    mPosition.y += deltaHeigth;
}

void Camera::moveForward(float speed)
{
//    QVector3D forward{mForward.x,mForward.y,mForward.z};
//    QVector3D pos{mPosition.x, mPosition.y, mPosition.z};
//    QVector3D result = pos.x() * forward * speed ;
//    mPosition = {result.x(),result.y(), result.z()};

}

void Camera::moveRight(float delta)
{
    //This fixes a bug in the up and right calculations
    //so camera always holds its height when straifing
    //should be fixed thru correct right calculations!
    gsl::Vector3D right = mRight;
    right.y = 0.f;
    mPosition += right * delta;
}

gsl::Vector3D Camera::position() const
{
    return mPosition;
}

gsl::Vector3D Camera::up() const
{
    return mUp;
}

void Camera::setYaw(float newYaw)
{
    mYaw = newYaw;
}

void Camera::setPitch(float newPitch)
{
    mPitch = newPitch;
}
