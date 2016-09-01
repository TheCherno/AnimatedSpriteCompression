#pragma once

#include <Flinty.h>

class BufferStream
{
private:
	std::vector<byte> m_Buffer;
	uint m_Position;
public:
	BufferStream(uint size = 0);
	~BufferStream();

	inline const byte* GetBuffer() const { return &m_Buffer[0]; }
	inline uint GetSize() const { return m_Buffer.size(); }

	template<typename T>
	void Write(const T* data, uint count = 1)
	{
		WriteInternal((const void*)data, sizeof(T) * count);
	}

	template<typename T>
	void Write(const T* data, uint count, uint offset)
	{
		WriteInternal((const void*)data, sizeof(T) * count, offset);
	}
private:
	void WriteInternal(const void* buffer, uint size);
	void WriteInternal(const void* buffer, uint size, uint offset);
};