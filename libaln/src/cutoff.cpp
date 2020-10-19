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

// cutoff.cpp


#ifdef ALNDLL
#define ALNIMP __declspec(dllexport)
#endif

#include <aln.h>
#include "alnpriv.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BOOL ALNAPI Cutoff(float flt, const ALNNODE* pNode, CEvalCutoff& cutoff)
{
    if (MINMAX_ISMAX(pNode))  // if pNode is a MAX
    {
        // cutoff if we're greater than or equal to existing min
        if (cutoff.bMin && (flt >= cutoff.fltMin))
        {
            return TRUE;  // cutoff!
        }

        if (!cutoff.bMax)
        {
            cutoff.bMax = TRUE;
            cutoff.fltMax = flt;
        }
        else if (flt > cutoff.fltMax)  // we set a higher lower bound on the value of the current MAX node
        {
            cutoff.fltMax = flt;
        }
    }
    else  // pNode is a MIN
    {
        ASSERT(MINMAX_ISMIN(pNode));

        // cutoff if we're less than or equal to existing max
        if (cutoff.bMax && (flt <= cutoff.fltMax))
        {
            return TRUE;
        }

        // no cutoff... set new min

        // assume min adjusted by node prior to evaluation:
        if (!cutoff.bMin)
        {
            cutoff.bMin = TRUE;
            cutoff.fltMin = flt;
        }
        else if (flt < cutoff.fltMin)  // we set a lower upper bound on the value of the current MIN node
        {
            cutoff.fltMin = flt;
        }
    }
    return FALSE; // no cutoff
}
