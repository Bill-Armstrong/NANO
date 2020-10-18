// calling conventions

/* MIT License

Copyright(c) 2020 William Ward Armstrong

Permission is hereby granted, free of charge, to any person obtaining a copy
of this softwareand associated documentation files(the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions :

The above copyright noticeand this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.*/

// alncfg.h

#ifndef __ALNCFG_H__
#define __ALNCFG_H__

/*
/////////////////////////////////////////////////////////////////////////////
// calling conventions
*/

/* define inline as static for non-C++ compilers */
#ifndef NODEFINE_INLINE

#if ( !defined(__cplusplus) && !defined(inline) )
#define inline static
#endif

#endif  /* NODEFINE_INLINE */

/* define __cdecl for non-Microsoft compilers */
#ifndef NODEFINE_CDECL

#if	( !defined(_MSC_VER) && !defined(__cdecl) )
#define __cdecl
#endif

#endif  /* NODEFINE_CDECL */

/* define __stdcall for non-Microsoft compilers */
#ifndef NODEFINE_STDCALL

#if	( !defined(_MSC_VER) && !defined(__stdcall) )
#define __stdcall
#endif

#endif /* NODEFINE_STDCALL */

/* define __fastcall for non-Microsoft compilers */
#ifndef NODEFINE_FASTCALL

#if	( !defined(_MSC_VER) && !defined(__fastcall) )
#define __fastcall
#endif

#endif /* NODEFINE_FASTCALL */


#define ALNAPI __stdcall


/*
/////////////////////////////////////////////////////////////////////////////
// DLL function exports - define ALNDLL if linking to libalndll(d).DLL
*/

#if defined(ALNDLL) && !defined(ALNIMP)
#define ALNIMP __declspec(dllimport)
#elif !defined(ALNIMP)
#define ALNIMP
#endif

/*
/////////////////////////////////////////////////////////////////////////////
// library files (Microsoft compilers only)
*/

#ifndef ALN_NOFORCE_LIBS

#ifdef _MSC_VER

#ifdef _DEBUG

  #ifdef ALNDLL
  #pragma comment(lib, "libalndlld.lib")      /* ALNDLL */
  #else

    #ifdef _MT
      #ifdef _DLL
      #pragma comment(lib, "libalndmtd.lib")  /* MT DLL */
      #else  /* !_DLL */
      #pragma comment(lib, "libalnmtd.lib")   /* MT */
      #endif /* _DLL */
    #else
      #pragma comment(lib, "libalnd.lib")
    #endif /* _MT */

  #endif /* ALNDLL */


#else   /* !_DEBUG */

  #ifdef ALNDLL
  #pragma comment(lib, "libalndll.lib")       /* ALNDLL */
  #else

    #ifdef _MT
      #ifdef _DLL
      #pragma comment(lib, "libalndmt.lib")   /* MT DLL */
      #else  /* !_DLL */
      #pragma comment(lib, "libalnmt.lib")    /* MT */
      #endif /* _DLL */
    #else
      #pragma comment(lib, "libaln.lib")
    #endif /* _MT */

  #endif /* ALNDLL */

#endif  /* _DEBUG */

#endif  /* _MSC_VER */

#endif  /* ALN_NOFORCE_LIBS */

/*
///////////////////////////////////////////////////////////////////////////////
*/

#endif  /* __ALNCFG_H__ */
