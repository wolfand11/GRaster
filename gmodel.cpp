#include "gmodel.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include "glog.h"
#include "gutils.h"
using namespace GMath;
using namespace std;

GOBJModel::GOBJModel(GModelType modelType, const std::string filename) : modelType(modelType), verts_(), uv_(), norms_(), facet_vrt_(), facet_tex_(), facet_nrm_(), diffusemap_(), normalmap_(), specularmap_()
{
    Setup(modelType, filename);
}

void GOBJModel::Setup(GModelType modelType, const std::string filename)
{
    if(this->modelType==GModelType::kMTInvalid)
    {
        GLog::LogError("model is setuped! originType = ", this->modelType, " newSetupType = ", modelType);
        return;
    }
    modelFilePath = filename;
    this->modelType = modelType;
    switch (modelType)
    {
    case GModelType::kMTInvalid:
        GLog::LogError("modelType is kInvalid");
        break;
    case GModelType::kMTTriange:
        verts_.assign({vec3(-1,-1,0),vec3(1,-1,0),vec3(0,1,0)});
        uv_.assign({vec2(0,0),vec2(1,0),vec2(1,1)});
        norms_.assign({vec3(0,0,1)});
        facet_vrt_.assign({3,2,1});
        facet_tex_.assign({3,2,1});
        facet_nrm_.assign({1,1,1});
        break;
    case GModelType::kMTCube:
        verts_.assign({
            // front
            vec3(-0.5,-0.5,-0.5),vec3(0.5,-0.5,-0.5),vec3(0.5,0.5,-0.5),vec3(-0.5,0.5,-0.5),
            // back
            vec3(-0.5,-0.5,0.5),vec3(0.5,-0.5,0.5),vec3(0.5,0.5,0.5),vec3(-0.5,0.5,0.5),
        });
        uv_.assign({
            vec2(0,0),vec2(1,0),vec2(1,1),vec2(0,1),
        });
        norms_.assign({
            // front
            vec3(0,0,-1),
            // right
            vec3(1,0,0),
            // back
            vec3(0,0,1),
            // left
            vec3(-1,0,0),
            // top
            vec3(0,1,0),
            // bottom
            vec3(0,-1,0),
        });
        // index start from 1
        facet_vrt_.assign({
            // front
            3,2,1, 4,3,1,
            // right
            7,6,2, 3,7,2,
            // back
            5,6,7, 5,7,8,
            // left
            4,1,5, 4,5,8,
            // top
            7,3,4, 8,7,4,
            // bottom
            1,2,6, 1,6,5
        });
        facet_tex_.assign({
            // front
            3,2,1, 4,3,1,
            // right
            3,2,1, 4,3,1,
            // back
            3,2,1, 4,3,1,
            // left
            3,2,1, 4,3,1,
            // top
            3,2,1, 4,3,1,
            // bottom
            3,2,1, 4,3,1,
        });
        facet_nrm_.assign({
            // front
            1,1,1,1,1,1,
            // right
            2,2,2,2,2,2,
            // back
            3,3,3,3,3,3,
            // left
            4,4,4,4,4,4,
            // top
            5,5,5,5,5,5,
            // bottom
            6,6,6,6,6,6,
        });
        break;
    case GModelType::kMTObj:
        std::ifstream in;
        in.open (filename, std::ifstream::in);
        if (in.fail()) return;
        std::string line;
        while (!in.eof()) {
            std::getline(in, line);
            std::istringstream iss(line.c_str());
            char trash;
            if (!line.compare(0, 2, "v ")) {
                iss >> trash;
                vec3 v;
                for (int i=0;i<3;i++) iss >> v[i];
                verts_.push_back(v);
            } else if (!line.compare(0, 3, "vn ")) {
                iss >> trash >> trash;
                vec3 n;
                for (int i=0;i<3;i++) iss >> n[i];
                norms_.push_back(n.normalize());
            } else if (!line.compare(0, 3, "vt ")) {
                iss >> trash >> trash;
                vec2 uv;
                for (int i=0;i<2;i++) iss >> uv[i];
                uv_.push_back(uv);
            }  else if (!line.compare(0, 2, "f ")) {
                int f,t,n;
                iss >> trash;
                int cnt = 0;
                while (iss >> f >> trash >> t >> trash >> n) {
                    facet_vrt_.push_back(--f);
                    facet_tex_.push_back(--t);
                    facet_nrm_.push_back(--n);
                    cnt++;
                }
                if (3!=cnt) {
                    std::cerr << "Error: the obj file is supposed to be triangulated" << std::endl;
                    in.close();
                    return;
                }
            }
        }
        in.close();
        std::cerr << "# v# " << verts_.size() << " f# "  << nfaces() << " vt# " << uv_.size() << " vn# " << norms_.size() << std::endl;
        break;
    }
    //load_texture(filename, "_diffuse.tga",    diffusemap_);
    //load_texture(filename, "_nm_tangent.tga", normalmap_);
    //load_texture(filename, "_spec.tga",       specularmap_);
}

int GOBJModel::nfaces() const {
    return facet_vrt_.size()/3;
}

vec3 GOBJModel::vert(const int i) const {
    return verts_[i];
}

vec3 GOBJModel::vert(const int iface, const int nthvert) const {
    return verts_[facet_vrt_[iface*3+nthvert]];
}

TGAColor GOBJModel::diffuse(const vec2 &uvf) const {
    return diffusemap_.get(uvf[0]*diffusemap_.get_width(), uvf[1]*diffusemap_.get_height());
}

vec3 GOBJModel::normal(const vec2 &uvf) const {
    TGAColor c = normalmap_.get(uvf[0]*normalmap_.get_width(), uvf[1]*normalmap_.get_height());
    vec3 res;
    for (int i=0; i<3; i++)
        res[2-i] = c[i]/255.*2 - 1;
    return res;
}

double GOBJModel::specular(const vec2 &uvf) const {
    return specularmap_.get(uvf[0]*specularmap_.get_width(), uvf[1]*specularmap_.get_height())[0];
}

vec2 GOBJModel::uv(const int iface, const int nthvert) const {
    return uv_[facet_tex_[iface*3+nthvert]];
}

vec3 GOBJModel::normal(const int iface, const int nthvert) const {
    return norms_[facet_nrm_[iface*3+nthvert]];
}

GGLModel GGLModel::CreateWithObjModel(GOBJModel *objModel, GMipmapType diff, GMipmapType norm, GMipmapType spec)
{
    GGLModel glModel;
    if(objModel==nullptr)
    {
        GLog::LogError("objModel==nullptr");
        return glModel;
    }
    for(int vertIdx=0; vertIdx<objModel->nfaces()*3; vertIdx++)
    {
        int posIdx = objModel->facet_vrt_[vertIdx];
        vec3 pos = objModel->verts_[posIdx];
        glModel.verts_.push_back(pos);
        int uvIdx = objModel->facet_tex_[vertIdx];
        vec2 uv = objModel->uv_[uvIdx];
        glModel.uv_.push_back(uv);
        int normIdx = objModel->facet_nrm_[vertIdx];
        vec3 norm = objModel->norms_[normIdx];
        glModel.norms_.push_back(norm);

        glModel.index_.push_back(vertIdx);
    }
    glModel.modelFilePath = objModel->modelFilePath;
    glModel.diff_mipmaptype = diff;
    glModel.norm_mipmaptype = norm;
    glModel.spec_mipmaptype = spec;

    load_mipmap(glModel.modelFilePath, "_diffuse", glModel.diffusemap_mipmaps_, diff!=GMipmapType::kMipmapOff?20:1);
    load_mipmap(glModel.modelFilePath, "_nm_tangent", glModel.normalmap_mipmaps_, norm!=GMipmapType::kMipmapOff?20:1);
    load_mipmap(glModel.modelFilePath, "_spec", glModel.specularmap_mipmaps_, spec!=GMipmapType::kMipmapOff?20:1);
    return glModel;
}

int GGLModel::nverts() const
{
    return verts_.size();
}

void GGLModel::load_texture(std::string texfile, TGAImage &img)
{
    std::cerr << "texture file " << texfile << " loading " << (img.read_tga_file(texfile.c_str()) ? "ok" : "failed") << std::endl;
    img.flip_vertically();
}

void GGLModel::load_mipmap(const std::string& modelFilePath, const std::string suffix, std::vector<TGAImage> &mipmaps, int mipmapMaxLevel)
{
    size_t dot = modelFilePath.find_last_of(".");
    if (dot==std::string::npos) return;
    std::string texfilePath = modelFilePath.substr(0,dot) + suffix + "/";
    for(int i=0; i<mipmapMaxLevel; i++)
    {
        string texfile = texfilePath + to_string(i) + ".tga";
        if(GUtils::IsFileExist(texfile))
        {
            TGAImage mipmap;
            load_texture(texfile, mipmap);
            mipmaps.push_back(move(mipmap));
        }
        else
        {
            break;
        }
    }
}
