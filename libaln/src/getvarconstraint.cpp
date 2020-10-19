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

// getvarconstraint.cpp

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
// constraint retrieval

ALNCONSTRAINT* ALNAPI GetVarConstraint(int nRegion, const ALN* pALN, 
                                       int nVar)
{
  ASSERT(pALN);
  ASSERT(nRegion >= 0 && nRegion < pALN->nRegions);
  ASSERT(nVar >= 0 && nVar < pALN->nDim);
  ALNREGION* pRegion = &(pALN->aRegions[nRegion]);

  while (TRUE)
  {
    // if region constrains all vars, then get constraint directly
    if (pRegion->nConstr == pALN->nDim)
    {
      ASSERT(pRegion->aConstr[nVar].nVarIndex == nVar);
      return &(pRegion->aConstr[nVar]);
    }
    else if (pRegion->afVarMap == NULL || TESTMAP(pRegion->afVarMap, nVar))
    {
      // no var map, or map indicates region contains this var
      for (int i = pRegion->nConstr - 1; i >= 0; i--)
      {
        ASSERT(pRegion->aConstr[i].nVarIndex >= 0 && 
               pRegion->aConstr[i].nVarIndex < pALN->nDim);
      
        if (pRegion->aConstr[i].nVarIndex == nVar)
          return &(pRegion->aConstr[i]);
      }
    }
    
    // variable not constrained in this region, check parent region
    if (pRegion->nParentRegion == -1)
      return NULL;  // var not found!?
    
    pRegion = &(pALN->aRegions[pRegion->nParentRegion]); 
  }
}

