#include "particle.h"
#include "constants.h"
#include "transformsystem.h"
#include "particlesystem.h"

Particle::Particle()
{
   mVertices.push_back(Vertex{ -0.5f, -0.5f, 0.0f,   1.0f, 0.0f, 0.0f,  0.0f,  0.0f});
   mVertices.push_back(Vertex{  0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,  1.0f,  0.0f});
   mVertices.push_back(Vertex{  0.5f,  0.5f, 0.0f,   0.0f, 0.0f, 1.0f,  1.0f,  1.0f});

   mVertices.push_back(Vertex{  0.5f,  0.5f, 0.0f,   0.0f, 0.0f, 1.0f,  1.0f,  1.0f});
   mVertices.push_back(Vertex{ -0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,  0.0f,  0.0f});
   mVertices.push_back(Vertex{ -0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,  0.0f,  1.0f});
   mMatrix.setToIdentity();
    init();
    ParticleSystem::addParticle(this);
}

Particle::Particle(gsl::Vector3D velocity, gsl::Vector3D position, float lifeLength)
{
    mMatrix.setToIdentity();
    mPosition = position;
    mVelocity = velocity;
    mMatrix.setPosition(position.x,position.y,position.z);
    mVertices.push_back(Vertex{ -0.3f, -0.3f, 0.0f,   1.0f, 0.6f, 0.0f,  0.0f,  0.0f});
    mVertices.push_back(Vertex{  0.3f, -0.3f, 0.0f,   1.0f, 0.0f, 0.3f,  1.0f,  0.0f});
    mVertices.push_back(Vertex{  0.3f,  0.3f, 0.0f,   1.0f, 0.0f, 0.0f,  1.0f,  1.0f});

    mVertices.push_back(Vertex{  0.3f,  0.3f, 0.0f,   1.0f, 0.6f, 0.0f,  0.0f,  1.0f});
    mVertices.push_back(Vertex{ -0.3f, -0.3f, 0.0f,   1.0f, 0.0f, 0.3f,  0.0f,  0.0f});
    mVertices.push_back(Vertex{ -0.3f,  0.3f, 0.0f,   1.0f, 0.6f, 0.0f,  0.0f,  1.0f});
     init();
     ParticleSystem::addParticle(this);
}

bool Particle::update()
{
    mPosition = mMatrix.getPosition();
    float dt = 0.007;
    mVelocity.y += gsl::Gravity * dt;
    mPosition += mVelocity *dt;
    mMatrix.setPosition(mPosition.x,mPosition.y, mPosition.z);


    elapsedTime += dt;
    return elapsedTime < lifeLength;
}

void Particle::init()
{
    initializeOpenGLFunctions();
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    //Vertex Array Object - VAO
    glGenVertexArrays( 1, &mVAO );
    glBindVertexArray(mVAO );

    //Vertex Buffer Object to hold vertices - VBO
    glGenBuffers( 1, &mVBO );
    glBindBuffer( GL_ARRAY_BUFFER, mVBO );

    glBufferData( GL_ARRAY_BUFFER, mVertices.size()*sizeof(Vertex), mVertices.data(), GL_STATIC_DRAW );

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
    if(mIndices.size() > 0) {
        glGenBuffers(1, &mEAB);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mEAB);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, mIndices.size() * sizeof(GLuint), mIndices.data(), GL_STATIC_DRAW);
    }

    glBindVertexArray(0);
}
