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

// debugevalminmax.cpp


#ifdef ALNDLL
#define ALNIMP __declspec(dllexport)
#endif

#include <aln.h>
#include "alnpriv.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef _DEBUG

float ALNAPI DebugEvalMinMax(const ALNNODE* pNode, const ALN* pALN, 
                            const float* afltX, ALNNODE** ppActiveLFN)
{
  ASSERT(NODE_ISMINMAX(pNode));

  // set first child
  ALNNODE* pChild0;
  if (MINMAX_EVAL(pNode))
    pChild0 = MINMAX_EVAL(pNode);
  else
    pChild0 = MINMAX_LEFT(pNode);

  // set next child
  ALNNODE* pChild1;
  if (pChild0 == MINMAX_LEFT(pNode))
    pChild1 = MINMAX_RIGHT(pNode);
  else
    pChild1 = MINMAX_LEFT(pNode);

  // eval first child
  ALNNODE* pActiveLFN0;
  float flt0 = DebugEval(pChild0, pALN, afltX, &pActiveLFN0);
  
	// eval second child
  ALNNODE* pActiveLFN1;
	float flt1 = DebugEval(pChild1, pALN, afltX, &pActiveLFN1);

  // get reference to region for this node
  const ALNREGION& region = pALN->aRegions[NODE_REGION(pNode)];

  // calc active child, active child response, and distance
  float fltDist, fltRespActive;
  int nActive = CalcActiveChild(fltRespActive, 
                                fltDist, 
                                flt0, flt1, pNode);
	
  if (nActive == 0)
  {
    *ppActiveLFN = pActiveLFN0;
  }
  else
  {
    *ppActiveLFN = pActiveLFN1;
  }
  
  return fltDist;
}


#endif