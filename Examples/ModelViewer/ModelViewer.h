#pragma once

#include "GameCore.h"
#include "GraphicsCore.h"
#include "CameraController.h"
#include "Camera.h"
#include "Model.h"
#include "ShadowCamera.h"


using namespace GameCore;
using namespace Math;
using namespace Graphics;

namespace FishEngine
{
	class Scene;
	class Camera;
}

class ModelViewer : public GameCore::IGameApp
{
public:

	ModelViewer(void);

	virtual void Startup(void) override;
	virtual void Cleanup(void) override;

	virtual void Update(float deltaT) override;
	virtual void RenderScene(void) override;

private:

	void RenderLightShadows(GraphicsContext& gfxContext);

	enum eObjectFilter { kOpaque = 0x1, kCutout = 0x2, kTransparent = 0x4, kAll = 0xF, kNone = 0x0 };
	void RenderObjects(GraphicsContext& Context, const Matrix4& ViewProjMat, eObjectFilter Filter = kAll);
	void CreateParticleEffects();
	Camera m_Camera;
	std::unique_ptr<CameraController> m_CameraController;
	Matrix4 m_ViewProjMatrix;
	D3D12_VIEWPORT m_MainViewport;
	D3D12_RECT m_MainScissor;

	RootSignature m_RootSig;
	GraphicsPSO m_DepthPSO;
	GraphicsPSO m_CutoutDepthPSO;
	GraphicsPSO m_ModelPSO;
#ifdef _WAVE_OP
	GraphicsPSO m_DepthWaveOpsPSO;
	GraphicsPSO m_ModelWaveOpsPSO;
#endif
	GraphicsPSO m_CutoutModelPSO;
	GraphicsPSO m_ShadowPSO;
	GraphicsPSO m_CutoutShadowPSO;
	GraphicsPSO m_WaveTileCountPSO;

	D3D12_CPU_DESCRIPTOR_HANDLE m_DefaultSampler;
	D3D12_CPU_DESCRIPTOR_HANDLE m_ShadowSampler;
	D3D12_CPU_DESCRIPTOR_HANDLE m_BiasedDefaultSampler;

	D3D12_CPU_DESCRIPTOR_HANDLE m_ExtraTextures[6];
	Model m_Model;
	std::vector<bool> m_pMaterialIsCutout;

	Vector3 m_SunDirection;
	ShadowCamera m_SunShadow;

	UserDescriptorHeap m_imguiSrvDescHeap;

public:
	FishEngine::Scene* m_Scene = nullptr;
};
