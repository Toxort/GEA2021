#include "meshhandler.h"

#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <qvector3d.h>
#include <qvector2d.h>
#include <vertex.h>
#include <algorithm>

#include "components.h"

//For ObjMesh bruker vi Ole sin kode, og QVector3D/2D istedenfor egen vector klasse. Jeg ser ingen grunn til å lage en egen obj import klasse,
//da denne er helt fin.
// Forskjellen ligger i at vår visualobject klassse er såppass annerledes at denne nå er standalone.

MeshHandler::MeshHandler()
{

}

MeshHandler::~MeshHandler()
{

}

void MeshHandler::init(MeshComponent &MeshComp, int LODlvl)
{
        //must call this to use OpenGL functions
        initializeOpenGLFunctions();

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        //Vertex Array Object - VAO
        glGenVertexArrays( 1, &MeshComp.mVAO[LODlvl] );
        glBindVertexArray( MeshComp.mVAO[LODlvl] );

        //Vertex Buffer Object to hold vertices - VBO
        glGenBuffers( 1, &MeshComp.mVBO[LODlvl] );
        glBindBuffer( GL_ARRAY_BUFFER, MeshComp.mVBO[LODlvl] );

        glBufferData( GL_ARRAY_BUFFER, MeshComp.mVertices[LODlvl].size()*sizeof(Vertex), MeshComp.mVertices[LODlvl].data(), GL_STATIC_DRAW );

        // 1rst attribute buffer : position
        glVertexAttribPointer(0, 3, GL_FLOAT,GL_FALSE, sizeof(Vertex), (GLvoid*)0);
        glEnableVertexAttribArray(0);

        // 2nd attribute buffer : normal
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,  sizeof(Vertex),  (GLvoid*)(3 * sizeof(GLfloat)) );
        glEnableVertexAttribArray(1);

        // 3rd attribute buffer : uvs
        glVertexAttribPointer(2, 2,  GL_FLOAT, GL_FALSE, sizeof( Vertex ), (GLvoid*)( 6 * sizeof( GLfloat ) ));
        glEnableVertexAttribArray(2);

        //Second buffer - holds the indices (Element Array Buffer - EAB):
        if(MeshComp.mIndices[LODlvl].size() > 0) {
            glGenBuffers(1, &MeshComp.mEAB[LODlvl]);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, MeshComp.mEAB[LODlvl]);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, MeshComp.mIndices[LODlvl].size() * sizeof(GLuint), MeshComp.mIndices[LODlvl].data(), GL_STATIC_DRAW);
        }

        glBindVertexArray(0);
}

void MeshHandler::readFile(std::string filename, MeshComponent *MeshComp, int LODlvl, CollisionComponent *CollComp, MeshComponent *CollLines)
{
    std::string tempFileName{0};
    if(LODlvl == 0)
    {
        tempFileName = filename;
    }else
    {
        filename.erase(filename.find(".obj"));
        tempFileName = filename + "_" + std::to_string(LODlvl);
        tempFileName = tempFileName + ".obj";
    }
    //qDebug() << "HERE filename: " << QString::fromStdString(tempFileName);
    std::ifstream fileIn;
    fileIn.open (tempFileName, std::ifstream::in);
    if(!fileIn)
        qDebug() << "Could not open file for reading: " << QString::fromStdString(tempFileName);

    //One line at a time-variable
    std::string oneLine;
    //One word at a time-variable
    std::string oneWord;

    std::vector<QVector3D> tempVertecies;
    std::vector<QVector3D> tempNormals;
    std::vector<QVector2D> tempUVs;

    //    std::vector<Vertex> mVertices;    //made in VisualObject
    //    std::vector<GLushort> mIndices;   //made in VisualObject

    // Varible for constructing the indices vector
    unsigned int temp_index = 0;

    //Reading one line at a time from file to oneLine
    while(std::getline(fileIn, oneLine))
    {
        //Doing a trick to get one word at a time
        std::stringstream sStream;
        //Pushing line into stream
        sStream << oneLine;
        //Streaming one word out of line
        oneWord = ""; //resetting the value or else the last value might survive!
        sStream >> oneWord;

        if (oneWord == "#")
        {
            //Ignore this line
            //            qDebug() << "Line is comment "  << QString::fromStdString(oneWord);
            continue;
        }
        if (oneWord == "")
        {
            //Ignore this line
            //            qDebug() << "Line is blank ";
            continue;
        }
        if (oneWord == "v")
        {
            //            qDebug() << "Line is vertex "  << QString::fromStdString(oneWord) << " ";
            QVector3D tempVertex;
            sStream >> oneWord;
            tempVertex.setX(std::stof(oneWord));
            sStream >> oneWord;
            tempVertex.setY(std::stof(oneWord));
            sStream >> oneWord;
            tempVertex.setZ(std::stof(oneWord));

            //Vertex made - pushing it into vertex-vector
            tempVertecies.push_back(tempVertex);


            // For making the collision box
            if(LODlvl == 0)
                findCollisionCorners(CollComp, tempVertex);

            continue;
        }
        if (oneWord == "vt")
        {
            //            qDebug() << "Line is UV-coordinate "  << QString::fromStdString(oneWord) << " ";
            QVector2D tempUV;
            sStream >> oneWord;
            tempUV.setX(std::stof(oneWord));
            sStream >> oneWord;
            tempUV.setY(std::stof(oneWord));

            //UV made - pushing it into UV-vector
            tempUVs.push_back(tempUV);

            continue;
        }
        if (oneWord == "vn")
        {
            //            qDebug() << "Line is normal "  << QString::fromStdString(oneWord) << " ";
            QVector3D tempNormal;
            sStream >> oneWord;
            tempNormal.setX(std::stof(oneWord));
            sStream >> oneWord;
            tempNormal.setY(std::stof(oneWord));
            sStream >> oneWord;
            tempNormal.setZ(std::stof(oneWord));

            //Vertex made - pushing it into vertex-vector
            tempNormals.push_back(tempNormal);
            continue;
        }
        if (oneWord == "f")
        {
            //            qDebug() << "Line is a face "  << QString::fromStdString(oneWord) << " ";
            //int slash; //used to get the / from the v/t/n - format
            int index, normal, uv;
            for(int i = 0; i < 3; i++)
            {
                sStream >> oneWord;     //one word read
                std::stringstream tempWord(oneWord);    //to use getline on this one word
                std::string segment;    //the numbers in the f-line
                std::vector<std::string> segmentArray;  //temp array of the numbers
                while(std::getline(tempWord, segment, '/')) //splitting word in segments
                {
                    segmentArray.push_back(segment);
                }
                index = std::stoi(segmentArray[0]);     //first is vertex
                if (segmentArray[1] != "")              //second is uv
                    uv = std::stoi(segmentArray[1]);
                else
                {
                    //qDebug() << "No uvs in mesh";       //uv not present
                    uv = 0;                             //this will become -1 in a couple of lines
                }
                normal = std::stoi(segmentArray[2]);    //third is normal

                //Fixing the indexes
                //because obj f-lines starts with 1, not 0
                --index;
                --uv;
                --normal;

                if (uv > -1)    //uv present!
                {
                    Vertex tempVert(tempVertecies[index], tempNormals[normal], tempUVs[uv]);
                    MeshComp->mVertices[LODlvl].push_back(tempVert);
                }
                else            //no uv in mesh data, use 0, 0 as uv
                {
                    Vertex tempVert(tempVertecies[index], tempNormals[normal], QVector2D(0.0f, 0.0f));
                    MeshComp->mVertices[LODlvl].push_back(tempVert);
                }
                MeshComp->mIndices[LODlvl].push_back(temp_index++);
            }
            continue;
        }
    }
    //beeing a nice boy and closing the file after use
    fileIn.close();

    if(LODlvl == 0)
        makeCollisionBox(CollComp,CollLines);

    init(*MeshComp, LODlvl);
}

void MeshHandler::createCollisionCorners(std::string filename, CollisionComponent *CollComp)
{
    std::string tempFileName{0};
        tempFileName = filename;

    //qDebug() << "HERE filename: " << QString::fromStdString(tempFileName);
    std::ifstream fileIn;
    fileIn.open (tempFileName, std::ifstream::in);
    if(!fileIn)
        qDebug() << "Could not open file for reading: " << QString::fromStdString(tempFileName);

    //One line at a time-variable
    std::string oneLine;
    //One word at a time-variable
    std::string oneWord;

    std::vector<QVector3D> tempVertecies;
    std::vector<QVector3D> tempNormals;
    std::vector<QVector2D> tempUVs;

    //    std::vector<Vertex> mVertices;    //made in VisualObject
    //    std::vector<GLushort> mIndices;   //made in VisualObject

    // Varible for constructing the indices vector
    unsigned int temp_index = 0;

    //Reading one line at a time from file to oneLine
    while(std::getline(fileIn, oneLine))
    {
        //Doing a trick to get one word at a time
        std::stringstream sStream;
        //Pushing line into stream
        sStream << oneLine;
        //Streaming one word out of line
        oneWord = ""; //resetting the value or else the last value might survive!
        sStream >> oneWord;

        if (oneWord == "#")
        {
            //Ignore this line
            //            qDebug() << "Line is comment "  << QString::fromStdString(oneWord);
            continue;
        }
        if (oneWord == "")
        {
            //Ignore this line
            //            qDebug() << "Line is blank ";
            continue;
        }
        if (oneWord == "v")
        {
            //            qDebug() << "Line is vertex "  << QString::fromStdString(oneWord) << " ";
            QVector3D tempVertex;
            sStream >> oneWord;
            tempVertex.setX(std::stof(oneWord));
            sStream >> oneWord;
            tempVertex.setY(std::stof(oneWord));
            sStream >> oneWord;
            tempVertex.setZ(std::stof(oneWord));

            //Vertex made - pushing it into vertex-vector
            tempVertecies.push_back(tempVertex);


            // For making the collision box

            findCollisionCorners(CollComp, tempVertex);

            continue;
        }
    }
    //beeing a nice boy and closing the file after use
    fileIn.close();
}

void MeshHandler::createXYZAxis(MeshComponent *MeshComp, CollisionComponent *CollisionComponent, MeshComponent *CollLines)
{
    MeshComp->mVertices[0].push_back(Vertex{0,0,0,1,0,0});
    MeshComp->mVertices[0].push_back(Vertex{3,0,0,1,0,0});
    MeshComp->mVertices[0].push_back(Vertex{0,0,0,0,1,0});
    MeshComp->mVertices[0].push_back(Vertex{0,3,0,0,1,0});
    MeshComp->mVertices[0].push_back(Vertex{0,0,0,0,0,1});
    MeshComp->mVertices[0].push_back(Vertex{0,0,3,0,0,1});

//    MeshComp->mIndices[0].insert( CollLines->mIndices[0].end(),
//    { 0,1,0,2,0,3});
    MeshComp->mDrawType = GL_LINES;

    init(*MeshComp, 0);
    //Min
        CollisionComponent->minCorner.setX(0.f);
        CollisionComponent->minCorner.setY(0.f);
        CollisionComponent->minCorner.setZ(0.f);

        CollisionComponent->maxCorner.setX(0.f);
        CollisionComponent->maxCorner.setY(0.f);
        CollisionComponent->maxCorner.setZ(0.f);

        CollisionComponent->mRaidus = 0.f;


        CollLines->mVertices->push_back(Vertex(0,0,0, 0,0,0,  0,0));
        CollLines->mVertices->push_back(Vertex(0,0,0, 0,0,0,  0,0));
        CollLines->mVertices->push_back(Vertex(0,0,0, 0,0,0,  0,0));
        CollLines->mVertices->push_back(Vertex(0,0,0, 0,0,0,  0,0));

        CollLines->mVertices->push_back(Vertex(0,0,0, 0,0,0,  0,0));
        CollLines->mVertices->push_back(Vertex(0,0,0, 0,0,0,  0,0));
        CollLines->mVertices->push_back(Vertex(0,0,0, 0,0,0,  0,0));
        CollLines->mVertices->push_back(Vertex(0,0,0, 0,0,0,  0,0));

        CollLines->mIndices[0].insert( CollLines->mIndices[0].end(),
        { 0, 1, 1, 2, 2, 3, 3, 0,
          4, 5, 5, 6, 6, 7, 7, 4,
          0, 4, 3, 7,
          1, 5, 2, 6
                         });

        CollLines->mDrawType = GL_LINES;

        init(*CollLines,0);
}

void MeshHandler::createTerrain(std::string filename, MeshComponent *MeshComp, CollisionComponent *CollisionComponent, MeshComponent *CollLines)
{

    std::vector<gsl::Vector3D> mAllDataPoints;

    float x,y,z;
    float xMin;
    float xMax;
    float zMin;
    float zMax;
    float yMin;
    float yMax;
    unsigned int n;


    std::ifstream inn;
    inn.open(filename.c_str());

    if(inn.is_open())
    {
        gsl::Vector3D pos;
        inn>>n;
        int loopLenth = n;
//        float i{0};
        mAllDataPoints.reserve(n);

        for(int i{0}; i<loopLenth;i++)
        {
            inn >> x;
            inn >> z;
            inn >> y;

            pos.setX(x);
            pos.setZ(z);
            pos.setY(y);

            mAllDataPoints.push_back(pos);
            // initielle min maks verider
            static bool once = true;
            if(once)
            {
                xMin = x;
                xMax = x;
                zMin = z;
                zMax = z;
                yMin = y;
                yMax = y;
                once = false;
            }
            // setter min maks
            if(x < xMin)
                xMin = x;

            if(x > xMax)
                xMax = x;

            if(z < zMin)
                zMin = z;

            if(z > zMax)
                zMax = z;

            if(y < yMin)
                yMin = y;

            if(y > yMax)
                yMax = y;

        }
    inn.close();
        // skalerer det ned til slik at flatern begynner i 0,0,0
        for(unsigned long long i = 0; i < loopLenth;i++)
        {
            mAllDataPoints[i].setX(mAllDataPoints[i].getX() - xMin);
            mAllDataPoints[i].setZ(mAllDataPoints[i].getZ() - zMin);
            mAllDataPoints[i].setY(mAllDataPoints[i].getY() - yMin);
        }
    }
    xMax = xMax-xMin;
    xMin = xMin-xMin;
    zMax = zMax-zMin;
    zMin = zMin-zMin;
    yMax = yMax-yMin;
    yMin = yMin-yMin;

    // setup for game objectet (ikke relevant for Mappen)...
    CollisionComponent->minCorner.setX(xMin);
    CollisionComponent->minCorner.setY(yMin);
    CollisionComponent->minCorner.setZ(zMin);

    CollisionComponent->maxCorner.setX(xMax);
    CollisionComponent->maxCorner.setY(yMax);
    CollisionComponent->maxCorner.setZ(zMax);

    // is only used for frustum culling so doesnt have to accurate.
    CollisionComponent->mRaidus = xMax*zMax;

    CollLines->mVertices[0].push_back(Vertex(xMin, yMin, zMax,   1,0,0,  0,0));
    CollLines->mVertices[0].push_back(Vertex(xMax, yMin, zMax,   1,0,0,  0,0));
    CollLines->mVertices[0].push_back(Vertex(xMax, yMax, zMax,   1,0,0,  0,0));
    CollLines->mVertices[0].push_back(Vertex(xMin, yMax, zMax,   1,0,0,  0,0));

    CollLines->mVertices[0].push_back(Vertex(xMin, yMin, zMin,   1,0,0,  0,0));
    CollLines->mVertices[0].push_back(Vertex(xMax, yMin, zMin,   1,0,0,  0,0));
    CollLines->mVertices[0].push_back(Vertex(xMax, yMax, zMin,   1,0,0,  0,0));
    CollLines->mVertices[0].push_back(Vertex(xMin, yMax, zMin,   1,0,0,  0,0));

    CollLines->mIndices[0].insert( CollLines->mIndices[0].end(),
    { 0, 1, 1, 2, 2, 3, 3, 0,
      4, 5, 5, 6, 6, 7, 7, 4,
      0, 4, 3, 7,
      1, 5, 2, 6 });

    CollLines->mDrawType = GL_LINES;
    init(*CollLines,0);
// ...setup ferdig


    int gridSize {10};
    int TerrainWidth = 1000;
    int TerrainLenght = 1460;
    const int cols = TerrainWidth / gridSize;
    const int rows = TerrainLenght / gridSize;

    // sorterer etter z slik at søkingen skal gå fortere.
    std::sort(mAllDataPoints.begin(), mAllDataPoints.end(), lessThanZ());

    float xtemp;
    float ztemp;
    for(int z = int(zMin); z < TerrainLenght;z+=gridSize)
    {
        for(int x = TerrainWidth;x > xMin; x-=gridSize)
        {
            std::vector<float> allHeightsInSquare;
            for(auto it : mAllDataPoints)
            {
                //siden den er sorter etter z så kan jeg slippe å søke de radene jeg allerde har søkt igjennom.
                if(it.z > z+gridSize)
                {
                    break;
                }
                // om høydeverdien er innenfor ruten jeg har satt av.
                if(it.x > x-gridSize && it.x < x && it.z > z && it.z < z + gridSize)
                {
                    allHeightsInSquare.push_back(it.y);
                }
            }
            if(!allHeightsInSquare.empty())
            {
                float averageHeight{0};
                // utregning av gjennomsittshøyden
                for(auto it: allHeightsInSquare)
                {
                    averageHeight += it;
                }
                averageHeight = averageHeight / allHeightsInSquare.size();
                MeshComp->mVertices[0].push_back(Vertex(x,averageHeight,z, 0,1,0,  0,0));
                allHeightsInSquare.clear();
                averageHeight = 0;
            }
            else if(!MeshComp->mVertices->empty())
            {
                float lastHeight = MeshComp->mVertices[0].at(MeshComp->mVertices->size()-1).mXYZ.getY();
                MeshComp->mVertices[0].push_back(Vertex(x,lastHeight,z, 0,1,0,  0,0));

                continue;
            }else
            {
                MeshComp->mVertices[0].push_back(Vertex(x,0,z, 0,1,0,  0,0));
                continue;
            }
        }
    }

    int c{0};
    int nrOfTerrainVertices = MeshComp->mVertices[0].size();
    for(GLuint i{}; i < nrOfTerrainVertices - (TerrainWidth/gridSize) - 1; i++)
    {
        if(c == (TerrainWidth/gridSize) - 1) // for å ungå å tenge en trekant fra ene siden til andre siden i neste rad.
        {
            c=0;
            continue;
        }
        MeshComp->mIndices->push_back(i);
        MeshComp->mIndices->push_back(i+1);
        MeshComp->mIndices->push_back(i+int(TerrainWidth/gridSize));

        MeshComp->mIndices->push_back(i + int(TerrainWidth/gridSize));
        MeshComp->mIndices->push_back(i+1);
        MeshComp->mIndices->push_back(i+(int(TerrainWidth/gridSize))+1);
        c++;
    }

MeshComp->mDrawType = GL_TRIANGLES;
//glPointSize(5.f);


//Normals:
QVector3D pCenter,p0,p1,p2,p3,p4,p5;
QVector3D n0,n1,n2,n3,n4,n5;

for(int i = 1;i < MeshComp->mVertices[0].size() - (TerrainWidth/gridSize);i++)
{

    //Får tak i alle punktene som trengs
    if(MeshComp->mVertices[0].at(i).mXYZ.x >0 && MeshComp->mVertices[0].at(i).mXYZ.z > 0 && MeshComp->mVertices[0].at(i).mXYZ.x < xMax - (TerrainWidth/gridSize)/9  && MeshComp->mVertices[0].at(i).mXYZ.z < zMax - (TerrainWidth/gridSize)/5)
    {
        pCenter = QVector3D{MeshComp->mVertices[0].at(i).mXYZ.x,MeshComp->mVertices[0].at(i).mXYZ.y,MeshComp->mVertices[0].at(i).mXYZ.z};

        //p0
        p0 = QVector3D{MeshComp->mVertices[0].at(i-(TerrainWidth/gridSize)).mXYZ.x,MeshComp->mVertices[0].at(i-(TerrainWidth/gridSize)).mXYZ.y,MeshComp->mVertices[0].at(i-(TerrainWidth/gridSize)).mXYZ.z};

        //p1
        p1 = QVector3D{MeshComp->mVertices[0].at(i+1).mXYZ.x,MeshComp->mVertices[0].at(i+1).mXYZ.y,MeshComp->mVertices[0].at(i+1).mXYZ.z};

        //p2
        p2 = QVector3D{MeshComp->mVertices[0].at(i+(TerrainWidth/gridSize)+1).mXYZ.x,MeshComp->mVertices[0].at(i+(TerrainWidth/gridSize)+1).mXYZ.y,MeshComp->mVertices[0].at(i+(TerrainWidth/gridSize)+1).mXYZ.z};

        //p3
        p3 = QVector3D{MeshComp->mVertices[0].at(i+(TerrainWidth/gridSize)).mXYZ.x,MeshComp->mVertices[0].at(i+(TerrainWidth/gridSize)).mXYZ.y,MeshComp->mVertices[0].at(i+(TerrainWidth/gridSize)).mXYZ.z};

        //p4
        p4 = QVector3D{MeshComp->mVertices[0].at(i-1).mXYZ.x,MeshComp->mVertices[0].at(i-1).mXYZ.y,MeshComp->mVertices[0].at(i-1).mXYZ.z};

        //p5
        p5 = QVector3D{MeshComp->mVertices[0].at(i-(TerrainWidth/gridSize)-1).mXYZ.x,MeshComp->mVertices[0].at(i-(TerrainWidth/gridSize)-1).mXYZ.y,MeshComp->mVertices[0].at(i-(TerrainWidth/gridSize)-1).mXYZ.z};
    }

    //lager vektorer til alle punktene
    QVector3D v0 = p0-pCenter;
    QVector3D v1 = p1-pCenter;
    QVector3D v2 = p2-pCenter;
    QVector3D v3 = p3-pCenter;
    QVector3D v4 = p4-pCenter;
    QVector3D v5 = p5-pCenter;
    QVector3D nV;

    //Regner ut normalene til alle trekantene rundt punktet
    //n0
    n0 = QVector3D::crossProduct(v0,v1);
    n0.normalize();
    //n1
    n1 = QVector3D::crossProduct(v1,v2);
    n1.normalize();
    //n2
    n2 = QVector3D::crossProduct(v2,v3);
    n2.normalize();
    //n3
    n3 = QVector3D::crossProduct(v3,v4);
    n3.normalize();
    //n4
    n4 = QVector3D::crossProduct(v4,v5);
    n4.normalize();
    //n5
    n5 = QVector3D::crossProduct(v5,v0);
    n5.normalize();

    nV = n0+n1+n2+n3+n4+n5;
    nV.normalize();

    MeshComp->mVertices[0].at(i).set_normal(nV.x(),nV.y(),nV.z());
}


terrainMesh = MeshComp;

init(*MeshComp,0);


}

void MeshHandler::createHeightCurves(MeshComponent *MeshComp, CollisionComponent *CollisionComponent, MeshComponent *CollLines)
{
    // ----- Setup -----

    CollisionComponent->minCorner.setX(0);
    CollisionComponent->minCorner.setY(0);
    CollisionComponent->minCorner.setZ(0);

    CollisionComponent->maxCorner.setX(1);
    CollisionComponent->maxCorner.setY(1);
    CollisionComponent->maxCorner.setZ(1);

    // is only used for frustum culling so doesnt have to accurate.
    CollisionComponent->mRaidus = 1000000;


    CollLines->mVertices[0].push_back(Vertex(0,0,0,   1,0,0,  0,0));
    CollLines->mVertices[0].push_back(Vertex(0,0,0,   1,0,0,  0,0));
    CollLines->mVertices[0].push_back(Vertex(0,0,0,   1,0,0,  0,0));
    CollLines->mVertices[0].push_back(Vertex(0,0,0,   1,0,0,  0,0));

    CollLines->mVertices[0].push_back(Vertex(0,0,0,   1,0,0,  0,0));
    CollLines->mVertices[0].push_back(Vertex(0,0,0,   1,0,0,  0,0));
    CollLines->mVertices[0].push_back(Vertex(0,0,0,   1,0,0,  0,0));
    CollLines->mVertices[0].push_back(Vertex(0,0,0,   1,0,0,  0,0));

    CollLines->mIndices[0].insert( CollLines->mIndices[0].end(),
    { 0, 1, 1, 2, 2, 3, 3, 0,
      4, 5, 5, 6, 6, 7, 7, 4,
      0, 4, 3, 7,
      1, 5, 2, 6 });

    CollLines->mDrawType = GL_LINES;
    init(*CollLines,0); 
    // ----------




    // ----- Høydekurver -----

//      p2--p3
//      |  /|
//      | / |
//      |/  |
//      p0--p1

    int gridSize {10};
    float TerrainWidth = 1000;
    float TerrainLenght = 1460;
    float xMax = 997;
    float zMax = 1457;
    float yMax = 95;
    float ekv = 5.f;
    int c {0};

    for(float h {0}; h < yMax; h += ekv)
    {
        for(int i = 0;i < terrainMesh->mVertices[0].size() - (TerrainWidth/gridSize) ;i++)
        {
            if(c == (TerrainWidth/gridSize) -1) // for not including the last square in the row
            {
                c=0;
                continue;
            }
            gsl::Vector2D p0 {terrainMesh->mVertices[0].at(i).mXYZ.getX(),terrainMesh->mVertices[0].at(i).mXYZ.y};
            gsl::Vector2D p1 {terrainMesh->mVertices[0].at(i+1).mXYZ.getX(),terrainMesh->mVertices[0].at(i+1).mXYZ.getY()};
            gsl::Vector2D p2 {terrainMesh->mVertices[0].at(i+int(TerrainWidth/gridSize)).mXYZ.getX(),terrainMesh->mVertices[0].at(i+int(TerrainWidth/gridSize)).mXYZ.getY()};
            gsl::Vector2D p3 {terrainMesh->mVertices[0].at(i+(int(TerrainWidth/gridSize))+1).mXYZ.getX(),terrainMesh->mVertices[0].at(i+(int(TerrainWidth/gridSize))+1).mXYZ.getY()};

            // Lower triangle
            if(p0.y < h && p1.y > h)
            {
                //utregning av posisjon langs de to punktene i forhold til p0
                float a = h - p0.y;
                float b = p1.y - p0.y;
                float pros = a/b;
                float posx = pros*gridSize;
                MeshComp->mVertices[0].push_back(Vertex(terrainMesh->mVertices[0].at(i).mXYZ.x  - posx, h, terrainMesh->mVertices[0].at(i).mXYZ.z, 0,1,0,  0,0));
            }
            else if (p0.y > h && p1.y < h)
            {
                // samme utregning for de andre bare i forhold til relevant punkt.
                float a = h - p1.y;
                float b = p0.y - p1.y;
                float pros = a/b;
                float posx = (1-pros)* gridSize;
                MeshComp->mVertices[0].push_back(Vertex(terrainMesh->mVertices[0].at(i).mXYZ.x - posx, h, terrainMesh->mVertices[0].at(i).mXYZ.z, 0,1,0,  0,0));
            }

            if(p0.y < h && p2.y > h)
            {
                float a = h - p0.y;
                float b = p2.y - p0.y;
                float pros = a/b;
                float posz = pros*gridSize;
                MeshComp->mVertices[0].push_back(Vertex(terrainMesh->mVertices[0].at(i).mXYZ.x, h, terrainMesh->mVertices[0].at(i).mXYZ.z + posz, 0,1,0,  0,0));
            }
            else if (p0.y > h && p2.y < h)
            {
                float a = h - p0.y;
                float b = p0.y - p2.y;
                float pros = a/b;
                float posz = (pros)*gridSize;
                MeshComp->mVertices[0].push_back(Vertex(terrainMesh->mVertices[0].at(i).mXYZ.x, h, terrainMesh->mVertices[0].at(i).mXYZ.z - posz, 0,1,0,  0,0));
            }

            if(p1.y < h && p2.y > h)
            {
                float a = h - p1.y;
                float b = p2.y - p1.y;
                float pros = a/b;
                float posxz = (pros)*gridSize;
                MeshComp->mVertices[0].push_back(Vertex(terrainMesh->mVertices[0].at(i+1).mXYZ.x + posxz, h, terrainMesh->mVertices[0].at(i+1).mXYZ.z + posxz , 0,1,0,  0,0));
            }
            else if (p1.y > h && p2.y < h)
            {
                float a = h - p1.y;
                float b = p1.y - p2.y;
                float pros = a/b;
                float posxz = (pros)*gridSize;
                MeshComp->mVertices[0].push_back(Vertex(terrainMesh->mVertices[0].at(i+1).mXYZ.x - posxz, h, terrainMesh->mVertices[0].at(i+1).mXYZ.z - posxz, 0,1,0,  0,0));
            }


//             Upper trianlge

            if(p2.y < h && p3.y > h)
            {
                float a = h - p2.y;
                float b = p3.y - p2.y;
                float pros = a/b;
                float posx = pros*gridSize;
                MeshComp->mVertices[0].push_back(Vertex(terrainMesh->mVertices[0].at(i+(TerrainWidth/gridSize)).mXYZ.x  - posx, h, terrainMesh->mVertices[0].at(i+100).mXYZ.z, 0,1,0,  0,0));
            }
            else if (p2.y > h && p3.y < h)
            {
                float a = h - p3.y;
                float b = p2.y - p3.y;
                float pros = a/b;
                float posx = (1-pros)* gridSize;
                MeshComp->mVertices[0].push_back(Vertex(terrainMesh->mVertices[0].at(i+(TerrainWidth/gridSize)).mXYZ.x  - posx, h, terrainMesh->mVertices[0].at(i+(TerrainWidth/gridSize)).mXYZ.z, 0,1,0,  0,0));
            }

            if(p1.y < h && p3.y > h)
            {
                float a = h - p1.y;
                float b = p3.y - p1.y;
                float pros = a/b;
                float posz = pros*gridSize;
                MeshComp->mVertices[0].push_back(Vertex(terrainMesh->mVertices[0].at(i+1).mXYZ.x, h, terrainMesh->mVertices[0].at(i+1).mXYZ.z + posz, 0,1,0,  0,0));
            }
            else if (p1.y > h && p3.y < h)
            {
                float a = h - p1.y;
                float b = p1.y - p3.y;
                float pros = a/b;
                float posz = (pros)*gridSize;
                MeshComp->mVertices[0].push_back(Vertex(terrainMesh->mVertices[0].at(i+1).mXYZ.x, h, terrainMesh->mVertices[0].at(i+1).mXYZ.z - posz, 0,1,0,  0,0));
            }

            if(p1.y < h && p2.y > h)
            {
                float a = h - p1.y;
                float b = p2.y - p1.y;
                float pros = a/b;
                float posxz = (pros)*gridSize;
                MeshComp->mVertices[0].push_back(Vertex(terrainMesh->mVertices[0].at(i+1).mXYZ.x + posxz, h, terrainMesh->mVertices[0].at(i+1).mXYZ.z + posxz , 0,1,0,  0,0));
            }
            else if (p1.y > h && p2.y < h)
            {
                float a = h - p1.y;
                float b = p1.y - p2.y;
                float pros = a/b;
                float posxz = (pros)*gridSize;
                MeshComp->mVertices[0].push_back(Vertex(terrainMesh->mVertices[0].at(i+1).mXYZ.x - posxz, h, terrainMesh->mVertices[0].at(i+1).mXYZ.z - posxz, 0,1,0,  0,0));
            }
            c++;
        }
    }

    MeshComp->mDrawType = GL_LINES;

    init(*MeshComp,0);
}

void MeshHandler::findCollisionCorners(CollisionComponent *CollComp, QVector3D &vertex)
{
        //Min
        if(vertex.x() < CollComp->minCorner.x())
        {
            CollComp->minCorner.setX(vertex.x());
        }
        if(vertex.y() < CollComp->minCorner.y())
        {
            CollComp->minCorner.setY(vertex.y());
        }
        if(vertex.z() < CollComp->minCorner.z())
        {
            CollComp->minCorner.setZ(vertex.z());
        }

        //Max
        if(vertex.x() > CollComp->maxCorner.x())
        {
            CollComp->maxCorner.setX(vertex.x());
        }
        if(vertex.y() > CollComp->maxCorner.y())
        {
            CollComp->maxCorner.setY(vertex.y());
        }
        if(vertex.z() > CollComp->maxCorner.z())
        {
            CollComp->maxCorner.setZ(vertex.z());
        }

        if(vertex.length() > CollComp->mRaidus)
        {
            CollComp->mRaidus = vertex.length();
        }

}


void MeshHandler::makeCollisionBox(CollisionComponent* CollisionComp, MeshComponent* CollisionLines)
{
    float minX = CollisionComp->minCorner.x();
    float minY = CollisionComp->minCorner.y();
    float minZ = CollisionComp->minCorner.z();
    float maxX = CollisionComp->maxCorner.x();
    float maxY = CollisionComp->maxCorner.y();
    float maxZ = CollisionComp->maxCorner.z();

    CollisionLines->mVertices[0].push_back(Vertex(minX, minY, maxZ,   1,0,0,  0,0));
    CollisionLines->mVertices[0].push_back(Vertex(maxX, minY, maxZ,   1,0,0,  0,0));
    CollisionLines->mVertices[0].push_back(Vertex(maxX, maxY, maxZ,   1,0,0,  0,0));
    CollisionLines->mVertices[0].push_back(Vertex(minX, maxY, maxZ,   1,0,0,  0,0));

    CollisionLines->mVertices[0].push_back(Vertex(minX, minY, minZ,   1,0,0,  0,0));
    CollisionLines->mVertices[0].push_back(Vertex(maxX, minY, minZ,   1,0,0,  0,0));
    CollisionLines->mVertices[0].push_back(Vertex(maxX, maxY, minZ,   1,0,0,  0,0));
    CollisionLines->mVertices[0].push_back(Vertex(minX, maxY, minZ,   1,0,0,  0,0));

//    CollisionLines->mVertices[0].insert( CollisionLines->mVertices[0].end(),
//      {//Vertex data for front points       color                       uv
//       Vertex{minX, minY, maxZ,       1, 0.301, 0.933,          0.f, 0.f},     // 0
//       Vertex{maxX, minY, maxZ,       1, 0.301, 0.933,          0.f, 0.f},
//       Vertex{maxX, maxY, maxZ,       1, 0.301, 0.933,          0.f, 0.f},
//       Vertex{minX, maxY, maxZ,       1, 0.301, 0.933,          0.f, 0.f},
//       //Vertex data for back
//       Vertex{minX, minY, minZ,      1, 0.301, 0.933,          0.f, 0.f},    // 4
//       Vertex{maxX, minY, minZ,      1, 0.301, 0.933,          0.f, 0.f},
//       Vertex{maxX, maxY, minZ,      1, 0.301, 0.933,          0.f, 0.f},
//       Vertex{minX, maxY, minZ,      1, 0.301, 0.933,          0.f, 0.f},
//                      });

    CollisionLines->mIndices[0].insert( CollisionLines->mIndices[0].end(),
    { 0, 1, 1, 2, 2, 3, 3, 0,
      4, 5, 5, 6, 6, 7, 7, 4,
      0, 4, 3, 7,
      1, 5, 2, 6
                     });

    CollisionLines->mDrawType = GL_LINES;

    init(*CollisionLines,0);
}


