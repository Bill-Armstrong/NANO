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

// cutoffevallfn.cpp

#ifdef ALNDLL
#define ALNIMP __declspec(dllexport)
#endif

#include <aln.h>
#include "alnpriv.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

long CountLeafevals;

///////////////////////////////////////////////////////////////////////////////
// LFN specific eval - returns distance to surface
//  - returns distance of LFN from point

float ALNAPI CutoffEvalLFN(const ALNNODE* pNode, const ALN* pALN, 
                            const float* afltX, ALNNODE** ppActiveLFN)
{
  ASSERT(afltX != NULL);
  ASSERT(pALN != NULL);
  ASSERT(pNode != NULL);
  ASSERT(ppActiveLFN != NULL);
  ASSERT(NODE_ISLFN(pNode));

  ASSERT(LFN_VARMAP(pNode) == NULL);      // var map not yet supported
  ASSERT(LFN_VDIM(pNode) == pALN->nDim);  // no different sized vectors yet

  *ppActiveLFN = (ALNNODE*)pNode;         // cast away the const...

  // calc dist of point from line
  int nDim = pALN->nDim;
  const float* afltW = LFN_W(pNode);
  float fltA = *afltW++;                 // skip past bias weight       
  for (int i = 0; i < nDim; i++)     // Go to nDim - 1 instead, then use this for quickeval.
  {
    fltA += afltW[i] * afltX[i];
  }
  CountLeafevals++;
  // NODE_DISTANCE(pNode) = fltA; optional?
  return fltA;
}
