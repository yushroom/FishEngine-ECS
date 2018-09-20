#include <FishEngine/Serialization/Archive.hpp>
#include <imgui/imgui.h>
#include <FishEngine/Object.hpp>

namespace FishEditor
{
	class InspectorArchive : public FishEngine::OutputArchive
	{
		void Serialize(short t) override {}
		void Serialize(unsigned short t) override {}
		void Serialize(int t) override
		{
			ImGui::LabelText(m_LastLabel, "%d", t);
		}
		void Serialize(unsigned int t)override
		{
			//ImGui::InputScalar("%ld", ImGuiDataType_U32, (void*)&t);
			int tt = t;
			Serialize(tt);
		}
		void Serialize(long t) override {}
		void Serialize(unsigned long t) override {}
		void Serialize(long long t) override {}
		void Serialize(unsigned long long t) override {}
		void Serialize(float t) override {}
		void Serialize(double t) override {}
		void Serialize(bool t) override
		{
			ImGui::Checkbox(m_LastLabel, &t);
		}
		void Serialize(std::string const & t) override {}

		void SerializeNullPtr() override {}	// nullptr
		void SerializeObject(FishEngine::Object* t) override
		{
			//t->Serialize(*this);
			ImGui::LabelText(m_LastLabel, "%s", t->name.c_str());
		}


		void MapKey(const char* name) override
		{
			//ImGui::Text(name);
			//ImGui::SameLine();
			m_LastLabel = name;
		}

	private:
		const char* m_LastLabel = nullptr;
	};

}