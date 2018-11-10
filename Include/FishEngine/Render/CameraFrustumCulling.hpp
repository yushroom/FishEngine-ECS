#pragma once

#include <FishEngine/Components/Camera.hpp>
#include <FishEngine/Mesh.hpp>

namespace FishEngine
{

class CameraFrustumCulling
{
public:
    CameraFrustumCulling(Camera* gameCamera, float aspectRatio)
    {
        viewProjMat = gameCamera->GetViewProjectionMatrix();
        
//        if (bgfx::getCaps()->homogeneousDepth)
//            box_ndc.SetMinMax({ -1, -1, -1 }, { 1, 1, 1 });
//        else
//            box_ndc.SetMinMax({ -1, -1, 0 }, { 1, 1, 1 });
    }
    
    bool Visiable(Mesh* mesh, const Matrix4x4& modelMat) const
    {
        auto mvp = viewProjMat * modelMat;
        bool insideFrustum = false;
        for (int i = 0; i < 8; ++i)
        {
            auto corner = mesh->m_Bounds.GetCorner(i);
            auto posV = mvp.MultiplyPoint(corner);
            insideFrustum = box_ndc.Contains(posV);
            if (insideFrustum)
                break;
        }
        return insideFrustum;
    }
    
private:
    Matrix4x4 viewProjMat;
    Bounds box_ndc;
};

}
