#include "Texture.hpp"

#define STBI_MAX_DIMENSIONS (1 << 27)

#include <stb_image.h>

#include <utility>
#include "Utilities.hpp"

namespace Spinner
{
    static Texture::Pointer BlackTexture;
    static Texture::Pointer WhiteTexture;
    static Texture::Pointer TransparentTexture;
    static Texture::Pointer MagentaTexture;
    static Texture::Pointer BlankNormal;

    Texture::Texture(const std::string &textureFilename) : Name(textureFilename)
    {
        LoadTexture(textureFilename);
        CreateMainImageView();
    }

    Texture::Texture(std::string name, const std::vector<uint8_t> &textureData, vk::Extent2D size, vk::Format format, int mipLevels) : Name(std::move(name))
    {
        Image = Image::CreateImage(size, format, vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst, vk::ImageType::e2D, vk::ImageTiling::eOptimal, mipLevels, vma::MemoryUsage::eGpuOnly);
        Image->Write(textureData, vk::ImageAspectFlagBits::eColor, nullptr);
        CreateMainImageView();
    }

    Texture::Texture(std::string name, const uint8_t *textureData, size_t textureDataSize, vk::Extent2D size, vk::Format format, int mipLevels) : Name(std::move(name))
    {
        Image = Image::CreateImage(size, format, vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst, vk::ImageType::e2D, vk::ImageTiling::eOptimal, mipLevels, vma::MemoryUsage::eGpuOnly);
        Image->Write(textureData, textureDataSize, vk::ImageAspectFlagBits::eColor, nullptr);
        CreateMainImageView();
    }

    Texture::Texture(std::string name, Spinner::Image::Pointer image, Spinner::Sampler::Pointer sampler) : Name(std::move(name)), Image(std::move(image)), Sampler(std::move(sampler))
    {
        if (Image != nullptr)
        {
            CreateMainImageView();
        }
    }

    void Texture::LoadTexture(const std::string &textureFilename)
    {
        uint32_t mipLevels = 1;
        Image = Image::LoadFromTextureFile(textureFilename, mipLevels);
    }

    Spinner::Image::Pointer Texture::GetImage() const
    {
        return Image;
    }

    void Texture::SetImage(Spinner::Image::Pointer image)
    {
        Image = std::move(image);
    }

    Spinner::Sampler::Pointer Texture::GetSampler() const
    {
        return Sampler;
    }

    void Texture::SetSampler(Spinner::Sampler::Pointer sampler)
    {
        Sampler = std::move(sampler);
    }

    std::string Texture::GetName() const
    {
        return Name;
    }

    void Texture::SetName(const std::string &name)
    {
        Name = name;
    }

    Texture::Pointer Texture::PixelColor(glm::vec4 color)
    {
        uint32_t pixel = glm::packUnorm4x8(color);

        auto texture = std::make_shared<Texture>("Pixel Color", reinterpret_cast<uint8_t *>(&pixel), 4, vk::Extent2D{1, 1}, vk::Format::eR8G8B8A8Unorm, 1);

        auto sampler = Sampler::CreateSampler();
        texture->SetSampler(sampler);

        return texture;
    }

    void Texture::CreateDefaultTextures()
    {
        BlackTexture = PixelColor(glm::vec4(0, 0, 0, 1));
        WhiteTexture = PixelColor(glm::vec4(1, 1, 1, 1));
        TransparentTexture = PixelColor(glm::vec4(0, 0, 0, 0));
        MagentaTexture = PixelColor(glm::vec4(1, 0, 1, 1));
        BlankNormal = PixelColor(glm::vec4(0.5f, 0.5f, 1.0f, 1.0f));
    }

    void Texture::ReleaseDefaultTextures()
    {
        BlackTexture.reset();
        WhiteTexture.reset();
        TransparentTexture.reset();
        MagentaTexture.reset();
        BlankNormal.reset();
    }

    Texture::Pointer Texture::GetBlackTexture()
    {
        return BlackTexture;
    }

    Texture::Pointer Texture::GetWhiteTexture()
    {
        return WhiteTexture;
    }

    Texture::Pointer Texture::GetTransparentTexture()
    {
        return TransparentTexture;
    }

    Texture::Pointer Texture::GetMagentaTexture()
    {
        return MagentaTexture;
    }

    Texture::Pointer Texture::GetBlankNormal()
    {
        return BlankNormal;
    }

    void Texture::CreateMainImageView()
    {
        if (Image == nullptr || Image->GetImage() == nullptr)
        {
            throw std::runtime_error("Cannot create main image view for an invalid image");
        }

        Image->CreateMainImageView(vk::ImageAspectFlagBits::eColor);
    }
} // Spinner