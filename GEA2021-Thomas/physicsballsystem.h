#ifndef PHYSICSBALLSYSTEM_H
#define PHYSICSBALLSYSTEM_H

#include "QVector3D"
#include "gameobject.h"
#include "queue"
#include <array>
class PhysicsBallSystem
{
public:
    PhysicsBallSystem();

    void update(GameObject& ballInn);
    void SetTerrainData(GameObject& TerrainInn);
    void move(float dt);
    void StartRolling();


//    static PhysicsBallSystem* getInstance();

    QVector3D barycentricCoordinates(const QVector2D &dette, const QVector2D &p1, const QVector2D &p2, const QVector2D &p3);
    QVector3D getAkselerationVector(GameObject& ballInn);
    float getHeightbyBarycentricCoordinates(GameObject& ballInn);

    GameObject *GetTerrain();

    void resetBall();

private:
//    float xPosOnTarrain;
//    float zPosOnTarrain;
    float gridSquareSize;
//    int gridXPos;
//    int gridZPos;
    float xzScale{10};
    int mRows{146};
    int mCols{100};

//    float height{0};

//    std::vector<gsl::Vector3D> mAllDataPoints;
//    double xMin{0};
//    double xMax{0};
//    double zMin{0};
//    double zMax{0};
//    double yMin{0};
//    double yMax{0};
//    int hMapRows{998};
//    int hMapCols{1457};



    //hardcoded for now should change when spawning on different square
    //make a init bool to get and set the curret pos;
//    QVector3D collisionVector{0,0,0};

//    QVector3D normal;
//    QVector3D collisionNormal{0,0,0};
//    bool normalSetup{true};
//    QVector3D lastNormal{0,0,0};
//    QVector3D currentNormal{0,0,0};
//    std::queue<QVector3D> normalLastAndCurrentQ;
//    bool onNewTriangle{false};
//    int id{0};

//    int lastSquareID{0};
//    int currentSquareID{0};

//    QVector3D lastNormal{9,9,9};
//    QVector3D BotLeftNormal{0,0,0};
//    QVector3D TopRightNormal{0,0,0};


//    QVector3D mVelocity{0,0,0};
//    QVector3D mAcceleration{0,0,0};
//    QVector3D mPosition{0,0,0};
//    QVector3D mFriction{0.3f,0.3f,0.3f};
//    static PhysicsBallSystem* mInstance;
    GameObject* mTerrain;
};

#endif // PHYSICSBALLSYSTEM_H
