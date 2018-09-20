#include <FishEngine/Serialization/Serialization.hpp>
#include <FishEngine/Serialization/Archive.hpp>
#include <FishEngine.hpp>

using namespace FishEngine;
using namespace FishEditor;

namespace FishEngine
{
	// Vector2
	FishEngine::InputArchive& operator>>(FishEngine::InputArchive& archive, Vector2& t)
	{
		archive.AddNVP("x", t.x);
		archive.AddNVP("y", t.y);
		return archive;
	}
	FishEngine::OutputArchive& operator<<(FishEngine::OutputArchive& archive, const Vector2& t)
	{
		archive.AddNVP("x", t.x);
		archive.AddNVP("y", t.y);
		return archive;
	}

	// Vector3
	FishEngine::InputArchive& operator>>(FishEngine::InputArchive& archive, Vector3& t)
	{
		archive.AddNVP("x", t.x);
		archive.AddNVP("y", t.y);
		archive.AddNVP("z", t.z);
		return archive;
	}
	FishEngine::OutputArchive& operator<<(FishEngine::OutputArchive& archive, const Vector3& t)
	{
		archive.AddNVP("x", t.x);
		archive.AddNVP("y", t.y);
		archive.AddNVP("z", t.z);
		return archive;
	}

	// Vector4
	FishEngine::InputArchive& operator>>(FishEngine::InputArchive& archive, Vector4& t)
	{
		archive.AddNVP("x", t.x);
		archive.AddNVP("y", t.y);
		archive.AddNVP("z", t.z);
		archive.AddNVP("w", t.w);
		return archive;
	}
	FishEngine::OutputArchive& operator<<(FishEngine::OutputArchive& archive, const Vector4& t)
	{
		archive.AddNVP("x", t.x);
		archive.AddNVP("y", t.y);
		archive.AddNVP("z", t.z);
		archive.AddNVP("w", t.w);
		return archive;
	}

	// Quaternion
	FishEngine::InputArchive& operator>>(FishEngine::InputArchive& archive, Quaternion& t)
	{
		archive.AddNVP("x", t.x);
		archive.AddNVP("y", t.y);
		archive.AddNVP("z", t.z);
		archive.AddNVP("w", t.w);
		return archive;
	}
	FishEngine::OutputArchive& operator<<(FishEngine::OutputArchive& archive, const Quaternion& t)
	{
		archive.AddNVP("x", t.x);
		archive.AddNVP("y", t.y);
		archive.AddNVP("z", t.z);
		archive.AddNVP("w", t.w);
		return archive;
	}
}

// Animator
void FishEngine::Animator::Deserialize(InputArchive& archive)
{
	FishEngine::Component::Deserialize(archive);
	archive.AddNVP("m_Clips", this->m_Clips);
	archive.AddNVP("m_CurrentClipIndex", this->m_CurrentClipIndex);
	archive.AddNVP("m_LocalTimer", this->m_LocalTimer);
}

void FishEngine::Animator::Serialize(OutputArchive& archive) const
{
	FishEngine::Component::Serialize(archive);
	archive.AddNVP("m_Clips", this->m_Clips);
	archive.AddNVP("m_CurrentClipIndex", this->m_CurrentClipIndex);
	archive.AddNVP("m_LocalTimer", this->m_LocalTimer);
}


// Camera
void FishEngine::Camera::Deserialize(InputArchive& archive)
{
	FishEngine::Component::Deserialize(archive);
	archive.AddNVP("m_Type", this->m_Type);
	archive.AddNVP("m_FieldOfView", this->m_FieldOfView);
	archive.AddNVP("m_NearClipPlane", this->m_NearClipPlane);
	archive.AddNVP("m_FarClipPlane", this->m_FarClipPlane);
	archive.AddNVP("m_Orthographic", this->m_Orthographic);
	archive.AddNVP("m_OrthographicSize", this->m_OrthographicSize);
}

void FishEngine::Camera::Serialize(OutputArchive& archive) const
{
	FishEngine::Component::Serialize(archive);
	archive.AddNVP("m_Type", this->m_Type);
	archive.AddNVP("m_FieldOfView", this->m_FieldOfView);
	archive.AddNVP("m_NearClipPlane", this->m_NearClipPlane);
	archive.AddNVP("m_FarClipPlane", this->m_FarClipPlane);
	archive.AddNVP("m_Orthographic", this->m_Orthographic);
	archive.AddNVP("m_OrthographicSize", this->m_OrthographicSize);
}


// Light
void FishEngine::Light::Deserialize(InputArchive& archive)
{
	FishEngine::Component::Deserialize(archive);
}

void FishEngine::Light::Serialize(OutputArchive& archive) const
{
	FishEngine::Component::Serialize(archive);
}


// Renderable
void FishEngine::Renderable::Deserialize(InputArchive& archive)
{
	FishEngine::Component::Deserialize(archive);
	archive.AddNVP("m_Materials", this->m_Materials);
	archive.AddNVP("m_Mesh", this->m_Mesh);
	archive.AddNVP("m_Skin", this->m_Skin);
}

void FishEngine::Renderable::Serialize(OutputArchive& archive) const
{
	FishEngine::Component::Serialize(archive);
	archive.AddNVP("m_Materials", this->m_Materials);
	archive.AddNVP("m_Mesh", this->m_Mesh);
	archive.AddNVP("m_Skin", this->m_Skin);
}


// Skybox
void FishEngine::Skybox::Deserialize(InputArchive& archive)
{
	FishEngine::Component::Deserialize(archive);
	archive.AddNVP("m_SkyboxMaterial", this->m_SkyboxMaterial);
}

void FishEngine::Skybox::Serialize(OutputArchive& archive) const
{
	FishEngine::Component::Serialize(archive);
	archive.AddNVP("m_SkyboxMaterial", this->m_SkyboxMaterial);
}


// Transform
void FishEngine::Transform::Deserialize(InputArchive& archive)
{
	FishEngine::Component::Deserialize(archive);
	archive.AddNVP("m_LocalRotation", this->m_LocalRotation);
	archive.AddNVP("m_LocalPosition", this->m_LocalPosition);
	archive.AddNVP("m_LocalScale", this->m_LocalScale);
	archive.AddNVP("m_Children", this->m_Children);
	archive.AddNVP("m_Father", this->m_Father);
	archive.AddNVP("m_RootOrder", this->m_RootOrder);
}

void FishEngine::Transform::Serialize(OutputArchive& archive) const
{
	FishEngine::Component::Serialize(archive);
	archive.AddNVP("m_LocalRotation", this->m_LocalRotation);
	archive.AddNVP("m_LocalPosition", this->m_LocalPosition);
	archive.AddNVP("m_LocalScale", this->m_LocalScale);
	archive.AddNVP("m_Children", this->m_Children);
	archive.AddNVP("m_Father", this->m_Father);
	archive.AddNVP("m_RootOrder", this->m_RootOrder);
}

