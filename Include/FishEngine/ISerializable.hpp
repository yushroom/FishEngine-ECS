#pragma once

namespace FishEngine
{
	class InputArchive;
	class OutputArchive;
	
	class SerializationInfo
	{
	public:
		
		template<class T>
		void AddVale(const char* key, const T& value);
		
		template<class T>
		T GetValue(const char* key);
	};
	
	class StreamingContext
	{
		
	};
	
	class ISerializabe
	{
	public:
		virtual void Deserialize(InputArchive& archive) = 0;
		virtual void Serialize(OutputArchive& archive) const = 0;
		
//		void GetObjectData(SerializationInfo& info, StreamingContext& ctx);
//		void SetObjecctData(SerializationInfo& info, SerializationInfo& ctx);
	};
}
