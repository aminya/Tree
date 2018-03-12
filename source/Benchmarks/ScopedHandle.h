#pragma once

using HANDLE = void*;

class ScopedHandle
{
public:
   explicit ScopedHandle(HANDLE handle);

   ScopedHandle(const ScopedHandle& other);
   ScopedHandle& operator=(const ScopedHandle& other);

   ScopedHandle(ScopedHandle&& other);
   ScopedHandle& operator=(ScopedHandle&& other);

   ~ScopedHandle();

   void Close();

   void Reset(HANDLE handle);

   bool IsValid() const;

   operator HANDLE() const;

private:

   HANDLE m_handle;
};
