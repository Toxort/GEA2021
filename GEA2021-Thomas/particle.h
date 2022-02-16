#ifndef PARTICLE_H
#define PARTICLE_H
#include <QOpenGLFunctions_4_1_Core>

#include "vector3d.h"
#include "matrix4x4.h"
#include "vertex.h"

/** @brief Simple quad with variables

  (incomplete)
  Should also have texture support...

 */
class Particle : public QOpenGLFunctions_4_1_Core
{
public:
    Particle();
    /** @brief constructor acts lika a a spawner

      When it is constructed it will be pushed in the std::vector. All of the elements
      in the vector will be renders.

       @param velocity
       @param position
       @param lifeLength
       @see ParticleSystem::addParticle()
     */
    Particle(gsl::Vector3D velocity, gsl::Vector3D position, float lifeLength = 2);
    gsl::Vector3D mVelocity{0,0,0};
    gsl::Vector3D mPosition{0,0,0};
    gsl::Matrix4x4 mMatrix;
    float gravity {-9.81f};
    float lifeLength{2};
    float elapsedTime{0};

    /** @brief Updates the particles position acording to its variables

       @return elapsedTime < lifeLength. Used for deleting the mesh when it should be deleted.
       @see ParticleSystem::update()
     */
    bool update();

    std::vector<Vertex> mVertices;
    std::vector<GLuint> mIndices;

    GLuint mVAO{0};
    GLuint mVBO{0};
    GLuint mEAB{0};

    GLenum mDrawType{GL_TRIANGLES};
private:

    void init();

};

#endif // PARTICLE_H
