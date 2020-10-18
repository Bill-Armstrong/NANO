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

// prepaln.cpp

#ifdef ALNDLL
#define ALNIMP __declspec(dllexport)
#endif

#include <aln.h>
#include "alnpriv.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BOOL ALNAPI DoPrepRegions(ALN* pALN);
BOOL ALNAPI DoPrepNode(ALN* pALN, ALNNODE* pNode);

BOOL ALNAPI PrepALN(ALN* pALN)
{
  ASSERT(pALN);

  if (!DoPrepRegions(pALN))
    return FALSE;

  if (!DoPrepNode(pALN, pALN->pTree))
    return FALSE;

  return TRUE;
}

BOOL ALNAPI DoPrepRegions(ALN* pALN)
{
  ASSERT(pALN);

  // calc all region and var quantities

  for (int i = 0; i < pALN->nRegions; i++)
  {
    ALNREGION* pRegion = pALN->aRegions + i;
    
    // iterate over var constraints
    for (int j = 0; j < pRegion->nConstr; j++)
    {
      float fltEpsilon = pRegion->aConstr[j].fltEpsilon;

      if (j == pALN->nOutput)
      {
        // make sure wmin and wmax are -1
        pRegion->aConstr[j].fltWMin = -1.0;
        pRegion->aConstr[j].fltWMax = -1.0;
      }

      // calc sq epsilon
      pRegion->aConstr[j].fltSqEpsilon = fltEpsilon * fltEpsilon; // this may be useless now
    }


  }

  return TRUE;
}

BOOL ALNAPI DoPrepNode(ALN* pALN, ALNNODE* pNode)
{
  // currently no prepping required here
  return TRUE;


  // otherwise, recurse over entire tree
  /*
  if (NODE_ISLFN(pNode))
  {
  }
  else
  {
    ASSERT(NODE_ISMINMAX(pNode));
		DoPrepNode(MINMAX_LEFT(pNode));
    DoPrepNode(MINMAX_RIGHT(pNode));
  }

  return TRUE;
  */
}

