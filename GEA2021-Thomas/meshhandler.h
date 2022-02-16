#ifndef MESHHANDLER_H
#define MESHHANDLER_H

#include <QOpenGLFunctions_4_1_Core>

#include "vertex.h"
//#include "material.h"

#include <string>
#include <vector>

#include <QMatrix4x4>

class GameObject;
class MeshComponent;
class CollisionComponent;

/**
  @brief Contains functionality to for creating meshes

 */
class MeshHandler : public QOpenGLFunctions_4_1_Core
{
public:
    MeshHandler();
    ~MeshHandler();

    /** @brief Sets up the VAO, VBO and the buffer attibutes.

      Each mesh has 3VAO and VBOs, one for each level of detail.

       @param MeshComp
       @param LODlvl
     */
    void init(MeshComponent &MeshComp, int LODlvl);

    /** @brief .obj file reader

      Goes through each line in the obj file and stores the vertecies, normals and uv coordinats.
      This function further also sets up the collison box values and the collision box mesh itself.

       @param filename
       @param MeshComp
       @param LODlvl
       @param CollisionComponent
       @param CollLines

       @author This funciton was taken and modified from Ole Flaten https://github.com/Hedmark-University-College-SPIM/GEA2021/tree/ole_experiments
     */
    void readFile(std::string filename, MeshComponent *MeshComp, int LODlvl, CollisionComponent *CollisionComponent, MeshComponent *CollLines);

    /** @brief Specific setup for xyz axis

      sets up teh vertices collision box for XYZ axis

       @param MeshComp
       @param CollisionComponent
       @param CollLines
     */
    void createXYZAxis(MeshComponent *MeshComp, CollisionComponent *CollisionComponent, MeshComponent *CollLines);
    //VisSim stuff...
    void createHeightCurves(MeshComponent *MeshComp, CollisionComponent *CollisionComponent, MeshComponent *CollLines);
    void createTerrain(std::string filename, MeshComponent *MeshComp, CollisionComponent *CollisionComponent, MeshComponent *CollLines);
    void createCollisionCorners(std::string filename, CollisionComponent *CollisionComponent);

private:


    void findCollisionCorners(CollisionComponent *CollComp, QVector3D &vertex);
    void findCollisionRadious(CollisionComponent *CollComp, QVector3D &vertex);
    void makeCollisionBox(CollisionComponent *CollisionComp, MeshComponent* CollisionLines);

    // code from VisSim
    MeshComponent* terrainMesh{nullptr};

    struct lessThanZ
    {
        inline bool operator() (gsl::Vector3D a, gsl::Vector3D b)
        {
            if(a.z < b.z) return true;
            if(a.z > b.z) return false;

            if(a.x < b.x) return true;
            if(a.x > b.x) return false;

            return false;
        }
    };

    struct lessThanX
    {
        inline bool operator() (gsl::Vector3D a, gsl::Vector3D b)
        {
            return (a.x < b.x);
        }
    };

};

#endif // MESHHANDLER_H
