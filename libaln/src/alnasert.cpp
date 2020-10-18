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

// alnasert.cpp

#ifdef ALNDLL
#define ALNIMP __declspec(dllexport)
#endif

#include <aln.h>
#include "alnpriv.h"

#ifdef _DEBUG   // entire file

#ifdef _WIN32   // entire file
#define WIN32_EXTRA_LEAN
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

// NOTE: in separate module so it can replaced if needed

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

LONG alnAssertBusy = -1;

int __stdcall ALNAssertFailed(const char* pszFileName, int nLine)
{
  char szMessage[512];
  
  // format message into buffer
  sprintf(szMessage, "File %hs, Line %d", pszFileName, nLine);
  
  // assume the debugger or auxiliary port
  char szT[512 + 20];
  sprintf(szT, "Assertion Failed: %s\n", szMessage);
  
  OutputDebugString(szT);
  
  if (InterlockedIncrement(&alnAssertBusy) > 0)
  {
    InterlockedDecrement(&alnAssertBusy);
    
    // assert within assert (examine call stack to determine first one)
    ALNDebugBreak();
    return FALSE;
  }
  
  // active popup window for the current thread  HOW CAN THIS BE IMPLEMENTED ?  WWA
  //HWND hWndParent = GetActiveWindow();
  //if (hWndParent != NULL)
  //hWndParent = GetLastActivePopup(hWndParent);
  
  // display the assert
  //int nCode = ::MessageBox(hWndParent, szMessage, "Assertion Failed!",
  //  MB_TASKMODAL|MB_ICONHAND|MB_ABORTRETRYIGNORE|MB_SETFOREGROUND);

  // cleanup
  InterlockedDecrement(&alnAssertBusy);

  //if (nCode == IDIGNORE)
    return FALSE;   // ignore
  
  //if (nCode == IDRETRY)
  //  return TRUE;    // will cause ALNDebugBreak

  ALNAbort();     // should not return (but otherwise ALNDebugBreak)
  return TRUE;
}

#endif  // _WIN32

#endif  // _DEBUG
