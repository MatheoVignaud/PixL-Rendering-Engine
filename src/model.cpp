#include <model.hpp>

Mesh::Mesh(SDL_GPUDevice *device, std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures)
{
    this->vertices = vertices;
    this->indices = indices;
    this->textures = textures;
    setupMesh(device);
}

void Mesh::Bind(SDL_GPURenderPass *renderPass, uint32_t binding)
{
    VBO.Bind(renderPass, binding);
    EBO.Bind(renderPass, SDL_GPU_INDEXELEMENTSIZE_32BIT);
    SDL_BindGPUFragmentSamplers(renderPass, 0, &textures[0].samplerBinding, 1);
    SDL_BindGPUFragmentSamplers(renderPass, 0, &textures[1].samplerBinding, 1);
}

void Mesh::setupMesh(SDL_GPUDevice *device)
{
    // create buffers/arrays
    VBO = CreateVBO(device, vertices.size() * sizeof(Vertex));
    VBO.Update(device, vertices.data(), vertices.size() * sizeof(Vertex));
    VBO.Upload(device);
    EBO = CreateEBO(device, indices.size() * sizeof(unsigned int));
    EBO.Update(device, indices.data(), indices.size() * sizeof(unsigned int));
    EBO.Upload(device);
}

void Model::loadModel(SDL_GPUDevice *device, std::string path)
{
    Assimp::Importer import;
    const aiScene *scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
        return;
    }
    directory = path.substr(0, path.find_last_of('/'));

    processNode(device, scene->mRootNode, scene);
}

void Model::processNode(SDL_GPUDevice *device, aiNode *node, const aiScene *scene)
{
    // process all the node's meshes (if any)
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(processMesh(device, mesh, scene));
    }
    // then do the same for each of its children
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        processNode(device, node->mChildren[i], scene);
    }
}

Mesh Model::processMesh(SDL_GPUDevice *device, aiMesh *mesh, const aiScene *scene)
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;

    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        Vertex vertex;
        vertex.Position.x = mesh->mVertices[i].x;
        vertex.Position.y = mesh->mVertices[i].y;
        vertex.Position.z = mesh->mVertices[i].z;

        if (mesh->HasNormals())
        {
            vertex.Normal.x = mesh->mNormals[i].x;
            vertex.Normal.y = mesh->mNormals[i].y;
            vertex.Normal.z = mesh->mNormals[i].z;
        }
        if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
        {
            vertex.TexCoord.x = mesh->mTextureCoords[0][i].x;
            vertex.TexCoord.y = mesh->mTextureCoords[0][i].y;
        }
        else
            vertex.TexCoord = glm::vec2(0.0f, 0.0f);
        // process vertex positions, normals and texture coordinates
        vertices.push_back(vertex);
    }
    // process indices
    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }
    // process material
    if (mesh->mMaterialIndex >= 0)
    {
        aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];

        std::vector<Texture> diffuseMaps = loadMaterialTextures(device, material, aiTextureType_DIFFUSE, "texture_diffuse");
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
        std::vector<Texture> specularMaps = loadMaterialTextures(device, material, aiTextureType_SPECULAR, "texture_specular");
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
        std::vector<Texture> normalMaps = loadMaterialTextures(device, material, aiTextureType_HEIGHT, "texture_normal");
        textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
        std::vector<Texture> heightMaps = loadMaterialTextures(device, material, aiTextureType_AMBIENT, "texture_height");
        textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());
    }

    return Mesh(device, vertices, indices, textures);
}

std::vector<Texture> Model::loadMaterialTextures(SDL_GPUDevice *device, aiMaterial *mat, aiTextureType type, std::string typeName)
{
    std::vector<Texture> textures;
    for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
    {
        aiString str;
        mat->GetTexture(type, i, &str);
        bool skip = false;
        for (unsigned int j = 0; j < textures_loaded.size(); j++)
        {
            if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0)
            {
                textures.push_back(textures_loaded[j]);
                skip = true;
                break;
            }
        }
        if (!skip)
        {
            std::cout << "Loading texture: " << str.C_Str() << std::endl;
            std::string Path = str.C_Str();
            Texture texture;
            texture.id = textures.size() + 1;
            texture.type = typeName;
            texture.path = str.C_Str();
            texture.samplerBinding = CreateSamplerFromImage(device, Path);
            textures.push_back(texture);
            textures_loaded.push_back(texture); // store it as texture loaded
        }
    }
    return textures;
}
