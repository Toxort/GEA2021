#include "physicsballsystem.h"
#include "queue"
#include <stdlib.h>
#include "components.h"

//PhysicsBallSystem* PhysicsBallSystem::mInstance = nullptr;


PhysicsBallSystem::PhysicsBallSystem()
{

}

void PhysicsBallSystem::update(GameObject& ballInn)
{

    gsl::Vector3D currentpos = ballInn.mTransformComp->mMatrix.getPosition();

    ballInn.mBallPhysicsComp->mPosition = QVector3D{currentpos.x,currentpos.y,currentpos.z};
    QVector3D aVector = getAkselerationVector(ballInn);

    float dt = 0.17f;
    // oppdatere hastighet
    ballInn.mBallPhysicsComp->mVelocity = ballInn.mBallPhysicsComp->mVelocity + aVector * dt;

    if(ballInn.mBallPhysicsComp->onNewTriangle && !ballInn.mBallPhysicsComp->isInAir)
    {
        //Sette kollisjonsnormal
        ballInn.mBallPhysicsComp->collisionNormal = (ballInn.mBallPhysicsComp->mVelocity - 2.f * (QVector3D::dotProduct(ballInn.mBallPhysicsComp->mVelocity, ballInn.mBallPhysicsComp->normal)) * ballInn.mBallPhysicsComp->normal);
        ballInn.mBallPhysicsComp->collisionNormal.normalize();


        if(ballInn.mBallPhysicsComp->isInAir)
            ballInn.mBallPhysicsComp->mVelocity = ballInn.mBallPhysicsComp->collisionNormal * ballInn.mBallPhysicsComp->mVelocity.length() / 1.0f;
        else
            ballInn.mBallPhysicsComp->mVelocity = ballInn.mBallPhysicsComp->collisionNormal * ballInn.mBallPhysicsComp->mVelocity.length() / 1.3f;


        // Kode jeg brukte fra del 1
//        QVector3D m = ballInn.mBallPhysicsComp->normalLastAndCurrentQ.front();
//        QVector3D n = ballInn.mBallPhysicsComp->normalLastAndCurrentQ.back();

//        ballInn.mBallPhysicsComp->collisionNormal = (m + n)/ (m.length()+n.length());
//        ballInn.mBallPhysicsComp->collisionNormal.normalize();

//        QVector3D nextVelocity = ballInn.mBallPhysicsComp->mVelocity - (2 * (ballInn.mBallPhysicsComp->mVelocity * ballInn.mBallPhysicsComp->collisionNormal) * ballInn.mBallPhysicsComp->collisionNormal);
//        ballInn.mBallPhysicsComp->mVelocity = nextVelocity;
    }

    //Sette posisjon i henhold til newtons annen lov (Masse er 1 så den er ikke tatt med i utregningen)
    ballInn.mBallPhysicsComp->mPosition = ballInn.mBallPhysicsComp->mPosition + (ballInn.mBallPhysicsComp->mVelocity + 0.5 * QVector3D{0,-9.81,0}) * dt * dt;
    ballInn.mTransformComp->mMatrix.setPosition(ballInn.mBallPhysicsComp->mPosition.x(), ballInn.mBallPhysicsComp->mPosition.y(), ballInn.mBallPhysicsComp->mPosition.z() );


 // om ballen er under flate, setter jeg høyden til å være høyden til flaten.
    if(ballInn.mTransformComp->mMatrix.getPosition().y < getHeightbyBarycentricCoordinates(ballInn) + ballInn.mBallPhysicsComp->Radius-0.05f)
        ballInn.mTransformComp->mMatrix.setPosition(ballInn.mTransformComp->mMatrix.getPosition().x,
                                                getHeightbyBarycentricCoordinates(ballInn)+0.99f,
                                                ballInn.mTransformComp->mMatrix.getPosition().z );
}

void PhysicsBallSystem::SetTerrainData(GameObject &TerrainInn)
{
    mTerrain = &TerrainInn;
}

void PhysicsBallSystem::move(float dt)
{

}

QVector3D PhysicsBallSystem::barycentricCoordinates(const QVector2D &dette, const QVector2D &p1, const QVector2D &p2, const QVector2D &p3)
{
    QVector2D p12a = p2-p1;
    QVector2D p13a = p3-p1;
    QVector3D p12{p12a.x(),p12a.y(),0};
    QVector3D p13{p13a.x(),p13a.y(),0};
    QVector3D n = QVector3D::crossProduct(p12, p13);
    float areal_123 = n.length(); // dobbelt areal
    QVector3D baryc; // til retur. Husk

    // u
    QVector2D p = p2 - dette;
    QVector2D q = p3 - dette;
    QVector3D pa{p.x(), p.y(), 0};
    QVector3D qa{q.x(), q.y(), 0};
    n = QVector3D::crossProduct(pa,qa);
    baryc.setX(n.z()/areal_123);

    // v
    p = p3 - dette;
    q = p1 - dette;
    pa = {p.x(), p.y(), 0};
    qa = {q.x(), q.y(), 0};
    n = QVector3D::crossProduct(pa,qa);
    baryc.setY(n.z()/areal_123);

    // w
    p = p1 - dette;
    q = p2 - dette;
    pa = {p.x(), p.y(), 0};
    qa = {q.x(), q.y(), 0};
    n = QVector3D::crossProduct(pa,qa);
    baryc.setZ(n.z()/areal_123);

    return baryc;
}


float PhysicsBallSystem::getHeightbyBarycentricCoordinates(GameObject& ballInn)
{

// gjør det samme som i get akslereration vector, se kommentarer der.

    ballInn.mBallPhysicsComp->xPosOnTarrain = (ballInn.mTransformComp->mMatrix.getPosition().x - mTerrain->mTransformComp->mMatrix.getPosition().x);
    ballInn.mBallPhysicsComp->zPosOnTarrain = (ballInn.mTransformComp->mMatrix.getPosition().z - mTerrain->mTransformComp->mMatrix.getPosition().z);


    gridSquareSize = xzScale;
    ballInn.mBallPhysicsComp->gridXPos = int(floor(ballInn.mBallPhysicsComp->xPosOnTarrain / xzScale));
    ballInn.mBallPhysicsComp->gridZPos = int(floor(ballInn.mBallPhysicsComp->zPosOnTarrain / xzScale));

    if(ballInn.mBallPhysicsComp->gridXPos >= 0 && ballInn.mBallPhysicsComp->gridZPos >= 0 && ballInn.mBallPhysicsComp->gridXPos < mCols && ballInn.mBallPhysicsComp->gridZPos < mRows)
    {


        float xCoordInSquare = fmod(ballInn.mBallPhysicsComp->xPosOnTarrain,gridSquareSize)/gridSquareSize;
        float zCoordInSquare = fmod(ballInn.mBallPhysicsComp->zPosOnTarrain,gridSquareSize)/gridSquareSize;

        QVector3D uvw;
        float answer;
        QVector2D p1{0,0};
        QVector2D p2{1,0};
        QVector2D p3{0,1};
        QVector2D p4{1,1};

        float p1y = mTerrain->mMeshComp->mVertices[0].at(ballInn.mBallPhysicsComp->gridZPos*mCols + (100-ballInn.mBallPhysicsComp->gridXPos)).mXYZ.getY();
        float p2y = mTerrain->mMeshComp->mVertices[0].at(ballInn.mBallPhysicsComp->gridZPos*mCols + (100-ballInn.mBallPhysicsComp->gridXPos-1)).mXYZ.getY();
        float p3y = mTerrain->mMeshComp->mVertices[0].at((ballInn.mBallPhysicsComp->gridZPos+1)*mCols + (100-ballInn.mBallPhysicsComp->gridXPos)).mXYZ.getY();
        float p4y = mTerrain->mMeshComp->mVertices[0].at((ballInn.mBallPhysicsComp->gridZPos+1)*mCols + (100-ballInn.mBallPhysicsComp->gridXPos-1)).mXYZ.getY();


        //Finne hvilken av de to trekantene spilleren står på
        if(xCoordInSquare <= (zCoordInSquare))
        {
            QVector2D posInsSquare{xCoordInSquare,zCoordInSquare};
            uvw = barycentricCoordinates(posInsSquare,p1,p4,p3);
            // regner ut høydeverdien
            answer = p1y*uvw.x()+p4y*uvw.y()+p3y*uvw.z();
            return answer;
        }
        if(xCoordInSquare > (zCoordInSquare))
        {
            uvw = barycentricCoordinates(QVector2D{float(xCoordInSquare),float(zCoordInSquare)},p1,p2,p4);
            // regner ut høydeverdien
            answer = p1y*uvw.x()+p2y*uvw.y()+p4y*uvw.z();
            return answer;
        }
    }else{
        qDebug() << "Ball is out of bounds!";
    }
    return 0.f;

}

QVector3D PhysicsBallSystem::getAkselerationVector(GameObject& ballInn)
{        
    //world koordinater, men i forhold til height mapet der 0,0 er nederst til venstre (sør-vest).

    mCols = 100;
    mRows = 146;

    ballInn.mBallPhysicsComp->xPosOnTarrain = (ballInn.mTransformComp->mMatrix.getPosition().x - mTerrain->mTransformComp->mMatrix.getPosition().x);
    ballInn.mBallPhysicsComp->zPosOnTarrain = (ballInn.mTransformComp->mMatrix.getPosition().z - mTerrain->mTransformComp->mMatrix.getPosition().z);

    gridSquareSize = xzScale;
    //hvilken grid ballen er på
    ballInn.mBallPhysicsComp->gridXPos = int(floor(ballInn.mBallPhysicsComp->xPosOnTarrain / xzScale));
    ballInn.mBallPhysicsComp->gridZPos = int(floor(ballInn.mBallPhysicsComp->zPosOnTarrain / xzScale));

    float xCoordInSquare = fmod(ballInn.mBallPhysicsComp->xPosOnTarrain,gridSquareSize)/gridSquareSize;
    float zCoordInSquare = fmod(ballInn.mBallPhysicsComp->zPosOnTarrain,gridSquareSize)/gridSquareSize;



    if(ballInn.mBallPhysicsComp->gridXPos >= 0 && ballInn.mBallPhysicsComp->gridZPos >= 0 && ballInn.mBallPhysicsComp->gridXPos < mCols-gridSquareSize && ballInn.mBallPhysicsComp->gridZPos < mRows-gridSquareSize)
    {   //Koordinat grids

        QVector2D p1_2D{0,0};
        QVector2D p2_2D{10,0};
        QVector2D p3_2D{0,10};
        QVector2D p4_2D{10,10};


        float p1y = mTerrain->mMeshComp->mVertices[0].at(ballInn.mBallPhysicsComp->gridZPos*mCols + (mCols-ballInn.mBallPhysicsComp->gridXPos)).mXYZ.getY();
        float p2y = mTerrain->mMeshComp->mVertices[0].at(ballInn.mBallPhysicsComp->gridZPos*mCols + (mCols-ballInn.mBallPhysicsComp->gridXPos-1)).mXYZ.getY();
        float p3y = mTerrain->mMeshComp->mVertices[0].at((ballInn.mBallPhysicsComp->gridZPos+1)*mCols + (mCols-ballInn.mBallPhysicsComp->gridXPos)).mXYZ.getY();
        float p4y = mTerrain->mMeshComp->mVertices[0].at((ballInn.mBallPhysicsComp->gridZPos+1)*mCols + (mCols-ballInn.mBallPhysicsComp->gridXPos-1)).mXYZ.getY();



        QVector3D p1{p1_2D.x(),p1y,p1_2D.y()};
        QVector3D p2{p2_2D.x(),p2y,p2_2D.y()};
        QVector3D p3{p3_2D.x(),p3y,p3_2D.y()};
        QVector3D p4{p4_2D.x(),p4y,p4_2D.y()};

//        qDebug() << "p1: " << p1 << "p2: " << p2 << "p3: " << p3 << "p4: " << p4;

        //Finne hvilken av de to trekantene spilleren står på
        if(xCoordInSquare <= (zCoordInSquare))
        {
            // p1, p3, p4
            QVector3D v0 = p3 - p1;
            QVector3D v1 = p4 - p1;

            ballInn.mBallPhysicsComp->normal = QVector3D::crossProduct(v0,v1);
            ballInn.mBallPhysicsComp->normal.normalize();


            // setup for queue systemet
            if(ballInn.mBallPhysicsComp->normalSetup)
            {
                ballInn.mBallPhysicsComp->normalLastAndCurrentQ.push(ballInn.mBallPhysicsComp->inAirNormal);
                ballInn.mBallPhysicsComp->normalLastAndCurrentQ.push(ballInn.mBallPhysicsComp->inAirNormal);
                ballInn.mBallPhysicsComp->normalSetup = false;
            }

            //sjekke om ballen er over baken
            if( ballInn.mTransformComp->mMatrix.getPosition().y < getHeightbyBarycentricCoordinates(ballInn) + ballInn.mBallPhysicsComp->Radius )
            {
                ballInn.mBallPhysicsComp->isInAir = false;
            }else{
                ballInn.mBallPhysicsComp->isInAir = true;
            }

            //oppdatere nåværende og forrige normal
            if(ballInn.mBallPhysicsComp->normalLastAndCurrentQ.back() != ballInn.mBallPhysicsComp->normal && ballInn.mBallPhysicsComp->isInAir == false)
            {
                ballInn.mBallPhysicsComp->normalLastAndCurrentQ.pop();
                ballInn.mBallPhysicsComp->normalLastAndCurrentQ.push(ballInn.mBallPhysicsComp->normal);
                ballInn.mBallPhysicsComp->onNewTriangle = true;
            }else
            {
                ballInn.mBallPhysicsComp->onNewTriangle = false;
            }

            // utregning av akselerasjonsvektor
            QVector3D aVec{9.81f*ballInn.mBallPhysicsComp->normal.x()*ballInn.mBallPhysicsComp->normal.y(),
                        9.81f*((ballInn.mBallPhysicsComp->normal.y()*ballInn.mBallPhysicsComp->normal.y())-1),
                        9.81f*ballInn.mBallPhysicsComp->normal.y()*ballInn.mBallPhysicsComp->normal.z()};

            // alternativ utregning av akselerasjonvektor
//            QVector3D aVec = (1.f /ballInn.mBallPhysicsComp->Mass) *
//                    ((ballInn.mBallPhysicsComp->Mass*9.81f*ballInn.mBallPhysicsComp->normal*ballInn.mBallPhysicsComp->normal.y())
//                     + ballInn.mBallPhysicsComp->Mass * QVector3D{0,-9.81f,0});
//            QVector3D aVec{0.3f,0,0};

            // om ballen er over bakken er akselerasjonvektor rett ned.
            if(ballInn.mBallPhysicsComp->isInAir)
                return QVector3D{0,-1,0};
            else
                return aVec;
        }
        // om ballen er på andre siden av quaden
        // jeg gjør det samme for begge trekantene.
        if(xCoordInSquare > (zCoordInSquare))
        {
            // p1, p2, p4
            QVector3D v0 = p4 - p1;
            QVector3D v1 = p2 - p1;

            ballInn.mBallPhysicsComp->normal = QVector3D::crossProduct(v0,v1);

            ballInn.mBallPhysicsComp->normal.normalize();

            if(ballInn.mBallPhysicsComp->normalSetup)
            {
                ballInn.mBallPhysicsComp->normalLastAndCurrentQ.push(ballInn.mBallPhysicsComp->inAirNormal);
                ballInn.mBallPhysicsComp->normalLastAndCurrentQ.push(ballInn.mBallPhysicsComp->inAirNormal);
                ballInn.mBallPhysicsComp->normalSetup = false;
            }

            if( ballInn.mTransformComp->mMatrix.getPosition().y < getHeightbyBarycentricCoordinates(ballInn) + ballInn.mBallPhysicsComp->Radius )
            {
                ballInn.mBallPhysicsComp->isInAir = false;
            }else{
                ballInn.mBallPhysicsComp->isInAir = true;
            }

            if(ballInn.mBallPhysicsComp->normalLastAndCurrentQ.back() != ballInn.mBallPhysicsComp->normal && ballInn.mBallPhysicsComp->isInAir == false)
            {

                ballInn.mBallPhysicsComp->isInAir = false;
                ballInn.mBallPhysicsComp->normalLastAndCurrentQ.pop();
                ballInn.mBallPhysicsComp->normalLastAndCurrentQ.push(ballInn.mBallPhysicsComp->normal);
                ballInn.mBallPhysicsComp->onNewTriangle = true;
            }else
            {
                ballInn.mBallPhysicsComp->onNewTriangle = false;
            }

            QVector3D aVec{9.81f*ballInn.mBallPhysicsComp->normal.x()*ballInn.mBallPhysicsComp->normal.y(),9.81f*((ballInn.mBallPhysicsComp->normal.y()*ballInn.mBallPhysicsComp->normal.y())-1),9.81f*ballInn.mBallPhysicsComp->normal.y()*ballInn.mBallPhysicsComp->normal.z()};
//            QVector3D aVec = (1.f /ballInn.mBallPhysicsComp->Mass) *
//                    ((ballInn.mBallPhysicsComp->Mass*9.81f*ballInn.mBallPhysicsComp->normal*ballInn.mBallPhysicsComp->normal.y())
//                     + ballInn.mBallPhysicsComp->Mass * QVector3D{0,-9.81f,0});
//            QVector3D aVec{0.3f,0,0};

            if(ballInn.mBallPhysicsComp->isInAir)
                return QVector3D{0,-1,0};
            else
                return aVec;
        }
    }else{
//        qDebug() << "Ball is out of bounds!";
    }
}

void PhysicsBallSystem::resetBall()
{
//    mVelocity = {0,0,0};
//    mPosition = {0,0,0};
//    collisionVector = {0,0,0};
//    lastNormal  = {0,0,0};
//    currentNormal  = {0,0,0};
//    normalLastAndCurrentQ.pop();
//    normalLastAndCurrentQ.pop();
//    onNewTriangle = false;
//    id = 0;
}

//GameObject *PhysicsBallSystem::GetTerrain()
//{
//    return terrain;
//}
