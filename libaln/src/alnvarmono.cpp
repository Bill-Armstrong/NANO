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

// alnvarmono.cpp

#ifdef ALNDLL
#define ALNIMP __declspec(dllexport)
#endif

#include <aln.h>
#include "alnpriv.h"
#include <errno.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// ALN variable monotonicity type
// monotonicity type returned in pnMono
// returns ALN_* error code, (ALN_NOERROR on success)
ALNIMP int ALNAPI ALNVarMono(const ALN* pALN, int nVar, int* pnMono)
{
    // parameter variance
    if (pALN == NULL)
        return ALN_GENERIC;

    if (nVar < 0 || nVar >= pALN->nDim)
        return ALN_GENERIC;

    if (pnMono == NULL)
        return ALN_GENERIC;

    *pnMono = CheckMonotonicity(pALN->pTree, pALN, nVar);
    ASSERT(*pnMono == MONO_CONSTANT || *pnMono == MONO_FREE ||
        *pnMono == MONO_STRONGINC || *pnMono == MONO_STRONGDEC ||
        *pnMono == MONO_WEAKINC || *pnMono == MONO_WEAKDEC);

    return ALN_NOERROR;
}

///////////////////////////////////////////////////////////////////////////////
// workhorse of ALNVarMono

int ALNAPI CheckMonotonicity(const ALNNODE* pNode, const ALN* pALN, int nVar)
{
    ASSERT(pNode);
    ASSERT(pALN);
    ASSERT(nVar >= 0 && nVar < pALN->nDim);

    // traverse tree and examine weights on variable
    if (NODE_MINMAXTYPE(pNode) & NF_LFN)
    {
        // examine actual weight on var
        float fltW = LFN_W(pNode)[nVar + 1];   // ...skip bias weight
        if (fltW < 0)
        {
            return MONO_STRONGDEC;
        }
        else if (fltW > 0)
        {
            return MONO_STRONGINC;
        }
        // else has zero weight so return MONO_NONE

        return MONO_CONSTANT; // return monotonicity
    }

    ASSERT(NODE_MINMAXTYPE(pNode) & NF_MINMAX);

    // monotonicity initially set to undefined
    int nMono = -1;

    int nChildren = MINMAX_NUMCHILDREN(pNode);
    ALNNODE* const* apChildren = MINMAX_CHILDREN(pNode);
    for (int i = 0; i < nChildren; i++)
    {
        const ALNNODE* pChild = apChildren[i];
        ASSERT(pChild);

        int nChildMono = CheckMonotonicity(pChild, pALN, nVar);

        if (nChildMono == MONO_FREE)
            return MONO_FREE;   // child is free, so we're free

        else if (i == 0)
            nMono = nChildMono; // first time through, so take on child mono

        else if (nMono == nChildMono)
            continue;           // no change

          // move from const to weak if child is weak or strong                
        else if (nMono == MONO_CONSTANT &&
            (nChildMono == MONO_WEAKINC || nChildMono == MONO_STRONGINC))
            nMono = MONO_WEAKINC;

        else if (nMono == MONO_CONSTANT &&
            (nChildMono == MONO_WEAKDEC || nChildMono == MONO_STRONGDEC))
            nMono = MONO_WEAKDEC;

        // remain weak if child is strong or constant
        else if (nMono == MONO_WEAKINC &&
            (nChildMono == MONO_STRONGINC || nChildMono == MONO_CONSTANT))
            continue;

        else if (nMono == MONO_WEAKDEC &&
            (nChildMono == MONO_STRONGDEC || nChildMono == MONO_CONSTANT))
            continue;

        // move from strong to weak if we're strong and child is weak or constant
        else if (nMono == MONO_STRONGINC &&
            (nChildMono == MONO_WEAKINC || nChildMono == MONO_CONSTANT))
            nMono = MONO_WEAKINC;

        else if (nMono == MONO_STRONGDEC &&
            (nChildMono == MONO_WEAKDEC || nChildMono == MONO_CONSTANT))
            nMono = MONO_WEAKDEC;

        // opposite child monotonicities      
        else return MONO_FREE;
    }

    // no conflicting child monotonicities
    ASSERT(nMono != -1);
    return nMono;
}

