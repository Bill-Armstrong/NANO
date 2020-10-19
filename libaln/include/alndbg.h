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


// alndbg.h


#ifndef __ALNDBG_H__
#define __ALNDBG_H__

#ifndef __ALNCFG_H__
#include <alncfg.h>
#endif

/*
/////////////////////////////////////////////////////////////////////////////
// Special ALNDebugBreak: used to break into debugger at critical times
// Non-Microsoft, non-Intel compilers will need to implement DebugBreak()
// This was removed by WWA on July 22, 2019.
*/

#ifndef ALNDebugBreak
#define ALNDebugBreak()
#endif


/*
///////////////////////////////////////////////////////////////////////////////
// diagnostic support
*/

#ifdef _DEBUG

ALNIMP void __cdecl ALNTrace(const char* pszFormat, ...);

#ifndef ALNTRACE
#define ALNTRACE ALNTrace
#endif  /* ALNTRACE */

#ifndef assert
#include <assert.h>
#endif

#ifndef ALNASSERT
#define ALNASSERT(f) assert(f)
#endif  /* ALNASSERT */

#ifndef ALNVERIFY
#define ALNVERIFY(f) ALNASSERT(f)
#endif  /* ALNVERIFY */

#else   /* _DEBUG */

inline void __cdecl ALNTrace(const char* pszFmt, ...) { }

#ifndef ALNTRACE
#define ALNTRACE              1 ? (void)0 : ALNTrace
#endif  /* ALNTRACE */

#ifndef ALNASSERT
#define ALNASSERT(f) ((void)0)
#endif  /* ALNASSERT */

#ifndef ALNVERIFY
#define ALNVERIFY(f) ((void)(f))
#endif  /* ALNVERIFY */

#endif  /* !_DEBUG */


/*
///////////////////////////////////////////////////////////////////////////////
*/

#endif  /* __ALNDBG_H__ */
