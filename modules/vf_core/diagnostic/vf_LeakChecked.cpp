/*============================================================================*/
/*
  VFLib: https://github.com/vinniefalco/VFLib

  Copyright (C) 2008 by Vinnie Falco <vinnie.falco@gmail.com>

  This library contains portions of other open source products covered by
  separate licenses. Please see the corresponding source files for specific
  terms.
  
  VFLib is provided under the terms of The MIT License (MIT):

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
  IN THE SOFTWARE.
*/
/*============================================================================*/

#if VF_USE_LEAKCHECKED

/*============================================================================*/
// Type-independent portion of Counter
class LeakCheckedBase::CounterBase::Singleton
{
public:
  void push_back (CounterBase* counter)
  {
    m_list.push_front (counter);
  }

  void detectAllLeaks ()
  {
    for (;;)
    {
      CounterBase* counter = m_list.pop_front ();

      if (!counter)
        break;
    
      counter->detectLeaks ();
    }
  }

  static Singleton& getInstance ()
  {
    static Singleton instance;

    return instance;
  }

private:
  LockFreeStack <CounterBase> m_list;
};

//------------------------------------------------------------------------------

LeakCheckedBase::CounterBase::CounterBase ()
  : m_className ("")
{
  Singleton::getInstance().push_back (this);
}

void LeakCheckedBase::CounterBase::detectAllLeaks ()
{
  Singleton::getInstance().detectAllLeaks ();
}

void LeakCheckedBase::CounterBase::detectLeaks ()
{
  // If there's a runtime error from this line, it means there's
  // an order of destruction problem between different translation units!
  this->checkPureVirtual ();

  int const count = m_count.get ();

  if (count > 0)
  {
    DBG ("[LEAK] " << count << " of " << getClassName ());
  }
}

//------------------------------------------------------------------------------

void LeakCheckedBase::detectAllLeaks ()
{
  CounterBase::detectAllLeaks ();
}

#endif
