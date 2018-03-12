#include "ScopedHandle.h"

#include <Windows.h>

namespace
{
   auto Duplicate(HANDLE handle) -> HANDLE
   {
      if (handle == INVALID_HANDLE_VALUE)
      {
         return nullptr;
      }

      HANDLE duplicate;

      const auto successfullyDuplicated = !DuplicateHandle(
         GetCurrentProcess(),
         handle,
         GetCurrentProcess(),
         &duplicate,
         0,
         FALSE, DUPLICATE_SAME_ACCESS);

      return successfullyDuplicated ? duplicate : nullptr;
   }
}

ScopedHandle::ScopedHandle(HANDLE handle) :
   m_handle{ handle }
{
}

ScopedHandle::ScopedHandle(const ScopedHandle& other)
{
   m_handle = Duplicate(other.m_handle);
}

ScopedHandle& ScopedHandle::operator=(const ScopedHandle& other)
{
   if (this != &other)
   {
      m_handle = Duplicate(other.m_handle);
   }

   return *this;
}

ScopedHandle::ScopedHandle(ScopedHandle&& other)
{
   m_handle = other.m_handle;
   other.m_handle = nullptr;
}

ScopedHandle& ScopedHandle::operator=(ScopedHandle&& other)
{
   if (this != &other)
   {
      m_handle = other.m_handle;
      other.m_handle = nullptr;
   }

   return *this;
}

ScopedHandle::~ScopedHandle()
{
   Close();
}

void ScopedHandle::Close()
{
   if (IsValid())
   {
      CloseHandle(m_handle);
      m_handle = nullptr;
   }
}

void ScopedHandle::Reset(HANDLE handle)
{
   Close();

   m_handle = handle;
}

bool ScopedHandle::IsValid() const
{
   return (m_handle != nullptr) && (m_handle != INVALID_HANDLE_VALUE);
}

ScopedHandle::operator HANDLE() const
{
   return m_handle;
}
