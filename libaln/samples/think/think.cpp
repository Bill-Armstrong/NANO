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
"Copyright (C)  2019 William W. Armstrong\n"
"Licensed under LGPL\n\n";

double dblTrainErr;
int nMaxEpochs;
int nNumberLFNs;
// the following can be a positive square error limit on training or a negative number
// special cases are -25 -35 -50 -75 -90. -50 is the same as -1 or any number <0 and > -24.99
// -75 and -90 likely stop training before a perfect fit and -35 and -25 may tend to overfit.
auto const MSEORF = -75.0;
void setSplitAlpha(ALNDATAINFO* pdata);


int main(int argc, char* argv[])
{
	// example input: think(VeryNoisy_2.txt, 2)
	std::cout << argv[1] << "  " << argv[2] << "  " << argv[3] << "  " << argv[4] << " " << argv[5] << endl;
	if (argc != 6) std::cout << endl;

	if (argc != 6) // We expect 5 arguments as listed here (argc is not counted among them
	{
		std::cout << "Bad argument list!\n" << "Usage: " << "Data_file_name nDim nMaxEpochs dblMSEorF SKIPCOL" << std::endl;
		return 1;
	}
	CDataFile file;
	// INSERT INPUT FILE PATH AND NAME
	if (file.Read(argv[1])) // Important: this file can't have headers; it is all doubles
	{
		std::cout << "Reading file " << argv[1] << " Succeeded!" << std::endl;
	}
	else
	{
		std::cout << "Reading file failed!" << std::endl;
		return 1;
	}
	int nTRmaxSamples = file.RowCount();
	int nCols = file.ColumnCount();
	// SPECIFY HOW MANY OF THE COLUMNS YOU WANT TO USE IN TRAINING (NUMBER OF INPUTS PLUS ONE OUTPUT)?
	int nDim = atoi(argv[2]);
	nMaxEpochs = atoi(argv[3]);
	int SKIPCOL = atoi(argv[5]);
	int nTRcols = 2 * nDim + 1; // The nDim columns of sample data in adblTRdata
	 //are extended to 2 * nDim + 1 total columns for the noise-attenuation tool.

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
	double dblMSEorF = atof(argv[4]); // a negative value indicates use of an F-test to stop splitting
	// This sets up the training buffer of doubles adblTRbuffer. The F-test is specified in split_ops.cpp
	ALNDATAINFO* pdata = pALN->GetDataInfo();
	pdata->nTRmaxSamples = nTRmaxSamples;
	pdata->nTRcurrSamples = 0;
	pdata->nTRcols = nTRcols;
	pdata->nTRinsert = 0;
	pdata->dblMSEorF = dblMSEorF;
	if (dblMSEorF < 0) setSplitAlpha(pdata);
	//SPECIFY FOR EACH ALN INPUT THE COLUMN OF THE DATA FILE USED (LAST IS THE DESIRED OUTPUT (corrupted by noise)
	int* ColumnNumber = (int*)malloc(nDim * sizeof(int));
	int col = 0; // file columns are numbered from the left starting at 0 (no lags allowed)
	for (int ALNinput = 0; ALNinput < nDim; ALNinput++)
	{
		if (col == SKIPCOL) col++; // skip col and go to the next file column; additional lines like this for other skipped columns
		ColumnNumber[ALNinput] = col; // N.B. if columns are skipped, this has to be corrected above
		col++;
	}	
	//load the buffer; as the buffer gets each new sample, it is compared to existing samples to determine noise variance
	double* adblX = (double*)malloc(nDim * sizeof(double));
	for (int i = 0; i < nTRmaxSamples; i++)
	{
		for (int j = 0; j < nDim; j++)
		{
			adblX[j] = file.GetAt(i, ColumnNumber[j], 0);
		}
		pALN->addTRsample(adblX, nDim);
	}
	// reduce the noise variance, ideally by half 
	//pALN->reduceNoiseVariance();
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
	int nLinesPrinted = 5;
	for (int ii = nBegin; ii < nBegin + nLinesPrinted; ii++) // Just a sampling of the file
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
	int nNotifyMask = AN_TRAIN | AN_EPOCH; // required callbacks for information or insertion of data
	for (int iteration = 0; iteration < 50; iteration++)
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
		for (int k = 0; k < nDim - 1; k++) // Get the domain coordinates
		{
			adblX[k] = file.GetAt(i, ColumnNumber[k], 0);
		}
		entry = pALN->QuickEval(adblX, ppActiveLFN); // get the ALN-computed value at that place
		ExtendTR.SetAt(i, nCols, entry, 0);
	}
	ExtendTR.Write("ExtendedTR.txt");
}
