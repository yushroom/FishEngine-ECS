#include <FishEngine/Material.hpp>
#include <FishEngine/Mesh.hpp>
#include <FishEngine/GameApp.hpp>
#include <FishEngine/ECS.hpp>
#include <FishEngine/Components/Transform.hpp>
#include <FishEngine/Components/Camera.hpp>
#include <FishEngine/Components/Light.hpp>
#include <FishEngine/Components/Renderable.hpp>
#include <FishEngine/Systems/FreeCameraSystem.hpp>
#include <FishEngine/Components/SingletonInput.hpp>
#include <FishEngine/Assets.hpp>

class ModelViewer : public GameApp
{
public:
	void Start() override
	{
		auto vs = FISHENGINE_ROOT "Shaders/runtime/PBR_vs.bin";
		auto fs = FISHENGINE_ROOT "Shaders/runtime/PBR_fs.bin";
		m_Shader = ShaderUtil::Compile(vs, fs);
		
		const char* test_path = FISHENGINE_ROOT "Assets/Models/T-Rex.glb";
		Mesh* trex = MeshUtil::FromGLTF(test_path);

		{
			auto go = m_Scene->CreateGameObject();
			m_Scene->GameObjectAddComponent<Camera>(go);
			go->GetTransform()->SetLocalPosition(0, 1, -10);
			m_Scene->GameObjectAddComponent<FreeCamera>(go);
		}
		{
			auto go = m_Scene->CreateGameObject();
			auto light = m_Scene->GameObjectAddComponent<Light>(go);
			light->direction.z = 1;
		}
		
		auto go = m_Scene->CreateGameObject();
		Renderable* rend = m_Scene->GameObjectAddComponent<Renderable>(go);
		rend->mesh = trex;

		Material* mat = new Material();
		mat->SetShader(m_Shader);
		Vector4 pbrparams(0, 0.5f, 0, 0);
		mat->SetVector("BaseColor", Vector4::one);
		mat->SetVector("PBRParams", pbrparams);
		rend->material = mat;

		m_Scene->AddSystem(new FreeCameraSystem());
	}
	
private:
	Shader* m_Shader = nullptr;
};


int main(void)
{
	ModelViewer demo;
	demo.Run();
	return 0;
}
