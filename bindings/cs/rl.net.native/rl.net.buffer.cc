#include "rl.net.buffer.h"

using namespace rl_net_native;

API const buffer* CloneBufferSharedPointer(const buffer* original)
{
    return new buffer(*original);
}

API void ReleaseBufferSharedPointer(const buffer* buffer)
{
    delete buffer;
}

API const unsigned char* GetSharedBufferBegin(const buffer* buffer)
{
    return (*buffer)->preamble_begin();
}

API const size_t GetSharedBufferLength(const buffer* buffer)
{
    return (*buffer)->buffer_filled_size();
}
