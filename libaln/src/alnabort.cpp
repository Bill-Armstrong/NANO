// ALN Library

/* MIT License

Copyright (c) 2020 William Ward Armstrong

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE. */

// alnabort.cpp


#ifdef ALNDLL
#define ALNIMP __declspec(dllexport)
#endif

#include <aln.h>
#include "alnpriv.h"

// NOTE: in separate module so it can replaced if needed

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// abort proc address
static ALNABORTPROC _pfnAbort = NULL;

// set ALN abort procedure: if ALN needs to abort, it will call this
//   function; if not set, ALN will simply call abort()
// you can use this to chain an abort proc for another library to the ALN
//  library, and simply call ALNAbort
ALNIMP void ALNAPI ALNSetAbortProc(ALNABORTPROC pfnAbortProc)
{
    _pfnAbort = pfnAbortProc;
}

// ALN library abort... cleans up internal ALN library, then calls
// abort procedure defined by call to ALNSetAbortProc()
ALNIMP void __stdcall ALNAbort(void)
{
    // any ALN lib cleanup here

    if (!_pfnAbort || _pfnAbort == ALNAbort)
        abort();

    else
        (*_pfnAbort)();
}
