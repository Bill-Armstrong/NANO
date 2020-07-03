// ALN Library
// Copyright (C) 2018 William W. Armstrong.
// 
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// Version 3 of the License, or (at your option) any later version.
// 
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
// 
// For further information contact 
// William W. Armstrong
// 3624 - 108 Street NW
// Edmonton, Alberta, Canada  T6J 1B4

// adaptlfn.cpp

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
// LFN specific adapt

void ALNAPI AdaptLFN(ALNNODE* pNode, ALN* pALN, const float* adblX, 
                     float dblResponse, BOOL bUsefulAdapt, const TRAINDATA* ptdata)
{
  ASSERT(NODE_ISLFN(pNode));
  ASSERT(LFN_ISINIT(pNode));
  ASSERT(LFN_VARMAP(pNode) == NULL);      // var map not yet supported
  ASSERT(LFN_VDIM(pNode) == pALN->nDim);  // no different sized vectors yet
  ASSERT(NODE_ISEVAL(pNode));
  // constraining region
  ASSERT(NODE_REGION(pNode) >= 0 && NODE_REGION(pNode) < pALN->nRegions);
  ALNREGION& region = pALN->aRegions[NODE_REGION(pNode)];
  // get output var constraint
  int nDim = pALN->nDim;
  // count useful adapts
  if (bUsefulAdapt)
  {
    NODE_RESPCOUNT(pNode)++; 
  }
  if (NODE_ISCONSTANT(pNode))  // IMPORTANT CHANGE: we allow LFN which can't split to still adapt
  {
    return;                     // no adaptation of this constant subtree
  }
  // This procedure adapts the nDim centroid values and the nDim - 1 weight values so that the changes
  // are likely to be individually small but together correct about fraction dblLearnrate of the error
  // of the surface w. r. t. the training point. The error (dblError) is the distance in the output axis 
  // from the smoothed ALN function surface to the sample function value
  // (dbl Error is positive when the ALN function surface is greater than the sample value).
  // We have to think of the error as that of the *surface*, not the data point!
	float dblError = ptdata->dblGlobalError; //This is the error just for this training point
	// notify begining of LFN adapt
	if (CanCallback(AN_LFNADAPTSTART, ptdata->pfnNotifyProc, ptdata->nNotifyMask))
	{
		LFNADAPTINFO lai;
		lai.adblX = adblX;
		lai.pLFN = pNode;
		lai.dblError = dblError;
		lai.dblResponse = dblResponse;
		Callback(pALN, AN_LFNADAPTSTART, &lai, ptdata->pfnNotifyProc, ptdata->pvData);
	}
	if (bUsefulAdapt)
	{
		ASSERT(LFN_SPLIT(pNode) != NULL);
		LFN_SPLIT_COUNT(pNode)++;
		LFN_SPLIT_SQERR(pNode) += dblError * dblError * dblResponse;
		LFN_SPLIT_RESPTOTAL(pNode) += dblResponse;

	// copy LFN vector pointers onto the stack for faster access
		int nDim = LFN_VDIM(pNode);
		ASSERT(nDim == pALN->nDim);
		float* adblW = LFN_W(pNode);	    // weight vector( must be shifted to use the same index as adblC, adblX 
		adblW++; // shift
		float* adblC = LFN_C(pNode);			// centroid vector
		float* adblD = LFN_D(pNode);			// average square dist from centroid vector

		// If response is < 1, then the adjustment of the centroid and weights is lessened.
		// We need two learning rates.  The first is for the centroids and weights, which divides by 2*nDim -1, thus
		// putting them on an equal footing with respect to correcting a share of the error.
		float dblLearnRate = ptdata->dblLearnRate;
		//float dblLearnRespParam = dblLearnRate * dblResponse * region.dblLearnFactor /(double)(2 * nDim - 1); 
		float dblLearnRespParam = dblLearnRate * region.dblLearnFactor / (double)(2 * nDim - 1); // we should remove all smoothing!!!!
		// ADAPT CENTROID FOR OUTPUT
		// L is the value of the affine function of the linear piece at the input components of X
		// V is the value of the sample X[nDim - 1]
		// dblError = L - V,   N.B. if L is greater than the sample the error is positive
		// We neglect the fillet if any and make the average V the target for adblC[nDim - 1]
		adblC[nDim -1] += (adblX[nDim -1] - adblC[nDim - 1]) * dblLearnRespParam;
		// ADAPT CENTROID AND WEIGHT FOR EACH INPUT VARIABLE  
		float dblXmC = 0;
		float dblBend = 0;
		for (int i = 0; i < nDim -1; i++) //Skip the output centroid at nDim - 1.
		{
			// get pointer to variable constraints
			ALNCONSTRAINT* pConstr = GetVarConstraint(NODE_REGION(pNode), pALN, i);
			// skip any variables of constant monotonicity; W is constant and X is irrelevant
			if (pConstr->dblWMax == pConstr->dblWMin) continue;
			// Compute the distance of X from the old centroid in axis i
			dblXmC = adblX[i] - adblC[i];
			// UPDATE VARIANCE BY EXPONENTIAL SMOOTHING
			// We adapt this first so the adaptation of the centroid to this input will not affect it
			ASSERT(adblD[i] >= 0);
			// Use exponential smoothing
			adblD[i] += (dblXmC * dblXmC - adblD[i]) * dblLearnRate; // This learning rate is not involved in correcting dblError
			// The exponentially smoothed estimate of variance
			// adblD[i] is not allowed to go below dblSqEpsilon of the current input variable to
			// slow rotations along some axes and prevent division by 0.
			//if (adblD[i] < pConstr->dblSqEpsilon)adblD[i] = pConstr->dblSqEpsilon; This should not be a problem
			// UPDATE THE CENTROID BY EXPONENTIAL SMOOTHING
			adblC[i] += dblXmC * dblLearnRespParam;
			// Update the distance of X[i] from the centroid (the compiler will optimize these steps)
			dblXmC = adblX[i] - adblC[i];
			// ADAPT WEIGHTS
			adblW[i] -= dblError * dblLearnRespParam * dblXmC / adblD[i];
			// Bound the weight
			adblW[i] = max(min(pConstr->dblWMax, adblW[i]), pConstr->dblWMin);

			// COLLECT DATA FOR LATER SPLITTING THIS PIECE:
			// We analyze the errors of sample value minus ALN value V - L = -dblError (N.B. minus) on the piece which are
			// further from the centroid than  a constant times the stdev of the points on the piece along the current axis.
			// Since adblD[i], averaged over the samples is the variance of the samples in axis i, assuming a uniform distribution
			// the half-width is when dblXmC * dblXmC >= dblConst4Half * D[i] where dblConst4Half = pow(1.5,1.0/3.0) = 1.144712695.
			// If the V - L  is positive (negative) away from the centre compared to the error at the centre, then we need a split of the LFN into a MAX (MIN) node.
			// Even if the fit is extremely bad, we need to know the convexity to split the piece in the right direction, MIN or MAX.
			// To capture the information for all the samples on the piece and all the nDim -1 domain directions, we use exponential smoothing.
			if (LFN_CANSPLIT(pNode) && bUsefulAdapt)
			{
				dblBend = (dblXmC * dblXmC > 1.144712695F * adblD[i]) ? -dblError : dblError;
				LFN_SPLIT_T(pNode) += (dblBend - LFN_SPLIT_T(pNode)) * dblLearnRate;
			}
		} // end loop over all nDim-1 domain dimensions

		// compress the weighted centroid info into W[0]       
		float * const pdblW0 = LFN_W(pNode);
		*pdblW0 = adblC[nDim - 1];
		for (int i = 0; i < nDim - 1; i++)
		{
			*pdblW0 -= adblW[i] * adblC[i]; // here the adblW pointer is still shifted up by one float
		}
		// notify end of LFN adapt
		if (CanCallback(AN_LFNADAPTEND, ptdata->pfnNotifyProc, ptdata->nNotifyMask))
		{
			LFNADAPTINFO lai;
			lai.adblX = adblX;
			lai.pLFN = pNode;
			lai.dblError = dblError;
			lai.dblResponse = dblResponse;
			Callback(pALN, AN_LFNADAPTEND, &lai, ptdata->pfnNotifyProc, ptdata->pvData);
		}
	}
}
