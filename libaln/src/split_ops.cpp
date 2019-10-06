// ALN Library (libaln)
// file split_ops.cpp
// 
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

#ifdef ALNDLL
#define ALNIMP __declspec(dllexport)
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// include files
#include <stdafx.h>
#include <alnpp.h>
#include <datafile.h>


// include classes
#include ".\cmyaln.h"
#include "aln.h"

// We use dblRespTotal in two ways and the following definition helps.
#define DBLNOISEVARIANCE dblRespTotal

extern BOOL bStopTraining; // This becomes TRUE and stops training when pieces are no longer splitting.
void setSplitAlpha(ALNDATAINFO* pDataInfo);
void splitControl(ALN* pALN, ALNDATAINFO* pDataInfo);
void zeroSplitValues(ALN* pALN, ALNNODE* pNode);
void splitUpdateValues(ALN * pALN, ALNDATAINFO* pDataInfo);
void doSplits(ALN* pALN, ALNNODE* pNode, double dblLimit);
int ALNAPI SplitLFN(ALN* pALN, ALNNODE* pNode);

// We use the first three fields in ALNLFNSPLIT (declared in aln.h)
// in two different ways: for training and between training intervals.
// The following routines use the SPLIT typedef between trainings, near the end of alntrain.cpp.
// During nMaxEpochs epochs of training, where the hyperplanes in leaf nodes change weights,
// the pieces adapt to the data the best they can. Following that, 
// a decision must be made for each leaf node whether to split or not.
// To do that, the differences of pairs of close sample values in the data
// which are stored in aNoiseSampleTool are adjusted for the current weights of the piece
// to compensate for the samples being in slightly different domain locations.
// Then splitcontrol takes the total square training errors of
// each piece. This is compared to the total of the noise variance samples.
// over the same training points.  If the average square training error is 
// greater than dblLimit times the average of the noise variance samples on the piece, then using
// an F-test, the piece is split because it does not yet fit within the limits of noise.


// Explanation of dblLimit
// dblLimit = 2.59 says that splitting of a linear piece is prevented when the mean square
// training error of a piece becomes less than 2.59 times the average of the noise variance
// samples on it. This value comes from tables of the F-test for d.o.f. > 7 and probability 90%.
// For 90% with 3 d.o.f the value is 5.39, i.e. with fewer d.o.f. training stops sooner
// and the training error will generally be larger than with a lower F-value.
// The values below for adblFconstant35 are interpolated and may not be accurate.
// The values for 25 are the reciprocals of those for 75.

static const double adblFconstant90[13]{ 9.00, 5.39, 4.11, 3.45, 3.05, 2.78, 2.59, 2.44, 2.32, 1.79, 1.61, 1.51, 1.40 };
static const double adblFconstant75[13]{ 3.00, 2.36, 2.06, 1.89, 1.78, 1.70, 1.64, 1.59, 1.55, 1.36, 1.28, 1.24, 1.19 };
static const double adblFconstant50[13]{ 1,1,1,1,1,1,1,1,1,1,1,1,1 };
// The following two have not had any beneficial effect. Who knows when they might be useful?
static const double adblFconstant35[13]{ 0.58, 0.65, 0.70, 0.73, 0.75, 0.77, 0.78, 0.79, 0.80, 0.86, 0.88, 0.90, 0.92 };
static const double adblFconstant25[13]{ 0.333, 0.424, 0.485, 0.529, 0.562, 0.588, 0.610, 0.629, 0.645, 0.735, 0.781, 0.806, 0.840 };
static const double adblFconstant10[13]{ 0.111, 0.185, 0.243, 0.290, 0.327, 0.359, 0.386, 0.410, 0.431, 0.558, 0.621, 0.662, 0.714 };
static double adblF_Alpha[13]{ 1,1,1,1,1,1,1,1,1,1,1,1,1 };

void setSplitAlpha(ALNDATAINFO* pDataInfo)
{
	// If dblMSEorF < 0, this routine is called once before any training.
	double dblLimit = pDataInfo->dblMSEorF;
	if (dblLimit >= 0) return;
	for (int i = 0; i < 13; i++) // We are doing an F-test
	{
		adblF_Alpha[i] = adblFconstant50[i]; // this is the default if 0 > dblLimit >= -25.01
		if (dblLimit <  -9.99)adblF_Alpha[i] = adblFconstant10[i];
		if (dblLimit < -24.99)adblF_Alpha[i] = adblFconstant25[i];
		if (dblLimit < -34.99)adblF_Alpha[i] = adblFconstant35[i];
		if (dblLimit < -49.99)adblF_Alpha[i] = adblFconstant50[i];
		if (dblLimit < -74.99)adblF_Alpha[i] = adblFconstant75[i];
		if (dblLimit < -89.00)adblF_Alpha[i] = adblFconstant90[i];
	}
}
void splitControl(ALN* pALN, ALNDATAINFO* pDataInfo)  // routine
{
	double dblLimit = pDataInfo->dblMSEorF;
	ASSERT(pALN);
	ASSERT(pALN->pTree);
	// initialize all the SPLIT values to zero
	zeroSplitValues(pALN, pALN->pTree);
	// get square errors of pieces on training set and the noise variance estimates
	splitUpdateValues(pALN, pDataInfo);
	// With the above statistics, doSplits recursively determines splits of eligible pieces.
    doSplits(pALN, pALN->pTree, dblLimit);
    // Resetting the SPLIT components to zero by zeroSplitValues is done in alntrain.
}

// Routines that set some fields to zero

void zeroSplitValues(ALN* pALN, ALNNODE* pNode) // routine
{
	// initializes split counters of all leaf nodes before the next training period
	ASSERT(pNode);
	if (NODE_ISMINMAX(pNode))
	{
		zeroSplitValues(pALN, MINMAX_LEFT(pNode));
		zeroSplitValues(pALN, MINMAX_RIGHT(pNode));
	}
	else
	{
		ASSERT(NODE_ISLFN(pNode));
		if (LFN_CANSPLIT(pNode))
		{
			(pNode->DATA.LFN.pSplit)->nCount = 0;
			(pNode->DATA.LFN.pSplit)->dblSqError = 0;
			(pNode->DATA.LFN.pSplit)->DBLNOISEVARIANCE = 0;
		}
	}
}

// Routines that get the training errors and noise variance values.

void splitUpdateValues(ALN * pALN, ALNDATAINFO* pDataInfo) // routine
{
	// Assign the square errors on the training set and the noise variance
	// sample values to the leaf nodes of the ALN.

	double desired = 0;
	double alnval = 0;
	int nDim = pALN->nDim;
	int nDimm1 = nDim - 1;
	int nDimt2 = nDim * 2;
	int nDimt2m1 = nDimt2 - 1;
	int nDimt2p1 = nDimt2 + 1;
	int nDimt2p1ti;
	double* adblX = (double*)malloc(nDim * sizeof(double));
	ALNNODE* pActiveLFN;
	double* adblTRdata = pDataInfo->adblTRdata;
	int nrows = pDataInfo->nTRcurrSamples;
	double dblMSEorF = pDataInfo->dblMSEorF;
	for (long i = 0; i < nrows; i++)
	{
		nDimt2p1ti = nDimt2p1 * i;
		for (int j = 0; j < nDimm1; j++) // just the domain values of the sample
		{
			adblX[j] = adblTRdata[nDimt2p1ti + j];
		}
		adblX[nDimm1] = 0; // hide the desired value (not used anyway)
		alnval = ALNQuickEval(pALN, adblX, &pActiveLFN); // the current ALN value
		if (LFN_CANSPLIT(pActiveLFN)) // Skip this leaf node if it can't split anyway.
		{
			double noiseSampleTemp;
			desired = adblTRdata[nDimt2p1ti + nDimm1];
			(pActiveLFN->DATA.LFN.pSplit)->nCount++;
			(pActiveLFN->DATA.LFN.pSplit)->dblSqError += (alnval - desired) * (alnval - desired);
			if (dblMSEorF <= 0)
			{
				noiseSampleTemp = adblTRdata[nDimt2p1ti + nDimt2m1]; // Get the difference of desired sample values in the tool
				// This has to be corrected for the slopes of the LFN
				for (int kk = 0; kk < nDim - 1; kk++) // Just do the domain dimensions.
				{
					// get the weights for the LFN and correct the sample for slope
					// Adding 1 in kk + 1 skips the bias weight.
					noiseSampleTemp -= LFN_W(pActiveLFN)[kk + 1] * adblTRdata[nDimt2p1ti + nDim + kk];
				}
				// The following should be a sample for the noise variance of the piece
				// which is paired with data sample i in case dblMSEorF is negative and we are doing an F-test.
				(pActiveLFN->DATA.LFN.pSplit)->DBLNOISEVARIANCE += 0.5 * noiseSampleTemp * noiseSampleTemp;
			}
		}
	} // end loop over both files
	free(adblX);
} // END of splitUpdateValues

void doSplits(ALN* pALN, ALNNODE* pNode, double dblMSEorF) // routine
{
	// This routine visits all the leaf nodes and determines whether or not to split.
	// If dblLimit < 0, it uses an F test with d.o.f. based on the number of samples counted,
	// but if dblLimit >= 0 it uses the actual dblLimit value to compare to the square training error.
	int nDim = pALN->nDim;
	int nDimt2 = nDim * 2; // The idea is that if we don't have at least this number of points, 
	// then we can't have two pieces with at least nDim samples and none shared
	ASSERT(pNode);
	if (NODE_ISMINMAX(pNode))
	{
		doSplits(pALN, MINMAX_LEFT(pNode), dblMSEorF);
		doSplits(pALN, MINMAX_RIGHT(pNode), dblMSEorF);
	}
	else
	{
		ASSERT(NODE_ISLFN(pNode));
		if (LFN_CANSPLIT(pNode))
		{
			long Count = (pNode->DATA.LFN.pSplit)->nCount;
			if (Count > nDim && Count > 15) // There are enough samples on the piece to consider splitting
			{
				double dblPieceSquareTrainError = (pNode->DATA.LFN.pSplit)->dblSqError; // total square error on the piece
				double dblPieceNoiseVariance = (double)Count; // Used when there is no F-test.
				double dblSplitLimit = dblMSEorF; // if dblLimit is <= 0, otherwise they test training MSE < dblLimit
				if (dblMSEorF < 0) // if this is TRUE, we do the F test.
				{
					dblPieceNoiseVariance = (pNode->DATA.LFN.pSplit)->DBLNOISEVARIANCE; // total of noise variance samples
					//the average of noise variance samples estimates the actual noise variance.
					int dofIndex; // get the dblSplitLimit corresponding to the degrees of freedom of the F test
					dofIndex = Count - 2;
					if (Count > 10) dofIndex = 8;
					if (Count > 20) dofIndex = 9;
					if (Count > 30) dofIndex = 10;
					if (Count > 40) dofIndex = 11;
					if (Count > 60) dofIndex = 12; // MYTEST  encourage splitting worked, now it's too much
					dblSplitLimit = adblF_Alpha[dofIndex]; // One can reject the H0 of a good fit with various percentages
					// 90, 75, 50, 35, 25. E.g. 90% says that if the training error is greater than the dblSplitLimit prescribes
					// it is 90% sure that the fit is bad.  A higher percentage needs less training time.
					// Note that when there are few hits on the piece, the dblSplitLimit is larger and 
					// the criterion for fitting well enough is easier to satisfy.
				}
				else
				{
					dblSplitLimit = dblMSEorF;
				}

				if (dblPieceSquareTrainError > dblPieceNoiseVariance * dblSplitLimit)
				{
					// The piece doesn't fit and needs to split; then training must continue.
					SplitLFN(pALN, pNode); // We split *every* leaf node that reaches this point.
					// We start an epoch with bStopTraining == TRUE, but if any leaf node might still split,
					bStopTraining = FALSE; //  we set it to FALSE and continue to another epoch of training.
				}
				else
				{
					// The piece fits well enough and doesn't need to split or train
					//LFN_FLAGS(pNode) &= ~LF_SPLIT;  // this flag setting prevents further splitting   MYTEST
					// The problem here is adjoining pieces become responsible for the rest of the fit.
				}
			}
			else
			{
				// The piece has at most 2 * nDim -1 samples on it, stop splitting it. 
				//LFN_FLAGS(pNode) &= ~LF_SPLIT;  // this flag setting prevents further splitting
				// It may still need to train
				bStopTraining = FALSE; //  we set it to FALSE and continue to another epoch of training.
			}
		}
	}
}

int ALNAPI SplitLFN(ALN* pALN, ALNNODE* pNode)
{
	// We only split if the piece doesn't fit within the noise limit
	// and the direction of split will likely not be close
	if (LFN_SPLIT_T(pNode) < 0) // This "<" is TRUE if the values of the samples
		// in the training data are higher than the LFN surface some distance from the centroid.
		// This causes the LFN to split into a MAX of two LFNs.
	{
		return ALNAddLFNs(pALN, pNode, GF_MAX, 2, NULL);
		// A max is convex down:  \/,  \_/ etc.
	}
	else
	{
		return ALNAddLFNs(pALN, pNode, GF_MIN, 2, NULL);
		// A min of several LFNs is like a dome.
	}
}

//ALNDATAINFO* GetDataInfo()  MYTEST WHAT IS THIS ROUTINE DOING HERE?????
//{
//	return nullptr;
//}
