#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <string>

namespace gsl  //Game School Lib
{
    const std::string ProjectFolderName{"../GEA2021/"};
    const std::string AssetFilePath{ProjectFolderName + "Assets/"};
    const std::string TextureFilePath{AssetFilePath + "Textures/"};
    const std::string MeshFilePath{AssetFilePath + "Meshes/"};
    const std::string SoundFilePath{AssetFilePath + "Sounds/"};
    const std::string ShaderFilePath{ProjectFolderName + "Shaders/"};

    enum Shaders
    {
        COLORSHADER = 0,
        TEXTURESHADER,
        PHONGSHADER
    };

    const int NumberOfShaders{4};
    const int NumberOfTextures{16};
    const int NumberOfMaterials{10};
    const float Gravity{-9.81};
}
#endif // CONSTANTS_H
