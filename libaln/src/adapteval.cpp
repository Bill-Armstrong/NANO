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

// adapteval.cpp


#ifdef ALNDLL
#define ALNIMP __declspec(dllexport)
#endif

#include <aln.h>
#include "alnpriv.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

float ALNAPI AdaptEval(ALNNODE* pNode, ALN* pALN, const float* afltX, 
                        CCutoffInfo* pCutoffInfo, ALNNODE** ppActiveLFN)
{
  ASSERT(pNode);
  ASSERT(pALN);
  ASSERT(afltX);
  ASSERT(ppActiveLFN);
  
  // do a cutoff eval to get active LFN and distance
  ALNNODE* pActiveLFN = NULL;
  float flt;
  CEvalCutoff cutoff;
 
  // check for cutoff info
  if (pCutoffInfo != NULL)
  {
    // set up cutoff
    ALNNODE* pEval = pCutoffInfo->pLFN;
    if (pEval != NULL)
    {
      BuildCutoffRoute(pEval);
    }

    // evaluate using cutoff
    flt = AdaptEval(pNode, pALN, afltX, cutoff, &pActiveLFN);

    // set new cutoff info
    pCutoffInfo->pLFN = pActiveLFN;
    pCutoffInfo->fltValue = flt;
  }
  else
  {
    // eval with expanded cutoff
    flt = AdaptEval(pNode, pALN, afltX, cutoff, &pActiveLFN);
  }

#ifdef _DEBUG
  ALNNODE* pLFNCheck = NULL;
  float fltCheck = DebugEval(pNode, pALN, afltX, &pLFNCheck);
	ASSERT(flt == fltCheck); //MYTEST
	ASSERT(pLFNCheck == pActiveLFN); // This could break because of equal LFNs after a split.  This was only corrected if smoothing > 0
	// but now it is corrected in the case of no smoothing.
	// See what happens if we don't use the cutoffs
	//flt = fltCheck; // MYTEST assumes debug version is correct (no cutoffs)
	//pActiveLFN = pLFNCheck; // MYTEST
#endif  // MYTEST 
  *ppActiveLFN = pActiveLFN;

  return flt;
}
