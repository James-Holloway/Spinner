#include "Scene.hpp"

#include <utility>
#include <iostream>
#include "tiny_gltf.h"
#include "Utilities.hpp"
#include "MeshBuilder.hpp"
#include "MeshData/StaticMeshVertex.hpp"
#include "Components/CameraComponent.hpp"

namespace Spinner
{
    Scene::Scene(std::string name) : Name(std::move(name))
    {
        ObjectTree = std::make_shared<Spinner::SceneObject>("Scene root", true);
        SceneBuffer = Buffer::CreateBuffer(sizeof(SceneConstants), vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst, vma::MemoryUsage::eCpuToGpu, 0, true);
    }

    bool Scene::AddObjectToScene(const SceneObject::Pointer &object, SceneObject::Pointer parent)
    {
        if (object == nullptr)
        {
            return false;
        }
        if (parent == nullptr)
        {
            parent = GetObjectTree();
        }

        parent->AddChild(object);
        return true;
    }

    void Scene::Update(uint32_t currentFrame)
    {
        auto sceneConstants = GetSceneConstants();
        // Update scene camera
        auto activeCameraComponent = Components::CameraComponent::GetActiveCameraRawPointer();
        if (activeCameraComponent == nullptr)
        {
            return;
        }
        activeCameraComponent->UpdateSceneConstants(sceneConstants);

        // TODO shader time + delta time

        // Update scene constants
        UpdateSceneConstants(sceneConstants);

        // Update mesh constants
        auto sharedThis = shared_from_this();
        ObjectTree->TraverseActive([&](const SceneObject::Pointer &sceneObject) -> bool
        {
            auto meshComponents = sceneObject->GetComponentRawPointers<Components::MeshComponent>();
            for (auto &meshComponent : meshComponents)
            {
                // Update constant buffer with position
                auto meshConstants = meshComponent->GetMeshConstants();
                meshConstants.Model = sceneObject->GetWorldMatrix();
                meshComponent->UpdateConstantBuffer(meshConstants);

                // Potentially update descriptor sets
                meshComponent->Update(sharedThis, currentFrame);
            }

            return true;
        });
    }

    void Scene::Draw(CommandBuffer::Pointer &commandBuffer)
    {
        // Only draw when active
        if (!IsActive())
            return;

        // Don't draw if no active camera
        auto activeCameraComponent = Components::CameraComponent::GetActiveCameraRawPointer();
        if (activeCameraComponent == nullptr)
        {
            return;
        }

        // TODO transparent pass
        std::vector<Components::MeshComponent *> toDrawOpaque;
        ObjectTree->TraverseActive([&](const SceneObject::Pointer &sceneObject) -> bool
        {
            auto meshComponents = sceneObject->GetComponentRawPointers<Components::MeshComponent>();
            for (auto &meshComponent : meshComponents)
            {
                toDrawOpaque.push_back(meshComponent);
            }

            return true;
        });

        // Opaque pass
        for (auto &meshComponent : toDrawOpaque)
        {
            meshComponent->Draw(commandBuffer);
        }
    }

    struct MeshInformation
    {
        Spinner::MeshBuffer::Pointer MeshBuffer;
        std::string Name;
    };

    static bool DoesMeshHaveAttribute(const tinygltf::Mesh &mesh, const std::string &attribute)
    {
        if (mesh.primitives.empty())
            return false;

        for (auto &primitive : mesh.primitives)
        {
            if (primitive.attributes.contains(attribute))
            {
                return true;
            }
        }

        return false;
    }

    static bool GetGLTFAttribute(const tinygltf::Model &model, const tinygltf::Primitive &primitive, const std::string &attributeName, tinygltf::Accessor &accessor, tinygltf::BufferView &bufferView, tinygltf::Buffer &buffer)
    {
        if (primitive.attributes.empty())
            return false;

        auto attribIter = primitive.attributes.find(attributeName);
        if (attribIter == primitive.attributes.end())
        {
            return false;
        }

        accessor = model.accessors[attribIter->second];
        bufferView = model.bufferViews[accessor.bufferView];
        buffer = model.buffers[bufferView.buffer];

        return true;
    }

    static std::vector<MeshInformation> CreateStaticMeshBuffersFromMesh(const tinygltf::Model &model, const tinygltf::Mesh &mesh)
    {
        tinygltf::Accessor accessor;
        tinygltf::BufferView bufferView;
        tinygltf::Buffer buffer;

        std::vector<MeshInformation> meshes;

        int64_t primitiveIndex = -1;
        for (auto &primitive : mesh.primitives)
        {
            primitiveIndex++;
            std::vector<MeshData::StaticMeshVertex> vertices;
            std::vector<MeshBuffer::IndexType> indices;

            // Position
            size_t vertexCount = 0;
            if (GetGLTFAttribute(model, primitive, "POSITION", accessor, bufferView, buffer))
            {
                vertexCount = accessor.count;

                if (vertices.size() + vertexCount > std::numeric_limits<MeshBuffer::IndexType>::max() - 1)
                {
                    throw std::runtime_error(std::string("Cannot create mesh ") + mesh.name + " as it has too many vertices");
                }

                vertices.resize(vertexCount);

                auto positionBuffer = reinterpret_cast<const float *>(&buffer.data[bufferView.byteOffset + accessor.byteOffset]);
                for (size_t i = 0; i < accessor.count; i++)
                {
                    auto &vertex = vertices[i];

                    glm::vec3 position = {positionBuffer[i * 3 + 0], positionBuffer[i * 3 + 1], positionBuffer[i * 3 + 2]};
                    vertex.Position = position;
                }
            }

            if (vertexCount == 0)
            {
                continue;
            }

            // Indices
            {
                accessor = model.accessors[primitive.indices];

                bufferView = model.bufferViews[accessor.bufferView];
                buffer = model.buffers[bufferView.buffer];

                indices.resize(accessor.count);

                const void *indexBuffer = (&buffer.data[bufferView.byteOffset + accessor.byteOffset]);
                for (size_t i = 0; i < accessor.count; i++)
                {
                    MeshBuffer::IndexType index = 0;

                    switch (accessor.componentType)
                    {
                        case TINYGLTF_COMPONENT_TYPE_BYTE:
                            index = static_cast<MeshBuffer::IndexType>(static_cast<uint8_t>(reinterpret_cast<const int8_t *>(indexBuffer)[i]));
                            break;
                        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
                            index = static_cast<MeshBuffer::IndexType>(reinterpret_cast<const uint8_t *>(indexBuffer)[i]);
                            break;
                        case TINYGLTF_COMPONENT_TYPE_SHORT:
                            index = static_cast<MeshBuffer::IndexType>(reinterpret_cast<const int16_t *>(indexBuffer)[i]);
                            break;
                        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
                            index = static_cast<MeshBuffer::IndexType>(reinterpret_cast<const uint16_t *>(indexBuffer)[i]);
                            break;
                        case TINYGLTF_COMPONENT_TYPE_INT:
                            index = static_cast<MeshBuffer::IndexType>(reinterpret_cast<const int32_t *>(indexBuffer)[i]);
                            break;
                        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
                            index = static_cast<MeshBuffer::IndexType>(reinterpret_cast<const uint32_t *>(indexBuffer)[i]);
                            break;
                    }

                    indices[i] = index;
                }
            }

            // Normal
            if (GetGLTFAttribute(model, primitive, "NORMAL", accessor, bufferView, buffer))
            {
                auto normalBuffer = reinterpret_cast<const float *>(&buffer.data[bufferView.byteOffset + accessor.byteOffset]);
                for (size_t i = 0; i < accessor.count; i++)
                {
                    auto &vertex = vertices[i];

                    glm::vec3 normal = {normalBuffer[i * 3 + 0], normalBuffer[i * 3 + 1], normalBuffer[i * 3 + 2]};
                    vertex.Normal = normal;
                }
            }

            // Tangent
            if (GetGLTFAttribute(model, primitive, "TANGENT", accessor, bufferView, buffer))
            {
                auto tangentBuffer = reinterpret_cast<const float *>(&buffer.data[bufferView.byteOffset + accessor.byteOffset]);
                for (size_t i = 0; i < accessor.count; i++)
                {
                    auto &vertex = vertices[i];

                    glm::vec4 tangent = {tangentBuffer[i * 4 + 0], tangentBuffer[i * 4 + 1], tangentBuffer[i * 4 + 2], tangentBuffer[i * 4 + 3]};
                    vertex.Tangent = tangent;
                }
            }

            // UV
            if (GetGLTFAttribute(model, primitive, "TEXCOORD_0", accessor, bufferView, buffer))
            {
                auto uvBuffer = reinterpret_cast<const float *>(&buffer.data[bufferView.byteOffset + accessor.byteOffset]);
                for (size_t i = 0; i < accessor.count; i++)
                {
                    auto &vertex = vertices[i];

                    glm::vec2 uv = {uvBuffer[i * 2 + 0], uvBuffer[i * 2 + 1]};
                    vertex.UV = uv;
                }
            }

            // Color
            if (GetGLTFAttribute(model, primitive, "COLOR_0", accessor, bufferView, buffer))
            {
                auto colorBuffer = reinterpret_cast<const float *>(&buffer.data[bufferView.byteOffset + accessor.byteOffset]);
                for (size_t i = 0; i < accessor.count; i++)
                {
                    auto &vertex = vertices[i];

                    if (accessor.type == TINYGLTF_TYPE_VEC3)
                    {
                        glm::vec4 color = {colorBuffer[i * 3 + 0], colorBuffer[i * 3 + 1], colorBuffer[i * 3 + 2], 1.0f};
                        vertex.Color = color;
                    }
                    else if (accessor.type == TINYGLTF_TYPE_VEC4)
                    {
                        glm::vec4 color = {colorBuffer[i * 4 + 0], colorBuffer[i * 4 + 1], colorBuffer[i * 4 + 2], colorBuffer[i * 4 + 3]};
                        vertex.Color = color;
                    }
                }
            }

            std::string meshName = "Unnamed Material " + std::to_string(primitiveIndex);
            if (primitive.material >= 0)
            {
                meshName = model.materials.at(primitive.material).name;
            }

            meshes.emplace_back(MeshData::StaticMeshVertex::CreateMeshBuilder().SetIndices(indices).SetVertexData(vertices).Create(), meshName);
        }

        return meshes;
    }

    static std::vector<MeshInformation> CreateSkinnedMeshBuffersFromMesh(const tinygltf::Model &model, const tinygltf::Mesh &mesh)
    {
        throw std::runtime_error("Cannot currently create mesh buffer from a skinned mesh");
    }

    static std::vector<MeshInformation> CreateMeshBuffersFromMesh(const tinygltf::Model &model, const tinygltf::Mesh &mesh)
    {
        if (!DoesMeshHaveAttribute(mesh, "POSITION"))
        {
            throw std::runtime_error("Cannot create mesh buffer from mesh without any POSITION element");
        }

        // Skinned mesh
        if (DoesMeshHaveAttribute(mesh, "WEIGHTS_0"))
        {
            return CreateSkinnedMeshBuffersFromMesh(model, mesh);
        }
        // else Static mesh
        return CreateStaticMeshBuffersFromMesh(model, mesh);
    }

    static SceneObject::Pointer CreateSceneObjectFromMesh(const tinygltf::Model &model, const tinygltf::Mesh &mesh, const std::string &nodeName)
    {
        auto sceneObject = std::make_shared<SceneObject>(nodeName);

        auto meshes = CreateMeshBuffersFromMesh(model, mesh);

        for (auto &meshInformation : meshes)
        {
            auto meshBuffer = meshInformation.MeshBuffer;
            auto meshName = meshInformation.Name;
            auto meshComponent = sceneObject->AddComponent<Components::MeshComponent>();
            meshComponent->SetMeshBuffer(meshBuffer);
            meshComponent->PopulateFromShaders(MeshData::StaticMeshVertex::VertexShader, MeshData::StaticMeshVertex::FragmentShader, MeshData::StaticMeshVertex::DescriptorPool);
            meshComponent->SetComponentName(meshName);
        }

        return sceneObject;
    }

    static SceneObject::Pointer CreateEmptySceneObject(const std::string &nodeName)
    {
        return std::make_shared<SceneObject>(nodeName);
    }

    static SceneObject::Pointer CreateSceneObjectFromNode(const tinygltf::Model &model, const int nodeId)
    {
        auto &node = model.nodes.at(nodeId);

        SceneObject::Pointer sceneObject = nullptr;

        try
        {
            // Current node creation
            if (node.mesh >= 0)
            {
                auto &mesh = model.meshes.at(node.mesh);
                sceneObject = CreateSceneObjectFromMesh(model, mesh, node.name);
            }
            else
            {
                sceneObject = CreateEmptySceneObject(node.name);
            }

            // Don't try child nodes if this node failed
            if (sceneObject == nullptr)
            {
                return nullptr;
            }

            // Set position, rotation and scale
            if (!node.translation.empty())
            {
                sceneObject->SetLocalPosition(glm::vec3(static_cast<float>(node.translation[0]), static_cast<float>(node.translation[1]), static_cast<float>(node.translation[2])));
            }
            if (!node.rotation.empty())
            {
                sceneObject->SetLocalRotation(glm::quat(static_cast<float>(node.rotation[0]), static_cast<float>(node.rotation[1]), static_cast<float>(node.rotation[2]), static_cast<float>(node.rotation[3])));
            }
            if (!node.scale.empty())
            {
                sceneObject->SetLocalScale(glm::vec3(static_cast<float>(node.scale[0]), static_cast<float>(node.scale[1]), static_cast<float>(node.scale[2])));
            }

            // Child nodes
            for (auto childId : node.children)
            {
                auto child = CreateSceneObjectFromNode(model, childId);
                if (child != nullptr)
                {
                    sceneObject->AddChild(child);
                }
            }

        }
        catch (const std::exception &ex)
        {
            std::string modelName = model.scenes.at(model.defaultScene).name;
            std::cerr << "Error while loading GLTF model " << modelName << " at node " << node.name << ": " << ex.what() << '\n';
        }

        return sceneObject;
    }

    static SceneObject::Pointer CreateSceneObjectFromScene(const tinygltf::Model &model, const tinygltf::Scene &scene)
    {
        if (scene.nodes.empty())
        {
            return nullptr;
        }
        auto sceneObject = std::make_shared<SceneObject>(scene.name);

        for (auto node : scene.nodes)
        {
            auto nodeObject = CreateSceneObjectFromNode(model, node);
            if (nodeObject == nullptr)
            {
                continue;
            }

            sceneObject->AddChild(nodeObject);
        }

        return sceneObject;
    }

    SceneObject::Pointer Scene::LoadModel(const std::string &modelFilename)
    {
        std::string assetPath = GetAssetPath(AssetType::Model, modelFilename);

        if (!FileExists(assetPath))
        {
            throw std::runtime_error("Cannot load model from asset path " + assetPath + " as file does not exist");
        }

        auto extension = std::filesystem::path(assetPath).extension().string();
        std::transform(extension.begin(), extension.end(), extension.begin(), [](unsigned char c) -> unsigned char
        { return std::tolower(c); });

        tinygltf::TinyGLTF loader;
        tinygltf::Model model;
        std::string err;
        std::string warn;

        bool ret = false;
        if (extension == ".glb")
        {
            ret = loader.LoadBinaryFromFile(&model, &err, &warn, assetPath);
        }
        else if (extension == ".gltf")
        {
            ret = loader.LoadASCIIFromFile(&model, &err, &warn, assetPath);
        }

        if (!warn.empty())
        {
            std::cerr << "Model load warning for " << modelFilename << ": " << warn << '\n';
        }

        if (!err.empty())
        {
            std::cerr << "Model load error for " << modelFilename << ": " << err << '\n';
        }

        if (!ret)
        {
            throw std::runtime_error("Unable to parse binary GLTF " + modelFilename + " ");
        }

        // If multiple scenes
        if (model.scenes.size() > 1)
        {
            std::string fileName = std::filesystem::path(assetPath).filename().replace_extension().string();
            SceneObject::Pointer fileObject = std::make_shared<SceneObject>(fileName);
            for (auto &scene : model.scenes)
            {
                auto sceneObject = CreateSceneObjectFromScene(model, scene);
                if (sceneObject == nullptr)
                {
                    continue;
                }

                fileObject->AddChild(sceneObject);
            }
            return fileObject;
        }

        // If no default scene (somehow) then return nullptr
        if (model.defaultScene < 0)
        {
            return nullptr;
        }

        // If only one scene then create it
        return CreateSceneObjectFromScene(model, model.scenes.at(model.defaultScene));
    }

    bool Scene::IsActive() const noexcept
    {
        return Active;
    }

    void Scene::SetActive(bool active)
    {
        Active = active;
    }

    Buffer::Pointer Scene::GetSceneBuffer()
    {
        return SceneBuffer;
    }

    SceneConstants Scene::GetSceneConstants() const noexcept
    {
        return LocalSceneBuffer;
    }

    void Scene::UpdateSceneConstants(const SceneConstants &sceneConstants) noexcept
    {
        LocalSceneBuffer = sceneConstants;
        SceneBuffer->Write<SceneConstants>(LocalSceneBuffer);
    }

    SceneObject::Pointer Scene::GetObjectTree()
    {
        if (!HasSetObjectTreeScene)
        {
            ObjectTree->SceneParent = weak_from_this();
            HasSetObjectTreeScene = true;
        }

        return ObjectTree;
    }

} // Spinner