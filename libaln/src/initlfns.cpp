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

// initlfns.cpp

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
// init any uninitialized LFN's

void ALNAPI InitLFNs(ALNNODE* pNode, ALN* pALN, const float* afltX)
{
	ASSERT(pNode != NULL);
	ASSERT(pALN != NULL);
	ASSERT(afltX != NULL);
	
	// are we an LFN?
	if (NODE_ISLFN(pNode))
	{
    if (!LFN_ISINIT(pNode)) // not initialized?
    {
  		int nOutput = pALN->nOutput;
  		int nDim = LFN_VDIM(pNode);
      ASSERT(nDim == pALN->nDim); 
  		float* afltW = LFN_W(pNode) + 1; // weight vector... skip bias
  		float* afltC = LFN_C(pNode);			// centroid vector
  		float* afltD = LFN_D(pNode);			// ave sq dist from centroid vector
			// vector initialization
  	  for (int i = 0; i < nDim; i++)
  	  { 
  			ALNCONSTRAINT* pConstr = GetVarConstraint(NODE_REGION(pNode), pALN, i);
  			ASSERT(pConstr != NULL);
			
        // init weights			
  			afltW[i] = (float)max((float)min(pConstr->fltWMax, ALNRandFloat() * 0.00002F - 0.00001F),
  			               pConstr->fltWMin);
  	    afltC[i] = afltX[i];
  	    afltD[i] = pConstr->fltSqEpsilon;
  	  }
  	  // the hyperplane is on the centroid to start so W[0] = 0
  	  LFN_W(pNode)[0] = 0;
    
  		ASSERT(afltW[nOutput] == -1.0F);// make sure output var is -1

  	  // successfully initialized
      LFN_FLAGS(pNode) |= LF_INIT;
    }
	}
  else
  {
	  // we're a minmax... iterate over children
	  ASSERT(NODE_ISMINMAX(pNode));

    InitLFNs(MINMAX_LEFT(pNode), pALN, afltX);
    InitLFNs(MINMAX_RIGHT(pNode), pALN, afltX);
  }
}
