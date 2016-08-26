#include "BufferStream.h"

BufferStream::BufferStream(uint size)
{
	if (size)
		m_Buffer = std::vector<byte>(size);
}

BufferStream::~BufferStream()
{
}

void BufferStream::WriteInternal(const void* buffer, uint size)
{
	m_Buffer.insert(m_Buffer.end(), (byte*)buffer, (byte*)buffer + size);
}
