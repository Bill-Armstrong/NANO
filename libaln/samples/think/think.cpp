// NANO program
// This program controls the libaln NANO library.  The user has to input the name of the data file and
// the columns to be used as inputs and the (single) output. The dblMSEorF value if negative, uses an F-test
// to decide whether to break a linear piece (a multi-dimensional hyperplane in general) into two separate
// hyperplanes connected by a max or min operator. Otherwise a positive level of Mean Square Training Error
// below which the pieces will not break can be set.


#ifdef __GNUC__
#include <typeinfo>
#endif

#include "alnpp.h"
#include "datafile.h"
#include "cmyaln.h"
#include "alnpriv.h"
#include <iostream>
#include <string>

extern BOOL bStopTraining;

static char szInfo[] = "NANO (Noise-Attenuating Neuron Online) program\n"
"Copyright (C)  2018 William W. Armstrong\n"
"Licensed under LGPL\n\n";

double dblTrainErr;
int nMaxEpochs;
int nNumberLFNs;
auto const MSEORF = 0.33333;


int main()
{
	CDataFile file;
	// INSERT INPUT FILE PATH AND NAME
	if (file.Read("SineNoisy.txt")) // Important: this file can't have headers; it is all doubles
	{
		std::cout << "Succeeded!" << std::endl;
	}
	else
	{
		std::cout << "Failed!" << std::endl;
		return 1;
	}
	int nTRmaxSamples = file.RowCount();
	int nCols = file.ColumnCount();
	// SPECIFY HOW MANY OF THE COLUMNS YOU WANT TO USE IN TRAINING (NUMBER OF INPUTS PLUS ONE OUTPUT)?
	const int nDim = 2;
	int nTRcols = 2 * nDim + 1; // The nDim rows of sample data in adblTRdata
	 //are extended to 2 * nDim + 1 total rows for the noise-attenuation tool.

	// create ALN 
	std::cout << "Creating ALN";
	CMyAln aln;
	if (!aln.Create(nDim, nDim - 1))
	{
		std::cout << "failed!" << std::endl;
		return 1;
	}
	else
	{
		std::cout << "succeeded!" << std::endl;
	}
	CMyAln* pALN = &aln;
	ALNNODE* pTree = pALN->GetTree();
	pALN->SetGrowable(pTree);
	ALNREGION* pRegion = pALN->GetRegion(0);
	pRegion->dblSmoothEpsilon = 0;
	SetSmoothingEpsilon(pRegion); // Should all smoothing be removed from the library?????? Maybe when more speed is needed.
	// DO WE USE THE F-TEST??
	double dblMSEorF = MSEORF; // a negative value indicates use of an F-test to stop splitting
	// This sets up the training buffer of doubles adblTRbuffer. The F-test is specified in split_ops.cpp
	ALNDATAINFO* pdata = pALN->GetDataInfo();
	pdata->nTRmaxSamples = nTRmaxSamples;
	pdata->nTRcurrSamples = 0;
	pdata->nTRcols = nTRcols;
	pdata->nTRinsert = 0;
	pdata->dblMSEorF = dblMSEorF;
	//SPECIFY FOR EACH ALN INPUT THE COLUMN OF THE DATA FILE USED (LAST IS THE DESIRED OUTPUT EXCEPT FOR NOISE)
	int ColumnNumber[nDim]{0,2}; // {0, 1 } tests to see what happens with no noise in the VeryNoisy.txt example.
	//load the buffer; as the buffer gets each new sample, it is compared to existing samples to determine noise variance
	double* adblX = (double*)malloc(nDim * sizeof(double));
	for (int i = 0; i < nTRmaxSamples; i++)
	{
		for (int j = 0; j < nDim; j++)
		{
			adblX[j] = file.GetAt(i, ColumnNumber[j], 0);
		}
		pALN->addTRsample(pALN, adblX, nDim);
	}

	std::cout << "Data for ALNDATAINFO after loading: " << std::endl;
	std::cout << "\nnTRmaxSamples = " << pdata->nTRmaxSamples << "  "
		<< "\nnTRcurrSamples = " << pdata->nTRcurrSamples << "  "
		<< "\nnTRcols  = " << pdata->nTRcols << "  "
		<< "\nnTRinsert = " << pdata->nTRinsert << std::endl;
	int nDimt2 = nDim *2;
	int nDimt2p1 = nDimt2 + 1;
	double averageNVsample = 0;
	// Print part of the buffer contents to check
	int nBegin = 0;
	for (int ii = nBegin; ii < nBegin + 629; ii++) // Just a sampling of the file
	{
		for (int jj = 0; jj < nDimt2p1; jj++)
		{
			if (jj == nDim) std::cout << "closest is ";
			if (jj == nDimt2) std::cout << " square dist = ";
			std::cout << pdata->adblTRdata[nDimt2p1 * ii + jj] << " ";
		}
		std::cout << std::endl;
		averageNVsample += 0.5 * pow(pdata->adblTRdata[nDimt2p1 * ii + nDimt2 - 1], 2);
	}
	averageNVsample /= 630;
	std::cout << "\n\n Average noise difference = " << averageNVsample << endl;
	BOOL bJitter = FALSE;
	bStopTraining = FALSE;
	double dblLearnRate = 0.2;  // small learning rate
	double dblMinRMSE = 0.00001;// This is set small and not very useful.  dblMSEorF is better.
	int nMaxEpochs = 20; // 20 epochs give enough time for pieces to move into position for splitting in the last epoch.
	int nNotifyMask = AN_TRAIN | AN_EPOCH; // required callbacks for information or insertion of data
	for (int iteration = 0; iteration < 200; iteration++)
	{
		if (!pALN->Train(nMaxEpochs, dblMinRMSE, dblLearnRate, bJitter, nNotifyMask))
		{
			std::cout << "Training failed!" << std::endl;
			return 1;
		}
		else
		{
			std::cout << "Training succeeded!" << std::endl;
		}
		// bStop Training is used because there is splitting
		if (bStopTraining)
		{
			std::cout << "Training was stopped because there was no more splitting of linear pieces." << std::endl;
			break;
		}
		if (iteration == 160) dblLearnRate = 0.1; // Causes less jiggling around of the pieces as training is closing in on the solution.
		if (iteration == 180) dblLearnRate = 0.05;
	}
	CDataFile ExtendTR;
	ExtendTR.Create(file.RowCount(), 1 + nCols);
	double entry;
	ALNNODE** ppActiveLFN = NULL;
	ALNDATAINFO* pdata2 = pALN->GetDataInfo();
	for (int i = 0; i < nTRmaxSamples; i++)
	{
		for (int j = 0; j < nCols; j++) // we include the correct function in the second column for checkin accuracy
		{
			entry = file.GetAt(i, j, 0);
			ExtendTR.SetAt(i, j, entry, 0);
		}
		adblX[0] = file.GetAt(i, 0, 0);
		adblX[1] = file.GetAt(i, 2, 0);
		entry = pALN->QuickEval(adblX, ppActiveLFN); // get the ALN-computed value.
		ExtendTR.SetAt(i, nCols, entry, 0);
	}
	ExtendTR.Write("ExtendedTR.txt");
}
