#include <FishEngine/Material.hpp>
#include <FishEngine/Mesh.hpp>
#include <FishEngine/GameApp.hpp>
#include <FishEngine/ECS.hpp>
#include <FishEngine/Components/Camera.hpp>
#include <FishEngine/Components/Light.hpp>
#include <FishEngine/Components/Renderable.hpp>
#include <FishEngine/Systems/FreeCameraSystem.hpp>
#include <FishEngine/Components/SingletonInput.hpp>


class ModelViewer : public GameApp
{
public:
	void Start() override
	{
		//m_Shader = ShaderUtil::Compile("/Users/yushroom/program/FishEngine-ECS/shader/vs.bin", "/Users/yushroom/program/FishEngine-ECS/shader/fs.bin");
		m_Shader = ShaderUtil::Compile("D:/program/FishEngine-ECS/shader/vs.bin", "D:/program/FishEngine-ECS/shader/fs.bin");


		{
			ECS::EntityID goID = m_Scene->CreateGameObject();
			m_Scene->GameObjectAddComponent<Camera>(goID);
			auto go = m_Scene->GetGameObjectByID(goID);
			go->GetTransform()->position.Set(0, 0, -15);
			m_Scene->GameObjectAddComponent<FreeCamera>(goID);
		}
		{
			ECS::EntityID goID = m_Scene->CreateGameObject();
			m_Scene->GameObjectAddComponent<Light>(goID);
		}
		
		for (int y = 0; y < 11; ++y)
		{
			for (int x = 0; x < 11; ++x)
			{
				ECS::EntityID goID = m_Scene->CreateGameObject();
				auto go = m_Scene->GetGameObjectByID(goID);
				auto& pos = go->GetTransform()->position;
				pos.x = -7.5f + x * 1.5f;
				pos.y = -7.5f + y * 1.5f;
				Renderable* rend = m_Scene->GameObjectAddComponent<Renderable>(goID);
				rend->mesh = Mesh::Sphere;

				Material* mat = new Material();
				mat->m_Shader = m_Shader;
				mat->pbrparams[0] = x * 0.1f;
				mat->pbrparams[1] = y * 0.1f;
				rend->material = mat;
			}
		}
		
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
