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

// cutoffevalminmax.cpp

#ifdef ALNDLL
#define ALNIMP __declspec(dllexport)
#endif

#include <aln.h>
#include "alnpriv.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// Switches for turning on/off optimizations
extern BOOL bAlphaBeta;
extern BOOL bDistanceOptimization;


///////////////////////////////////////////////////////////////////////////////
// minmax node specific eval - returns distance to surface
//  - non-destructive, ie, does not change ALN structure
// NOTE: cutoff always passed on stack!

float ALNAPI CutoffEvalMinMax(const ALNNODE* pNode, const ALN* pALN,
	const float* afltX, CEvalCutoff cutoff,
	ALNNODE** ppActiveLFN)
{
	ASSERT(NODE_ISMINMAX(pNode));

	// We use the sample counts and centroids of the child nodes to generate a hyperplane H roughly separating the child samples.
	// The branch to take first during an evaluation is the one representing the side of H afltX lies on.
	const ALNNODE* pChild0;
	const ALNNODE* pChild1;
	int nDim = pALN->nDim;
	// We take the dot product of the normal vector, in direction left centroid to right centroid, with afltX - H to find the branch which goes first.
	// Note that this handles the case where the normal is zero length as after a split and child centroids are equal.
	float dotproduct = 0;
	for (int i = 0; i < nDim - 1; i++)
	{
		dotproduct += MINMAX_NORMAL(pNode)[i] * afltX[i];
	}
	dotproduct += MINMAX_THRESHOLD(pNode); // add constant stored for speed; see split_ops.cpp
	if (dotproduct > 0)
	{
		pChild0 = MINMAX_RIGHT(pNode);
		pChild1 = MINMAX_LEFT(pNode);
	}
	else
	{
		pChild1 = MINMAX_RIGHT(pNode);
		pChild0 = MINMAX_LEFT(pNode);
	}

	/*
	// set first child -- Monroe's code prioritizes the past active node, see also buildcutoffroute.cpp
	if (MINMAX_EVAL(pNode))
		pChild0 = MINMAX_EVAL(pNode);
	else
		pChild0 = MINMAX_LEFT(pNode);

	// set next child
	if (pChild0 == MINMAX_LEFT(pNode))
		pChild1 = MINMAX_RIGHT(pNode);
	else
		pChild1 = MINMAX_LEFT(pNode);
	*/
	// get reference to region for this node
	const ALNREGION& region = pALN->aRegions[NODE_REGION(pNode)];
	float fltDist;

	// eval first child
	ALNNODE* pActiveLFN0;
	float flt0 = CutoffEval(pChild0, pALN, afltX, cutoff, &pActiveLFN0);

	// see if we can cutoff...
	if (bAlphaBeta && Cutoff(flt0, pNode, cutoff))
	{
		*ppActiveLFN = pActiveLFN0;
		return flt0;
	}
	// Check if second child is too far from sibling in any axis j; in which case
	// the tree of the second child is cut off and we return the value from the first child

	ASSERT(pChild1);
	if (bDistanceOptimization && MINMAX_SIGMA(pChild1))
	{
		for (int j = 0; j < nDim - 1; j++)
		{
			if (fabs(afltX[j] - MINMAX_CENTROID(pChild1)[j]) >  MINMAX_SIGMA(pChild1)[j]) // Do we need a safety factor?
			{
				return flt0;
			}
		}
	}

	// eval second child
	ALNNODE* pActiveLFN1;
	float flt1 = CutoffEval(pChild1, pALN, afltX, cutoff, &pActiveLFN1);

	// calc active child and distance without using CalcActiveChild()
	if((MINMAX_ISMAX(pNode) > 0) == (flt1 > flt0)) // int MINMAX_ISMAX is used as a bit-vector!
	{
		*ppActiveLFN = pActiveLFN1;
		fltDist = flt1;
	}
	else
	{
		*ppActiveLFN = pActiveLFN0;
		fltDist = flt0;
	}
  
  return fltDist;
}

