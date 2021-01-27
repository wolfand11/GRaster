#ifndef GMODEL_H
#define GMODEL_H

#include <vector>
#include <string>
#include "tgaimage.h"
#include "gmath.h"
#include "ggraphiclibdefine.h"

enum GModelType
{
    kMTInvalid,
    kMTTriange,
    kMTCube,
    kMTObj,
};

class GOBJModel {
public:
    GOBJModel() = default;
    GOBJModel(GModelType modelType, const std::string filename);
    void Setup(GModelType modelType, const std::string filename);
    int nfaces() const;
    GMath::vec3 normal(const int iface, const int nthvert) const;
    GMath::vec3 normal(const GMath::vec2 &uv) const;
    GMath::vec3 vert(const int i) const;
    GMath::vec3 vert(const int iface, const int nthvert) const;
    GMath::vec2 uv(const int iface, const int nthvert) const;
    TGAColor diffuse(const GMath::vec2 &uv) const;
    double specular(const GMath::vec2 &uv) const;

    GModelType modelType = GModelType::kMTInvalid;
    std::vector<GMath::vec3> verts_;
    std::vector<GMath::vec2> uv_;
    std::vector<GMath::vec3> norms_;
    std::vector<int> facet_vrt_;
    std::vector<int> facet_tex_;
    std::vector<int> facet_nrm_;
    TGAImage diffusemap_;
    TGAImage normalmap_;
    TGAImage specularmap_;
    std::string modelFilePath;
};

struct GGLModel
{
    static GGLModel CreateWithObjModel(GOBJModel* objModel, GMipmapType diff=GMipmapType::kMipmapOff, GMipmapType norm=GMipmapType::kMipmapOff, GMipmapType spec=GMipmapType::kMipmapOff);
    int nverts() const;

    void* verts_p()
    {
        return (void*)(verts_.data());
    }
    void* uv_p()
    {
        return (void*)(uv_.data());
    }
    void* norms_p()
    {
        return (void*)(norms_.data());
    }

    void* index_p()
    {
        return (void*)(index_.data());
    }

    int indexCount()
    {
        return index_.size();
    }

    GModelType modelType = GModelType::kMTInvalid;
    // opengl use common index buffer
    // vertex index == uv index == normal index
    std::vector<int> index_;
    // opengl vertex count == uv count == normal count
    std::vector<GMath::vec3> verts_;
    std::vector<GMath::vec2> uv_;
    std::vector<GMath::vec3> norms_;

    GMipmapType diff_mipmaptype;
    std::vector<TGAImage> diffusemap_mipmaps_;
    GMipmapType norm_mipmaptype;
    std::vector<TGAImage> normalmap_mipmaps_;
    GMipmapType spec_mipmaptype;
    std::vector<TGAImage> specularmap_mipmaps_;
    std::string modelFilePath;

    static void load_texture(const std::string texfile, TGAImage &img);
    static void load_mipmap(const std::string& modelFilePath, const std::string suffix, std::vector<TGAImage>& mipmaps, int mipmapMaxLevel);
};

#endif // GMODEL_H
