// NANO program
// This program controls the libaln NANO library


#ifdef __GNUC__
#include <typeinfo>
#endif

#include "alnpp.h"
#include "datafile.h"
#include "cmyaln.h"
#include <iostream>
#include <string>

extern BOOL bStopTraining;

static char szInfo[] = "NANO (Noise-Attenuating Neuron Online) program\n"
"Copyright (C)  2018 William W. Armstrong\n"
"Licensed under LGPL\n\n";

double dblTrainErr;
int nMaxEpochs;
int nNumberLFNs;


int main()
{
	CDataFile file;
	// INSERT INPUT FILE PATH AND NAME
	if (file.Read("VeryNoisy.txt"))
	{
		std::cout << "Succeeded!" << std::endl;
	}
	else
	{
		std::cout << "Failed!" << std::endl;
		return 1;
	}
	int nTRmaxSamples = file.RowCount();
	// SPECIFY HOW MANY OF THE COLUMNS YOU WANT TO USE IN TRAINING (NUMBER OF INPUTS PLUS ONE OUTPUT)?
	const int nDim = 2;
	int nTRcols = 2 * nDim + 1; // The rows of data are extended for the noise-attenuation tool.

	// seed ALN random number generator
	CAln::SRand(57);

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
	double dblMSEorF = 0.0001;
	// This sets up the training buffer TRbuffer
	ALNDATAINFO* pdata = pALN->GetDataInfo();
	pdata->nTRmaxSamples = nTRmaxSamples;
	pdata->nTRcurrSamples = 0;
	pdata->nTRcols = nTRcols;
	pdata->nTRinsert = 0;
	pdata->dblMSEorF = dblMSEorF;
	//SPECIFY FOR EACH ALN INPUT THE COLUMN OF THE DATA FILE USED (LAST IS THE DESIRED OUTPUT EXCEPT FOR NOISE)
	int ColumnNumber[nDim]{0,2}; // {0, 1 } tests to see what happens with no noise.
	//load the buffer
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
	int nDimt2 = 2 * nDim;
	int nDimt2p1 = nDimt2 + 1;
	// Print the buffer contents
	for (int ii = 0; ii < 3; ii++) // Just a sampling of the file
	{
		for (int jj = 0; jj < nDimt2p1; jj++)
		{
			std::cout << pdata->adblTRdata[ii * nDimt2p1 + jj] << " ";
			if (jj == 1) std::cout << "closest, square dist = ";
		}
		std::cout << std::endl << std::endl;
	}
	BOOL bJitter = FALSE;
	double dblLearnRate = 0.1;  // small learning rate
	double dblMinRMSE = 0.00001;
	int nMaxEpochs = 2;
	int nNotifyMask = AN_TRAIN | AN_EPOCH;
	for (int ii = 0; ii < 100; ii++)
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
		if(bStopTraining) std::cout << "Training was stopped because there was no more splitting of linear pieces." << std::endl;
	}
	CDataFile ExtendTR;
	ExtendTR.Create(2000, 4);
	double entry;
	ALNNODE** ppActiveLFN = NULL;
	ALNDATAINFO* pdata2 = pALN->GetDataInfo();
	for (int i = 0; i < 2000; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			entry = file.GetAt(i, j, 0);
			ExtendTR.SetAt(i, j, entry, 0);
		}
		adblX[0] = file.GetAt(i, 0, 0);
		adblX[1] = file.GetAt(i, 2, 0);
		entry = pALN->QuickEval(adblX, ppActiveLFN);
		ExtendTR.SetAt(i, 3, entry, 0);
	}
	ExtendTR.Write("ExtendedTR.txt");
}
