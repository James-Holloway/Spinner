#ifndef SPINNER_TEXTURE_HPP
#define SPINNER_TEXTURE_HPP

#include "Image.hpp"
#include "Sampler.hpp"
#include "GLM.hpp"

namespace Spinner
{

    class Texture
    {
    public:
        using Pointer = std::shared_ptr<Texture>;

        Texture() = default;
        explicit Texture(const std::string &textureFilename);
        Texture(std::string name, const std::vector<uint8_t> &textureData, vk::Extent2D size, vk::Format format = vk::Format::eR8G8B8A8Unorm, int mipLevels = 1);
        Texture(std::string name, const uint8_t *textureData, size_t textureDataSize, vk::Extent2D size, vk::Format format = vk::Format::eR8G8B8A8Unorm, int mipLevels = 1);
        Texture(std::string name, Spinner::Image::Pointer image,  Spinner::Sampler::Pointer sampler);
        virtual ~Texture() = default;

        void LoadTexture(const std::string &textureFilename);
        void CreateMainImageView();

        [[nodiscard]] Spinner::Image::Pointer GetImage() const;
        void SetImage(Spinner::Image::Pointer image);
        [[nodiscard]] Spinner::Sampler::Pointer GetSampler() const;
        void SetSampler(Spinner::Sampler::Pointer sampler);
        [[nodiscard]] std::string GetName() const;
        void SetName(const std::string &name);
        [[nodiscard]] bool GetIsTransparent() const;
        void SetIsTransparent(bool transparent);

    protected:
        std::string Name;
        Spinner::Image::Pointer Image;
        Spinner::Sampler::Pointer Sampler;

    public:
        static Pointer PixelColor(glm::vec4 color);

        static void CreateDefaultTextures(); // Creates the default textures (see below Get___Texture functions)
        static void ReleaseDefaultTextures(); // Releases references to the created default textures, but any other references will remain
        static Texture::Pointer GetBlackTexture();
        static Texture::Pointer GetWhiteTexture();
        static Texture::Pointer GetTransparentTexture();
        static Texture::Pointer GetMagentaTexture();
        static Texture::Pointer GetBlankNormal();
    };

} // Spinner

#endif //SPINNER_TEXTURE_HPP
