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

// alnconvertdtree.cpp

#ifdef ALNDLL
#define ALNIMP __declspec(dllexport)
#endif

#include <aln.h>
#include "alnpriv.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// conversion to dtree
// pointer to constructed DTREE returned in ppDtree
// returns ALN_* error code, (ALN_NOERROR on success)
// if ALN_ERRDTREE is returned, check dtree_errno
ALNIMP int ALNAPI ALNConvertDtree(const ALN* pALN, int nMaxDepth,
    DTREE** ppDtree)
{
    // parameter variance
    if (pALN == NULL)
        return ALN_GENERIC;

    if (ppDtree == NULL)
        return ALN_GENERIC;

    if (nMaxDepth < DTREE_MINDEPTH || nMaxDepth > DTREE_MAXDEPTH)
        return ALN_GENERIC;

    int nResult = ALN_NOERROR;

    // build dtree
    *ppDtree = BuildDtree(pALN, nMaxDepth);
    if (*ppDtree == NULL && dtree_errno != DTR_NOERROR)
        nResult = ALN_GENERIC;  // dtree lib error
    else if (*ppDtree == NULL)
        nResult = ALN_OUTOFMEM;

    return nResult;
}