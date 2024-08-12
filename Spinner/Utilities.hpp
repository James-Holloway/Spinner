#ifndef SPINNER_UTILITIES_HPP
#define SPINNER_UTILITIES_HPP

#include <vector>
#include <memory>
#include <string>
#include <filesystem>
#include <fstream>

#include "Extra/AlignedAllocator.hpp"

namespace Spinner
{
    inline bool Contains(const std::string &haystack, const std::string &needle)
    {
        return haystack.find(needle) != std::string::npos;
    }

    template<typename T>
    inline bool Contains(const std::vector<T> &haystack, const T &needle)
    {
        if (haystack.empty())
            return false;
        return (std::find(haystack.begin(), haystack.end(), needle) != haystack.end());
    }

    template<typename T>
    inline bool IsPow2(T x)
    {
        return (x & (x - 1)) == 0;
    }

    template<typename T>
    static inline T AlignUp(T val, T alignment)
    {
        assert(IsPow2(alignment));
        return (val + alignment - 1) & ~(alignment - 1);
    }

    template<typename T>
    static inline T AlignDown(T val, T alignment)
    {
        assert(IsPow2(alignment));
        return val & ~(alignment - 1);
    }

    enum class AssetType
    {
        Shader,
        Model,
        Texture
    };

    inline std::string GetAssetPath(AssetType assetType, std::string assetName)
    {
        std::filesystem::path currentDirectory = std::filesystem::current_path();

        switch (assetType)
        {
            case AssetType::Shader:
                assetName = "Shaders/" + assetName;
                break;
            case AssetType::Model:
            case AssetType::Texture:
                assetName = "Assets/" + assetName;
                break;
        }

        return currentDirectory.string() + "/" + assetName;
    }

    inline bool FileExists(const std::string &filePath)
    {
        return std::filesystem::exists(filePath) && !std::filesystem::is_directory(filePath);
    }

    template<uint32_t ALIGNMENT = 16>
    inline auto ReadFile(const std::string &filePath)
    {
        std::ifstream file(filePath, std::ios::ate | std::ios::binary);
        if (!file.is_open())
        {
            throw std::runtime_error("Failed to open file: " + filePath);
        }

        size_t fileSize = (size_t) file.tellg();
        // @formatter:off
        std::vector<char, ::AlignedAllocator<char, ALIGNMENT>> buffer(fileSize);
        // @formatter:on

        file.seekg(0);
        file.read(buffer.data(), static_cast<std::streamsize>(fileSize));

        file.close();

        return buffer;
    }
}

#endif //SPINNER_UTILITIES_HPP
