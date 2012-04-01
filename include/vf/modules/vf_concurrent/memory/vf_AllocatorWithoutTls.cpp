// Copyright (C) 2008 by Vinnie Falco, this file is part of VFLib.
// See the file LICENSE.txt for licensing information.

// This precedes every allocation
struct AllocatorWithoutTls::Header
{
  union
  {
    AllocatorWithoutTls::Block* block; // backpointer to the page

    char pad [Memory::allocAlignBytes];
  };
};

//------------------------------------------------------------------------------

class AllocatorWithoutTls::Block : Uncopyable
{
public:
  explicit Block (const size_t bytes) : m_refs (1)
  {
    m_end = reinterpret_cast <char*> (this) + bytes;
    m_free = reinterpret_cast <char*> (
      Memory::pointerAdjustedForAlignment (this + 1));
  }

  ~Block ()
  {
    vfassert (m_refs.get() == 0);
  }

  inline void addref ()
  {
    m_refs.addref ();
  }

  inline bool release ()
  {
    vfassert (m_refs.get () > 0);

    return m_refs.release ();
  }

  enum Result
  {
    success,  // successful allocation
    ignore,   // disregard the block
    consumed  // block is consumed (1 thread sees this)
  };

  Result allocate (size_t bytes, void* pBlock)
  {
    vfassert (bytes > 0);

    Result result;

    for (;;)
    {
      char* base = m_free.get ();

      if (base)
      {
        char* p = Memory::pointerAdjustedForAlignment (base);
        char* free = p + bytes;

        if (free <= m_end)
        {
          // Try to commit the allocation
          if (m_free.compareAndSet (free, base))
          {
            *(reinterpret_cast <void**> (pBlock)) = p;
            result = success;
            break;
          }
          else
          {
            // Someone changed m_free, retry.
          }
        }
        else
        {
          // Mark the block consumed.
          if (m_free.compareAndSet (0, base))
          {
            // Only one caller sees this, the rest get 'ignore'
            result = consumed;
            break;
          }
          else
          {
            // Happens with another concurrent allocate(), retry.
          }
        }
      }
      else
      {
        // Block is consumed, ignore it.
        result = ignore;
        break;
      }
    }

    return result;
  }

private:
  AtomicCounter m_refs;        // reference count
  AtomicPointer <char> m_free; // next free byte or 0 if inactive.
  char* m_end;                 // last free byte + 1
};

//------------------------------------------------------------------------------

inline AllocatorWithoutTls::Block* AllocatorWithoutTls::newBlock ()
{
  return new (m_pages->allocate ()) Block (m_pages->getPageBytes());
}

inline void AllocatorWithoutTls::deleteBlock (Block* b)
{
  // It is critical that we do not call the destructor,
  // because due to the lock-free implementation, a Block
  // can be accessed for a short time after it is deleted.
  /* b->~Block (); */ // DO NOT CALL!!!

  PageAllocator::deallocate (b);
}

AllocatorWithoutTls::AllocatorWithoutTls ()
  : m_pages (GlobalPageAllocator::getInstance ())
{
  if (m_pages->getPageBytes () < sizeof (Block) + 256)
    Throw (Error().fail (__FILE__, __LINE__, TRANS("the block size is too small")));

  m_active = newBlock ();
}

AllocatorWithoutTls::~AllocatorWithoutTls ()
{
  deleteBlock (m_active);
}

//------------------------------------------------------------------------------

void* AllocatorWithoutTls::allocate (const size_t bytes)
{
  const size_t actual = sizeof (Header) + bytes;

  if (actual > m_pages->getPageBytes ())
    Throw (Error().fail (__FILE__, __LINE__, TRANS("the memory request was too large")));

  Header* h;

  for (;;)
  {
    // Get an active block.
    Block* b = m_active;
    while (!b)
    {
	  VF_JUCE::Thread::yield ();
      b = m_active;
    }

    // (*) It is possible for the block to get a final release here
    //     In this case it will have been put in the garbage, and
    //     m_active will not match.
    //
    // TODO: DIFFERENTIAL REFERENCE COUNTER?

    // Acquire a reference.
    b->addref ();

    // Is it still active?
    if (m_active == b)
    {
      // Yes so try to allocate from it.
      const Block::Result result = b->allocate (actual, &h);

      if (result == Block::success)
      {
        // Keep the reference and return the allocation.
        h->block = b;
        break;
      }
      else if (result == Block::consumed)
      {
        // Remove block from active.
        m_active = 0;

        // Take away the reference we added
        b->release ();

        // Take away the original active reference.
        if (b->release ())
          deleteBlock (b);

        // Install a fresh empty active block.
        m_active = newBlock ();
      }
      else
      {
        if (b->release ())
          deleteBlock (b);
      }

      // Try again.
    }
    else
    {
      // Block became inactive, so release our reference.
      b->release ();

      // (*) It is possible for this to be a duplicate final release.
    }
  }

  return h + 1;
}

//------------------------------------------------------------------------------

void AllocatorWithoutTls::deallocate (void* p)
{
  Block* const b = (reinterpret_cast <Header*> (p) - 1)->block;

  if (b->release ())
    deleteBlock (b);
}
