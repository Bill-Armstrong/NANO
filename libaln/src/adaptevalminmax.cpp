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

// adaptevalminmax.cpp


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
// minmax node specific eval - evaluation and adaptation setup
//  - sets the distance, active and goal surfaces, and eval flag
// NOTE: cutoff always passed on stack!

float ALNAPI AdaptEvalMinMax(ALNNODE* pNode, ALN* pALN, const float* afltX, CEvalCutoff cutoff, ALNNODE** ppActiveLFN)
{
	ASSERT(NODE_ISMINMAX(pNode));

	// set node eval flags
 	NODE_FLAGS(pNode) |= NF_EVAL;  //NODE_FLAGS(pNode) ((pNode)->fNode)
	NODE_FLAGS(MINMAX_LEFT(pNode)) &= ~NF_EVAL;
	NODE_FLAGS(MINMAX_RIGHT(pNode)) &= ~NF_EVAL;//((pNode)->DATA.MINMAX.CHILDREN.CHILDSEPARATE.pRightChild)
	// set first child
	ALNNODE* pChild0;
	if (MINMAX_EVAL(pNode))    // ((pNode)->DATA.MINMAX.pEvalChild)
		pChild0 = MINMAX_EVAL(pNode);
	else
		pChild0 = MINMAX_LEFT(pNode);

	// set next child
	ALNNODE* pChild1;
	if (pChild0 == MINMAX_LEFT(pNode))
		pChild1 = MINMAX_RIGHT(pNode);
	else
		pChild1 = MINMAX_LEFT(pNode);

	// get reference to region for this node
	ALNREGION& region = pALN->aRegions[NODE_REGION(pNode)];
	
	// eval first child
	ALNNODE* pActiveLFN0;
	float flt0 = AdaptEval(pChild0, pALN, afltX, cutoff, &pActiveLFN0);

	// see if we can cutoff...
	if (Cutoff(flt0, pNode, cutoff))
	{
		*ppActiveLFN = pActiveLFN0;
		MINMAX_ACTIVE(pNode) = pChild0;
		NODE_DISTANCE(pNode) = flt0;
		MINMAX_RESPACTIVE(pNode) = 1.0;	 // we can't have < 1 without additional evaluation
		return flt0;  
	}  // Removed the cutoff to see what happens, now restored

	// eval second child
	ALNNODE* pActiveLFN1;
	float flt1 = AdaptEval(pChild1, pALN, afltX, cutoff, &pActiveLFN1);
		
	// Recall that flt0 == flt1 is not a rare event!  It always happens after a split,
	// however it happens then only once as the first adapt will likely destroy equality.
	MINMAX_RESPACTIVE(pNode) = 1.0;
	if ((MINMAX_ISMAX(pNode) > 0) == (flt1 > flt0)) // int MINMAX_ISMAX is used as a bit-vector!
	{
		NODE_DISTANCE(pNode) = flt1;
		*ppActiveLFN = pActiveLFN1;
		MINMAX_ACTIVE(pNode) = pChild1;
	}
	else
	{
		NODE_DISTANCE(pNode) = flt0;
		*ppActiveLFN = pActiveLFN0;
		MINMAX_ACTIVE(pNode) = pChild0;
	}
	return NODE_DISTANCE(pNode);
}



