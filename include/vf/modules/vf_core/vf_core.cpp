// Copyright (C) 2008 by Vinnie Falco, this file is part of VFLib.
// See the file LICENSE.txt for licensing information.

#include "vf/vf_StandardHeader.h"

#include "vf_core.h"

#if JUCE_MSVC && _DEBUG
#include <crtdbg.h>
#endif

// PROBLEM!
namespace vf
{
// PROBLEM!
#include "vf/modules/vf_concurrent/threads/vf_SharedObject.h"
}

namespace vf
{

#include "diagnostic/vf_CatchAny.cpp"
#include "diagnostic/vf_Debug.cpp"
#include "diagnostic/vf_Error.cpp"
#include "diagnostic/vf_FPUFlags.cpp"
#include "diagnostic/vf_LeakChecked.cpp"

#include "events/vf_OncePerSecond.cpp"
#include "events/vf_PerformedAtExit.cpp"

#include "math/vf_MurmurHash.cpp"

#include "threads/vf_InterruptibleThread.cpp"

#if JUCE_WINDOWS
#include "native/vf_win32_FPUFlags.cpp"

#else
#include "native/vf_posix_FPUFlags.cpp"

#endif

}
