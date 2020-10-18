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

// alnex.cpp
// exception handling routines


#ifdef ALNDLL
#define ALNIMP __declspec(dllexport)
#endif

#include <aln.h>
#include "alnpriv.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

///////////////////////////////////////////////////////////////////////////////
// CALNException

CALNException::CALNException(BOOL bAutoDelete /*= TRUE*/, 
                             const char* pszReason /*= NULL*/)
{
  m_bAutoDelete = bAutoDelete;
	
  m_szReason[0] = '\0';	  // set NULL at beginning of string
	if (pszReason)
		strncpy(m_szReason, pszReason, sizeof(m_szReason) - 1);
  m_szReason[255] = '\0';	// set NULL at end of string
}
	
void CALNException::Delete()
{
  if (m_bAutoDelete)
    delete this;
}

void ALNAPI ThrowALNException()
{
  static CALNException gALNException(FALSE);
  throw &gALNException;
}
 
///////////////////////////////////////////////////////////////////////////////
// CALNMemoryException

CALNMemoryException::CALNMemoryException(BOOL bAutoDelete /*= TRUE*/)
  : CALNException(bAutoDelete)
{
}

void ALNAPI ThrowALNMemoryException()
{
  static CALNMemoryException gALNMemoryException(FALSE);
  throw &gALNMemoryException;
}

///////////////////////////////////////////////////////////////////////////////
// CALNUserException

CALNUserException::CALNUserException(BOOL bAutoDelete /*= TRUE*/)
  : CALNException(bAutoDelete)
{
}

void ALNAPI ThrowALNUserException()
{
  static CALNUserException gALNUserException(FALSE);
  throw &gALNUserException;
}
