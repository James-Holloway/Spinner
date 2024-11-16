#include "Scene.hpp"

#include <map>
#include <utility>
#include <iostream>
#include <tiny_gltf.h>
#include <imgui.h>
#include "Utilities.hpp"
#include "MeshBuilder.hpp"
#include "MeshData/StaticMeshVertex.hpp"
#include "Components/Components.hpp"
#include "Material.hpp"
#include "Image.hpp"
#include "Texture.hpp"
#include "Lighting.hpp"
#include "Graphics.hpp"

namespace Spinner
{
    Scene::Scene(std::string name) : Name(std::move(name))
    {
        ObjectTree = std::make_shared<Spinner::SceneObject>("Scene root", true);
        Lighting = GetGlobalLighting();
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

    struct MeshInformation
    {
        Spinner::MeshBuffer::Pointer MeshBuffer;
        std::string Name;
        int MaterialIndex = 0;
    };

    struct SceneInformation
    {
        std::vector<Spinner::Material::Pointer> Materials;
        std::string Warnings;
        size_t NodeIndex = 0;
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

        accessor = model.accessors.at(attribIter->second);
        bufferView = model.bufferViews.at(accessor.bufferView);
        buffer = model.buffers.at(bufferView.buffer);

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

                auto positionBuffer = reinterpret_cast<const float *>(&buffer.data.at(bufferView.byteOffset + accessor.byteOffset));
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
                accessor = model.accessors.at(primitive.indices);

                bufferView = model.bufferViews.at(accessor.bufferView);
                buffer = model.buffers.at(bufferView.buffer);

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

            meshes.emplace_back(MeshData::StaticMeshVertex::CreateMeshBuilder().SetIndices(indices).SetVertexData(vertices).Create(), meshName, primitive.material);
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

    static SceneObject::Pointer CreateSceneObjectFromMesh(const tinygltf::Model &model, const tinygltf::Mesh &mesh, std::string nodeName, SceneInformation &sceneInfo)
    {
        if (nodeName.empty())
        {
            nodeName = mesh.name;
        }
        if (nodeName.empty())
        {
            nodeName = std::string("Mesh ") + std::to_string(sceneInfo.NodeIndex++);
        }

        auto sceneObject = std::make_shared<SceneObject>(nodeName);

        auto meshes = CreateMeshBuffersFromMesh(model, mesh);

        for (auto &meshInformation : meshes)
        {
            auto meshBuffer = meshInformation.MeshBuffer;
            auto meshName = meshInformation.Name;
            auto meshComponent = sceneObject->AddComponent<Components::MeshComponent>();
            meshComponent->SetMeshBuffer(meshBuffer);
            meshComponent->SetShaderGroup(MeshData::StaticMeshVertex::ShaderGroup);
            meshComponent->SetShadowShaderGroup(MeshData::StaticMeshVertex::ShadowShaderGroup);
            meshComponent->SetComponentName(meshName);

            if (meshInformation.MaterialIndex >= 0)
            {
                meshComponent->SetMaterial(sceneInfo.Materials.at(meshInformation.MaterialIndex));
            }
            else
            {
                auto material = Material::CreateMaterial("Unnamed Material");
                meshComponent->SetMaterial(material);
            }
        }

        return sceneObject;
    }

    static SceneObject::Pointer CreateEmptySceneObject(const std::string &nodeName)
    {
        return std::make_shared<SceneObject>(nodeName);
    }

    static SceneObject::Pointer CreateSceneObjectFromLight(const tinygltf::Model &model, const tinygltf::Light &light, std::string nodeName, SceneInformation &sceneInfo)
    {
        if (nodeName.empty())
        {
            nodeName = light.name;
        }
        if (nodeName.empty())
        {
            nodeName = std::string("Light ") + std::to_string(sceneInfo.NodeIndex++);
        }

        auto sceneObject = std::make_shared<SceneObject>(nodeName);
        auto lightComponent = sceneObject->AddComponent<Components::LightComponent>();


        // https://github.com/KhronosGroup/glTF/blob/main/extensions/2.0/Khronos/KHR_lights_punctual/README.md
        if (!light.type.empty())
        {
            if (light.type == "point")
            {
                lightComponent->SetLightType(LightType::Point);
            }
            else if (light.type == "spot")
            {
                lightComponent->SetLightType(LightType::Spot);
            }
            else if (light.type == "directional")
            {
                lightComponent->SetLightType(LightType::Directional);
            }
            else
            {
                lightComponent->SetLightType(LightType::None);
            }
        }

        if (light.color.size() >= 3)
        {
            lightComponent->SetLightColor(glm::vec3(glm::make_vec3(light.color.data())));
        }
        else
        {
            lightComponent->SetLightColor(glm::vec3(1.0f, 1.0f, 1.0f));
        }

        auto intensity = static_cast<float>(light.intensity);
        // Do some dirty conversions from the candela/lux units into Blender's intensity units (1.0)
        if (lightComponent->GetLightType() == LightType::Directional)
        {
            intensity /= 683.0f;
        }
        else
        {
            intensity /= 54.3514f * 1000.0f; // also convert Blender's 1000 intensity down to 1 intensity
        }
        lightComponent->SetLightStrength(intensity);

        // Ignore range

        lightComponent->SetInnerSpotAngle(static_cast<float>(light.spot.innerConeAngle)); // radians, which we like
        lightComponent->SetOuterSpotAngle(static_cast<float>(light.spot.outerConeAngle)); // radians, which we like

        return sceneObject;
    }

    // Recursively calls itself with its children's IDs
    static SceneObject::Pointer CreateSceneObjectFromNode(const tinygltf::Model &model, const int nodeId, SceneInformation &sceneInfo)
    {
        auto &node = model.nodes.at(nodeId);

        SceneObject::Pointer sceneObject = nullptr;

        try
        {
            // Current node creation
            if (node.mesh >= 0)
            {
                auto &mesh = model.meshes.at(node.mesh);
                sceneObject = CreateSceneObjectFromMesh(model, mesh, node.name, sceneInfo);
            }
            else if (node.light >= 0)
            {
                auto &light = model.lights.at(node.light);
                sceneObject = CreateSceneObjectFromLight(model, light, node.name, sceneInfo);
            }
            else
            {
                std::string nodeName = node.name.empty() ? (std::string("Node ") + std::to_string(sceneInfo.NodeIndex++)) : node.name;
                sceneObject = CreateEmptySceneObject(nodeName);
            }

            // Don't try child nodes if this node failed
            if (sceneObject == nullptr)
            {
                sceneInfo.Warnings += "Not adding child nodes of node id " + std::to_string(nodeId) + " \" " + node.name + "\" as this node failed to be created\n";
                return nullptr;
            }

            // Set position, rotation and scale
            if (!node.translation.empty())
            {
                sceneObject->SetLocalPosition(glm::vec3(static_cast<float>(node.translation[0]), static_cast<float>(node.translation[1]), static_cast<float>(node.translation[2])));
            }
            if (!node.rotation.empty())
            {
                auto rotation = glm::quat(static_cast<float>(node.rotation[3]), static_cast<float>(node.rotation[0]), static_cast<float>(node.rotation[1]), static_cast<float>(node.rotation[2]));
                sceneObject->SetLocalRotation(rotation);
            }
            if (!node.scale.empty())
            {
                sceneObject->SetLocalScale(glm::vec3(static_cast<float>(node.scale[0]), static_cast<float>(node.scale[1]), static_cast<float>(node.scale[2])));
            }
            if (!node.matrix.empty())
            {
                sceneObject->SetLocalMatrix(glm::mat4(glm::make_mat4x4(node.matrix.data())));
            }

            // Child nodes
            for (auto childId : node.children)
            {
                auto child = CreateSceneObjectFromNode(model, childId, sceneInfo);
                if (child != nullptr)
                {
                    sceneObject->AddChild(child);
                }
            }
        } catch (const std::exception &ex)
        {
            std::string modelName = model.scenes.at(model.defaultScene).name;
            sceneInfo.Warnings += "Error while loading GLTF model " + modelName + " at node " + node.name + ": " + ex.what() + '\n';
        }

        return sceneObject;
    }

    static SceneObject::Pointer CreateSceneObjectFromScene(const tinygltf::Model &model, const tinygltf::Scene &scene, SceneInformation &sceneInfo)
    {
        if (scene.nodes.empty())
        {
            return nullptr;
        }
        auto sceneObject = std::make_shared<SceneObject>(scene.name);

        for (auto node : scene.nodes)
        {
            auto nodeObject = CreateSceneObjectFromNode(model, node, sceneInfo);
            if (nodeObject == nullptr)
            {
                continue;
            }

            sceneObject->AddChild(nodeObject);
        }

        return sceneObject;
    }

    // Note: will not acknowledge magFilter's setting
    // Note: will not acknowledge wrapT's setting
    static Sampler::Pointer CreateSamplerFromTexture(const tinygltf::Model &model, const tinygltf::Texture &texture)
    {
        // Sampler selecting
        vk::Filter minMagFilter = vk::Filter::eLinear;
        vk::SamplerMipmapMode mipFilter = vk::SamplerMipmapMode::eLinear;
        vk::SamplerAddressMode repeat = vk::SamplerAddressMode::eRepeat;
        if (texture.sampler >= 0)
        {
            auto &colorSampler = model.samplers.at(texture.sampler);
            switch (colorSampler.minFilter)
            {
                case TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_NEAREST:
                    mipFilter = vk::SamplerMipmapMode::eNearest;
                case TINYGLTF_TEXTURE_FILTER_NEAREST:
                case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_NEAREST:
                    minMagFilter = vk::Filter::eNearest;
                    break;
                default:
                    break;
            }

            switch (colorSampler.wrapS)
            {
                case TINYGLTF_TEXTURE_WRAP_CLAMP_TO_EDGE:
                    repeat = vk::SamplerAddressMode::eClampToEdge;
                    break;
                case TINYGLTF_TEXTURE_WRAP_MIRRORED_REPEAT:
                    repeat = vk::SamplerAddressMode::eMirroredRepeat;
                    break;
                default:
                    break;
            }
        }

        return Sampler::CreateSampler(minMagFilter, mipFilter, repeat);
    }

    // Note: loads an invalid texture (magenta) when failing to load the texture
    static Image::Pointer CreateImageFromTexture(const tinygltf::Model &model, const tinygltf::Texture &texture, SceneInformation &sceneInfo)
    {
        if (texture.source < 0 || texture.source >= model.images.size())
        {
            sceneInfo.Warnings += "Texture source " + std::to_string(texture.source) + " was not in the valid range of images\n";
            return nullptr;
        }

        auto image = model.images.at(texture.source);
        if (!image.image.empty())
        {
            if (image.width == 0 || image.height == 0)
            {
                sceneInfo.Warnings += "Could not load a zero dimensioned image from texture \"" + texture.name + "\"\n";
                return nullptr;
            }

            vk::Format format = (image.bits == 16) ? vk::Format::eR16G16B16A16Unorm : vk::Format::eR8G8B8A8Unorm;
            if (image.component == 3)
            {
                format = (image.bits == 16) ? vk::Format::eR16G16B16Unorm : vk::Format::eR8G8B8Unorm;
            }

            auto loadedImage = Image::CreateImage({static_cast<uint32_t>(image.width), static_cast<uint32_t>(image.height)}, format);
            loadedImage->Write(image.image, vk::ImageAspectFlagBits::eColor, nullptr);

            return loadedImage;
        }
        else if (!image.uri.empty())
        {
            if (image.uri.starts_with("data:")) // Don't support data uris
            {
                sceneInfo.Warnings += "Could not load texture named \"" + texture.name + "\" from a 'data:' URI\n";
                return nullptr;
            }

            // Load from texture filename
            // Strip non-filename data (like relative paths)
            std::string textureFilename = std::filesystem::path(image.uri).filename().string();;

            return Image::LoadFromTextureFile(textureFilename);
        }

        sceneInfo.Warnings += "Could not load texture as it contains no bufferView or URI\n";

        return nullptr;
    }

    static Texture::Pointer CreateTextureFromTexture(const tinygltf::Model &model, const tinygltf::Texture &texture, SceneInformation &sceneInfo)
    {
        auto image = CreateImageFromTexture(model, texture, sceneInfo);
        if (image == nullptr)
        {
            return Texture::GetMagentaTexture();
        }

        auto sampler = CreateSamplerFromTexture(model, texture);

        return std::make_shared<Texture>(texture.name, image, sampler);
    }

    static void UpdateGlobalSceneInformationFromModel(const tinygltf::Model &model, SceneInformation &sceneInfo)
    {
        sceneInfo.Materials.resize(model.materials.size());
        for (size_t i = 0; i < model.materials.size(); i++)
        {
            auto &mat = model.materials[i];
            auto material = Material::CreateMaterial(mat.name);

            // Color + alpha
            auto &color = mat.pbrMetallicRoughness.baseColorFactor;
            auto alpha = static_cast<float>(mat.alphaCutoff);
            if (color.size() >= 3)
            {
                material->SetColor(glm::vec4{static_cast<float>(color[0]), static_cast<float>(color[1]), static_cast<float>(color[2]), alpha});
            }

            // Roughness
            auto roughness = static_cast<float>(mat.pbrMetallicRoughness.roughnessFactor);
            material->SetRoughness(roughness);

            // Metallic
            auto metallic = static_cast<float>(mat.pbrMetallicRoughness.metallicFactor);
            material->SetMetallic(metallic);

            // TODO emissive property (stored in gltf as a vec3 rather than just strength)

            auto colorTextureIndex = mat.pbrMetallicRoughness.baseColorTexture.index;
            if (colorTextureIndex >= 0)
            {
                auto colorTexture = CreateTextureFromTexture(model, model.textures.at(colorTextureIndex), sceneInfo);
                material->SetTexture(0, colorTexture);
            }

            sceneInfo.Materials[i] = material;
        }
    }


    /* Call hierarchy:
     * Scene::LoadModel - Loads a GLTF model
     *      UpdateGlobalSceneInformationFromModel - creates materials and loads textures
     *          CreateTextureFromTexture - creates a Spinner::Texture
     *              CreateImageFromTexture - loads a Spinner::Image from a tinygltf texture
     *              CreateSamplerFromTexture - creates a Spinner::Sampler from a tinygltf texture's sampler
     *      CreateSceneObjectFromScene - Creates a scene object from a GLTF model
     *          CreateSceneObjectFromNode - recursive, calls itself on its node's children
     *              DoesMeshHaveAttribute - used to see which kind of mesh to make (static/skinned)
     *              CreateStaticMeshBuffersFromMesh / CreateSkinnedMeshBuffersFromMesh - creates a static or skinned mesh based on available attributes
     *                  GetGLTFAttribute - used to get vertex attributes from a vertex data buffer
     *              CreateEmptySceneObject - creates an empty scene object for when there is no mesh
     */

    SceneObject::Pointer Scene::LoadModel(const std::string &modelFilename)
    {
        std::string assetPath = GetAssetPath(AssetType::Model, modelFilename);

        if (!FileExists(assetPath))
        {
            throw std::runtime_error("Cannot load model from asset path " + assetPath + " as file does not exist");
        }

        auto extension = std::filesystem::path(assetPath).extension().string();
        std::transform(extension.begin(), extension.end(), extension.begin(), [](unsigned char c) -> unsigned char { return std::tolower(c); });

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

        // Create materials
        SceneInformation sceneInfo{};
        UpdateGlobalSceneInformationFromModel(model, sceneInfo);

        // If multiple scenes
        if (model.scenes.size() > 1)
        {
            std::string fileName = std::filesystem::path(assetPath).filename().replace_extension().string();
            SceneObject::Pointer fileObject = std::make_shared<SceneObject>(fileName);
            for (auto &scene : model.scenes)
            {
                auto sceneObject = CreateSceneObjectFromScene(model, scene, sceneInfo);
                if (sceneObject == nullptr)
                {
                    continue;
                }

                fileObject->AddChild(sceneObject);
            }

            if (!sceneInfo.Warnings.empty())
            {
                std::cerr << sceneInfo.Warnings;
            }

            return fileObject;
        }

        // If no default scene (somehow) then return nullptr
        if (model.defaultScene < 0)
        {
            return nullptr;
        }

        // If only one scene then create it
        auto outSceneObject = CreateSceneObjectFromScene(model, model.scenes.at(model.defaultScene), sceneInfo);

        if (!sceneInfo.Warnings.empty())
        {
            std::cerr << sceneInfo.Warnings;
        }

        return outSceneObject;
    }

    bool Scene::IsActive() const noexcept
    {
        return Active;
    }

    void Scene::SetActive(bool active)
    {
        Active = active;
    }

    std::shared_ptr<Spinner::Lighting> Scene::GetLighting() const noexcept
    {
        return Lighting;
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

    std::weak_ptr<Spinner::Lighting> Scene::GlobalLighting = {};

    std::shared_ptr<Spinner::Lighting> Scene::GetGlobalLighting()
    {
        if (GlobalLighting.expired())
        {
            auto descriptorPool = DescriptorPool::CreateDefault(32);
            auto lighting = Lighting::CreateLighting();
            GlobalLighting = lighting;
            return lighting;
        }

        return GlobalLighting.lock();
    }

    std::vector<vk::DescriptorSetLayoutBinding> Scene::GetDescriptorSetLayoutBindings()
    {
        auto layoutBindings = std::vector<vk::DescriptorSetLayoutBinding>{
            vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eGeometry | vk::ShaderStageFlagBits::eFragment, nullptr),
        };

        return layoutBindings;
    }

    void Scene::RenderHierarchy()
    {
        constexpr static ImVec4 disabledTextColor = ImVec4(1.0f, 1.0f, 1.0f, 0.6f);

        auto selectedObject = SelectedInHierarchy.lock();
        int indexID = 0;

        GetObjectTree()->Traverse([&](const SceneObject::Pointer &object) -> bool
            {
                ImGui::Indent(8);
                bool isActive = object->IsActive();
                if (!isActive)
                {
                    ImGui::PushStyleColor(ImGuiCol_Text, disabledTextColor);
                }
                std::string id = object->GetName() + "##" + std::to_string(indexID++);
                if (ImGui::RadioButton(id.c_str(), object == selectedObject))
                {
                    if (object != selectedObject)
                    {
                        SelectedInHierarchy = object;
                    }
                    else
                    {
                        SelectedInHierarchy = {}; // deselect if already selected
                    }
                }

                if (!isActive)
                {
                    ImGui::PopStyleColor();
                }

                return true;
            },
            [&](const SceneObject::Pointer &object) -> void
            {
                ImGui::Unindent(8);
            });
    }

    void Scene::RenderSelectedProperties()
    {
        auto selectedObject = SelectedInHierarchy.lock();
        if (selectedObject == nullptr)
        {
            return;
        }

        selectedObject->RenderDebugUI();
    }
} // Spinner
