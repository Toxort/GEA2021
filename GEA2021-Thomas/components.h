#ifndef COMPONENTS_H
#define COMPONENTS_H

#include "constants.h"
#include "vertex.h"
#include "matrix4x4.h"
#include "gltypes.h"
#include "queue"
#include "soundsource.h"

#include <al.h>
#include <alc.h>

class Copmonents
{
public:
    Copmonents();
};

class TransformComponent
{
public:
    gsl::Matrix4x4 mMatrix;
    gsl::Matrix4x4 mScaleMatrix;
    gsl::Vector3D mRotation{0.f,0.f,0.f};
    gsl::Vector3D mScale{1.f,1.f,1.f};

};

class MeshComponent
{
public:
    std::vector<Vertex> mVertices[3];
    std::vector<GLuint> mIndices[3];

    GLuint mVAO[3]{0};
    GLuint mVBO[3]{0};
    GLuint mEAB[3]{0};

    GLenum mDrawType{GL_TRIANGLES};
    bool bUsingLOD{false};
    bool bUsingFrustumCulling{true};
};

class MaterialComponent
{
public:
    GLuint mShaderProgram{};
    GLint mTextureUnit{};
    std::string mTextureName;
};

class CollisionComponent
{
public:
    bool bShowCollisionBox{false};

    QVector3D maxCorner{0,0,0};
    QVector3D minCorner{0,0,0};
    QVector3D maxCornerRotated{0,0,0};
    QVector3D minCornerRotated{0,0,0};

    float mRaidus{0};

    /// AABB is axis oriented so it only has two orientations.
    bool bRotated{false};
    gsl::Matrix4x4 mMatrix;
    bool bSetRotatedOnce{true};
};

class AIComponent
{
public:
    bool isAlive{true};
    int hp{5};
    gsl::Vector3D mVelocity{0,0,0};
    gsl::Vector3D mPosition{0,0,0};
    bool heheSoundOnce{true};
};

class BallPhysicsComponent
{
public:
    QVector3D mVelocity{0,0,0};
    QVector3D mPosition{0,0,0};
    float Radius{1};
    float Mass{1};
    int gridXPos;
    int gridZPos;
    float xPosOnTarrain;
    float zPosOnTarrain;
    QVector3D inAirNormal{0,0,0};
    QVector3D normal{0,0,0};
    QVector3D collisionNormal{0,1,0};
    bool normalSetup{true};
//    QVector3D lastNormal{0,0,0};
//    QVector3D currentNormal{0,0,0};
    std::queue<QVector3D> normalLastAndCurrentQ;
    bool onNewTriangle{false};
    bool isInAir{true};
};

class SoundSourceComponent
{
public:
    SoundSource* mSoundSource[3];
    ALuint mSource;             ///< The sound source.
    ALuint mBuffer;
    gsl::Vector3D mVelocity;    ///< Vector containing source velocity.
    bool looping;
    float gain;
};

class SoundListenerComponent
{
public:
    //should be fetced from other component?
    gsl::Vector3D vel{0.0f, 0.0f, 0.0f};
    gsl::Vector3D dir{0.0f, 0.0f, 1.0f};
    gsl::Vector3D up{0.0f, 1.0f, 0.0f};
};



#endif // COMPONENTS_H
