// NANO program
// This program controls the libaln NANO library.  The user has to input the name of the data file and
// the columns to be used as inputs and the (single) output. The fltMSEorF value if negative, uses an F-test
// to decide whether to break a linear piece (a multi-dimensional hyperplane in general) into two separate
// hyperplanes connected by a max or min operator. If an F-test is not used, a positive level of Mean
// Square Training Error, below which the pieces will not break, can be set.

#ifdef __GNUC__
#include <typeinfo>
#endif
#include "aln.h"
#include "alnpp.h"
#include "datafile.h"
#include "cmyaln.h"
#include "alnpriv.h"
#include <iostream>
#include <string>
#include <chrono>  // for high_resolution_clock

long CountLeafevals; // Global to test optimization
extern BOOL bStopTraining;
// Switches for turning on/off optimizations
BOOL bAlphaBeta = FALSE;
BOOL bDistanceOptimization = FALSE;
static char szInfo[] = "NANO (Noise-Attenuating Neuron Online) program\n"
"Copyright (C)  2019 William W. Armstrong\n"
"Licensed under LGPL\n\n";
float fltTrainErr;
int nMaxEpochs;
int nNumberLFNs;
// fltMSEorF can be a positive square error limit on training below which pieces won't split, or a negative number calling for an F-test.
// This is a local stopping criterion comparable to the global stopping criteria for other neural nets using a validation set.
// Special cases are -25 -50 -75 where the values come from tables; the other values are approximate, calculated by a simple formula.
// F-tests for -75 and -90 are likelyto stop training before a perfect fit, and -35 and -25 may prolong training and tend to overfit.
// A negative value causes *much* slower loading of the training buffer.
auto const MSEORF = -50; // This has to be changed below for non-regression problems
void setSplitAlpha(ALNDATAINFO* pdata); // This works in the split routine to implement an F-test.
void DecayWeights(const ALNNODE* pNode, const ALN* pALN, float WeightBound, float WeightDecay);
float WeightDecay; //  This is a factor like 0.7599. It is used during classification into two classes when lower weights give better generalization.
float WeightBound;
float WeightBoundIncrease = 0.000008; // increase every iteration, e.g. in 1000 iterations it can go from 0.0004 to 0.01
float targetDigit = 2; //  Here we specify the digit to be recognized. The other digits are all in the second class.
int iterations = 600; // Initial iterations
BOOL bOneClass = FALSE; // This is for recognizing images of a class when not given and samples not in the set.
BOOL bClassify2 = TRUE; // FALSE produces the usual function learning; TRUE is for 2 class classification with a target class



int main(int argc, char* argv[])
{
	// The first four arguments are the data file name, nDim (number of ALNinputs including one for the output),
	// nMaxEpochs value (training epochs including one epoch of splitting), the fltMSEorF value for stopping splitting, and the weight bound,
	// Example input: think(VeryNoisySine.txt, 2, 20, 0.1, 12) or think(NoisySinCos20000.txt, 3, 3, -90, 0.5).
	// When the program is executed from Visual Studio, the above inputs can be specified in the properties of the think project, Debugging>>
	// Command Arguments with quotes around the file name and spaces instead of commas.
	// The working directory can also be defined there.
	// This version of the program doesn't allow skipping input file columns because it has to handle a large number of ALN inputs.
	std::cout << argc << " " << argv[1] << " " << argv[2] << " " << argv[3] << " " << argv[4] << " " << argv[5] << " " << argv[6]<< std::endl;
	int argCount = argc;
	int nDim = atoi(argv[2]);

	if (argc != 7 ) // We expect arguments as listed here (argc is not counted among them)
	{
		std::cout << "Bad argument list!\n" << "Usage: " << "Data_file_name nDim nMaxEpochs fltRMSEorF WeightBound Downshift  " << std::endl;
		return 1;
	}
	CDataFile file;
	// INSERT INPUT FILE PATH AND NAME
	if (file.Read(argv[1])) // Important: this file can't have headers; it is all floats
	{
		std::cout << "Reading file " << argv[1] << " Succeeded!" << std::endl;
	}
	else
	{
		std::cout << "Reading file failed!" << std::endl;
		return 1;
	}
	long nTRmaxSamples = file.RowCount(); 
	int nCols = file.ColumnCount();
	nMaxEpochs = atoi(argv[3]);
	float fltRMSEorF = (float) atof(argv[4]); // a negative value indicates use of an F-test to stop splitting, intuition understands fltRMSEorF, but if >0 we use the square
	float fltMSEorF = fltRMSEorF > 0 ? pow(fltRMSEorF, 2) : fltRMSEorF;
	WeightBound = (float) atof(argv[5]); // Plus or minus this value bounds the weights from above and below in cases where all inputs have the same characeristics
	int* ColumnNumber = (int*)malloc(nDim * sizeof(int));
	for (int ALNinput = 0; ALNinput < nDim; ALNinput++)
	{
		ColumnNumber[ALNinput] = ALNinput;
	}
	int nTRcols = 2 * nDim + 1; // The nDim columns of sample data in afltTRdata are extended 
								// to 2 * nDim + 1 total columns for the noise-attenuation tool.
	WeightDecay = atof(argv[6]);

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
	ALNNODE* pTree = pALN->GetTree(); // The tree is initially just one leaf node
	pALN->SetGrowable(pTree);

	if (bClassify2 && bOneClass)
	{
		for (int i = 1; i < nDim; i++)
		{
			LFN_W(pTree)[i] = -WeightBound;
		}
	}


	if (bClassify2 && !bOneClass)
	{
		// The following sets up the special ALN structure for pattern classification into two classes denoted by 1.0 and -1.0
		// Split the root
		ALNAddLFNs(aln, pTree, GF_MIN, 2, NULL);
		ALNNODE* pChildR = MINMAX_RIGHT(pTree);
		ALNNODE* pChildL = MINMAX_LEFT(pTree);
		ASSERT(NODE_ISLFN(pChildR));
		ASSERT(NODE_ISLFN(pChildL));
		// Now split the left child
		ALNAddLFNs(aln, pChildL, GF_MAX, 2, NULL);
		ALNNODE* pGChildR = MINMAX_RIGHT(pChildL);
		ALNNODE* pGChildL = MINMAX_LEFT(pChildL);
		ASSERT(NODE_ISLFN(pGChildR));
		ASSERT(NODE_ISLFN(pGChildL));
		// The next few lines set up a minimum with 0.95 and a maximum with -0.95 for the classification problems
		// This assures that all ALN outputs are in the interval [-0.95, 0.95]
		// Set up pChildR to cut off the ALN values above +1.0 using the minimum		
		for (int i = 0; i < nDim; i++)
		{
			LFN_W(pChildR)[i] = 0;
			LFN_C(pChildR)[i] = 0;
			LFN_D(pChildR)[i] = 0.0001; // just not zero and not enough to destroy optimization
		}
		LFN_W(pChildR)[0] = 0.95;
		LFN_W(pChildR)[nDim] = -1.0; // this is the weight for the output(0 is for the bias weight, there is a shift by one unit)
		LFN_C(pChildR)[nDim - 1] = 0.95;
		LFN_FLAGS(pChildR) |= NF_CONSTANT; // Don't allow the LFN to adapt
		LFN_FLAGS(pChildR) &= ~LF_SPLIT; //Don't allow the new right leaf to split
		// Set up the right grandchild pGChild to cut off the ALN values below -1.0 using the maximum
		for (int i = 0; i < nDim; i++)
		{
			LFN_W(pGChildR)[i] = 0;
			LFN_C(pGChildR)[i] = 0;
			LFN_D(pGChildR)[i] = 0.0001; // just not zero and not enough to destroy optimization
		}
		LFN_W(pGChildR)[0] = -0.95;
		LFN_W(pGChildR)[nDim] = -1.0; // this is the weight for the output(0 is for the bias weight, there is a shift by one unit)
		LFN_C(pGChildR)[nDim - 1] = -0.95;
		LFN_FLAGS(pGChildR) |= NF_CONSTANT;
		LFN_FLAGS(pGChildR) &= ~LF_SPLIT; //Don't allow the new right leaf to split
		// The left grandchild should be growable, non-constant, splittable, we set it to be flat at 1.0
		for (int i = 0; i < nDim; i++)
		{
			LFN_W(pGChildL)[i] = -0;
			LFN_C(pGChildL)[i] = 0;
			LFN_D(pGChildL)[i] = 0.0001; // just not zero and not enough to destroy optimization
		}
		LFN_W(pGChildL)[0] = 0.0;
		LFN_W(pGChildL)[nDim] = -1.0; // this is the weight for the output(0 is for the bias weight, there is a shift by one unit)
		LFN_C(pGChildL)[nDim - 1] = 0.0;
	} // End of special code for one- or two-class pattern classification

	ALNREGION* pRegion = pALN->GetRegion(0);
	pRegion->fltSmoothEpsilon = 0;
	// Restrictions on weights (0 is the region -- the region concept is not fully implemented)
	for (int m = 0; m < nDim-1; m++)
	{
		pALN->SetWeightMin(-WeightBound, m, 0);
		pALN->SetWeightMax(WeightBound, m, 0); // MYTEST  try all weights negative
	}
	// This sets up the training buffer of floats afltTRbuffer. The F-test is specified in split_ops.cpp
	ALNDATAINFO* pdata = pALN->GetDataInfo();
	pdata->nTRmaxSamples = nTRmaxSamples;
	pdata->nTRcurrSamples = 0;
	pdata->nTRcols = nTRcols;
	pdata->nTRinsert = 0;
	pdata->fltMSEorF = fltMSEorF;
	// The following sets the alpha for the F-test
	if (fltMSEorF < 0) setSplitAlpha(pdata);
	std::cout << "Loading the data buffer ... please wait" << std::endl;
	// Load the buffer; as the buffer gets each new sample, it is compared to existing samples to create a noise variance tool.
	// During training, once the weights of a piece are known, the noise variance can be estimated.
	float* afltX = (float*)malloc(nDim * sizeof(float));
	int colno;
	long samplesAdded = 0;
	float temp;
	for (long i = 0; i < nTRmaxSamples; i++)
	{
		// get the sample
		for (int j = 0; j < nDim; j++)
		{
			colno = ColumnNumber[j];
			afltX[j] = file.GetAt(i, colno, 0);
		}
		if (bClassify2)
		{
			temp = afltX[nDim - 1]; // Replace the desired value by +1.0 for the target, -1.0 for the others.
			afltX[nDim - 1] = (fabs(temp - targetDigit) < 0.1) ? 1.0 : -1.0;
		}
		if (!bOneClass || (afltX[nDim - 1] > 0)) pALN->addTRsample(afltX, nDim);  // skip putting all the non-target samples into the buffer if doing OneClass
	}
	ASSERT(pdata->nTRcurrSamples == samplesAdded);
	std::cout << "ALNDATAINFO: " << "TRmaxSamples = " << pdata->nTRmaxSamples << "  "
		<< "TRcurrSamples = " << pdata->nTRcurrSamples << "  "	<< "TRcols  = " << pdata->nTRcols << "  " << "TRinsert = " << pdata->nTRinsert << std::endl;
	int nDimt2 = nDim *2;
	int nDimt2p1 = nDimt2 + 1;
	float averageNVsample = 0;

	/*
	// Print part of the buffer contents to check
	int nBegin = 0;
	int nLinesPrinted = 15;
	for (int ii = nBegin; ii < nBegin + nLinesPrinted; ii++) // Just a sampling of the file
	{
		for (int jj = 0; jj < nDimt2p1; jj++)
		{
			if (jj == nDim) std::cout << "closest is ";
			if (jj == nDimt2) std::cout << " square dist = ";
			std::cout << pdata->afltTRdata[nDimt2p1 * ii + jj] << " ";
		}
		std::cout << std::endl;
		averageNVsample += 0.5 * pow(pdata->afltTRdata[nDimt2p1 * ii + nDimt2], 2); // MYTEST is this the right entry?
	}
	averageNVsample /= nLinesPrinted;
	std::cout << "\n\n Average noise difference = " << averageNVsample << std::endl;
	// For 6000 MNIST training samples, the average noise distance was on the order of millions.
	*/

	BOOL bJitter = FALSE;
	bStopTraining = FALSE;
	bAlphaBeta = FALSE;
	bDistanceOptimization = FALSE;
	float fltLearnRate = 0.2F;
	float fltMinRMSE = 0.00000001F;// This is set small and not very useful.  fltMSEorF is used now to stop training.
	int nNotifyMask = AN_TRAIN; // required callbacks for information or insertion of data. You can OR them together with |
	ALNNODE** ppActiveLFN = NULL;


	std::cout << std::endl << "Starting training " << std::endl;
	// Record start time
	auto start_training = std::chrono::high_resolution_clock::now();
	do
	{
		for (int iteration = 1; iteration <= iterations; iteration++)
		{
			if (iteration == 1 || iteration % 5 == 0) std::cout << "\nIteration " << iteration << " (of " << iterations << " )   ";
			flush(std::cout);
			auto start_iteration = std::chrono::high_resolution_clock::now();
			// VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV  Training!

			//pTree = pALN->GetTree();
			if (!pALN->Train(nMaxEpochs, fltMinRMSE, fltLearnRate, bJitter, nNotifyMask))
			{
				std::cout << " Training failed!" << std::endl;
				flush(std::cout);
				return 1;
			}

			// Every iteration increases the WeightBound
			for (int m = 0; m < nDim - 1; m++) // The slopes of the LFNs discriminating classes are not known in advance, so this step reduces the effect of the initial choice.
			{
				pALN->SetWeightMin(-WeightBound, m, 0);
				pALN->SetWeightMax(WeightBound, m, 0);
			}
			WeightBound += WeightBoundIncrease;

			if (iteration == 15 ) // Optimization is not needed when there are few leaf nodes, e.g. < 256
			{
				bAlphaBeta = TRUE; // Once these optimizatons are switched on, they stay on. (This is after the first split)  MYTEST
				bDistanceOptimization = TRUE;
			}
		
			auto finish_iteration = std::chrono::high_resolution_clock::now();
			std::chrono::duration<float> elapsed0 = finish_iteration - start_training;
			std::chrono::duration<float> elapsed1 = finish_iteration - start_iteration;
			if (iteration == 1 || iteration % 5  == 0)
			{
				std::cout << " Duration " << elapsed1.count() << " Elapsed time " << elapsed0.count() << " seconds" << std::endl;
				flush(std::cout);
			}
		}

		// OUTPUT OF RESULTS
		std::cout << std::endl << "Writing output file... please wait" << std::endl;
		CDataFile ExtendTR;
		int colsOut = (bClassify2 ? nCols + 2 : nCols + 1);
		ExtendTR.Create(file.RowCount(), colsOut); // Create a data file to hold the results
		float entry;
		long countcorrect = 0;
		long counterrors = 0;
		float desired;
		ALNDATAINFO* pdata2 = pALN->GetDataInfo();
		for (long i = 0; i < nTRmaxSamples; i++)
		{
			// Copy the row of the input data file
			for (int j = 0; j < nCols; j++) // we include all columns
			{
				entry = file.GetAt(i, j, 0);
				ExtendTR.SetAt(i, j, entry, 0);
			}
			// Evaluate the ALN on this row
			for (int k = 0; k < nDim; k++) // Get the domain coordinates and desired output
			{
				afltX[k] = file.GetAt(i, ColumnNumber[k], 0);
			}
			desired = afltX[nDim - 1]; // Store the desired output according to the training file
			afltX[nDim - 1] = -250; // Be sure the desired output will not help in the computation  MYTEST does this do it????
			entry = pALN->QuickEval(afltX, ppActiveLFN); // get the ALN-computed value for entry into the output file
			ExtendTR.SetAt(i, nCols, entry, 0); // the first additional column on the right is the ALN output
			float correctClass;
			if(bClassify2)
			{
				correctClass = (abs(desired - targetDigit) < 0.1) ? 1 : -1;
				if (entry * correctClass > 0)
				{
					countcorrect++; // This is for the recognition of a single digit e.g. "9" vs all the others.
					ExtendTR.SetAt(i, nCols + 1, 0, 0); // the second additional column on the right points out mistakes
				}
				else
				{
					counterrors++;
					ExtendTR.SetAt(i, nCols + 1, 9999.9F, 0); // the second additional column on the right points out mistakes
				}
			}
		}
		if (!ExtendTR.Write("ExtendedTR.txt"))
		{
			std::cout << "ExtendedTR.txt was locked, look at ExtendedTR2.txt instead." << std::endl;
			ExtendTR.Write("ExtendedTR2.txt");
		}
		//std::cout << "Writing trained aln file... please wait" << std::endl;
		//pALN->Write("NANOoutput.aln");
		if (bClassify2)
		{
			std::cout << "The number of correct classifications was " << countcorrect << " or " << 100.0 * countcorrect / file.RowCount() << " percent " << std::endl;
			std::cout << "The number of errors was " << counterrors << endl;
		}
		std::cout << "Continue training?? Enter number of additional iterations (multiple of 10), 0 to quit, 1 to modify." << 
			"\nModifications: iterations, MSEorF, Weight-bound, WeightDecay" << std::endl;
		std::cin >> iterations;
		if (iterations == 1)
		{
			std::cin >> iterations >> fltMSEorF >> WeightBound >> WeightDecay;
			pdata->fltMSEorF = fltMSEorF;
			for (int m = 0; m < nDim - 1; m++)
			{
				pALN->SetWeightMin(-WeightBound, m, 0);
				pALN->SetWeightMax(WeightBound, m, 0);
			}
		}
	} while (iterations > 0);

	/*
	// Speed test
	std::cout << std::endl << "Starting speed test... please wait" << std::endl;
	CountLeafevals = 0;
	// Record start time
	auto start = std::chrono::high_resolution_clock::now();
	for (long i = 0; i < nTRmaxSamples; i++)
	{
		for (int j = 0; j < nDim; j++)
		{
			afltX[j] = trainfile.GetAt(i, j, 0);
		}
		afltX[nDim - 1] = 0; // This desired output value is thus not input to QuickEval
		ALNoutput = pALN->QuickEval(afltX, ppActiveLFN);
	}
	// Record end time
	auto finish = std::chrono::high_resolution_clock::now();
	std::chrono::duration<float> elapsed = finish - start;
	// Now do the same loop without doing the QuickEval
	// Record start time
	start = std::chrono::high_resolution_clock::now();
	for (long i = 0; i < nTRmaxSamples; i++)
	{
		for (int j = 0; j < nDim; j++)
		{
			afltX[j] = trainfile.GetAt(i, j, 0);
		}
		afltX[nDim - 1] = 0; // This desired output value is thus not input to QuickEval
		// ALNoutput = pALN->QuickEval(afltX, ppActiveLFN);
	}
	// Record end time
	finish = std::chrono::high_resolution_clock::now();
	std::chrono::duration<float> elapsed2 = finish - start;
	std::cout << std::endl << "Elapsed time for " << nTRmaxSamples <<  " evaluations: " << elapsed.count() - elapsed2.count() << " seconds" << std::endl;
	std::cout << "Leaf evaluations during testing " << CountLeafevals << std::endl;
	*/

	//*********************************************************************************
	// For MNIST
	// Now go on to the recognition phase for testing
	std::cout << "Starting evaluation on the test file ... please wait." << std::endl;
	CDataFile testfile;
	if (testfile.Read("MNIST_NANO_TestFile.txt"))
	{
		std::cout << "Reading file succeeded!" << std::endl;
	}
	else
	{
		std::cout << "Reading file failed!" << std::endl;
	}
	int nCorrect = 0;
	int nWrong = 0;
	std::cout << std::endl << "Starting evaluation" << std::endl;
	BOOL bAlphaBeta = FALSE; // Optimizations are not done in the interest of accuracy.
	BOOL bDistanceOptimization = FALSE;
	float DesiredOutput, ALNoutput;
	for (long i = 0; i < 10000; i++)
	{
		for (int j = 0; j < nDim; j++) // Get the domain values
		{
			afltX[j] = testfile.GetAt(i, j, 0);
		}
		DesiredOutput = (fabs(afltX[nDim - 1] - targetDigit) < 0.1 ? 10 : -10); // This is the correct class in the test file MYTEST??????
		afltX[nDim - 1] = -250.0; // Put in an incorrect value here
		ALNoutput = pALN->QuickEval(afltX, ppActiveLFN);
		if (ALNoutput * DesiredOutput > 0) 
		{
			nCorrect++;
		}
		else
		{
			nWrong++;
		}
	}
	std::cout << "Correct: " << nCorrect << " Wrong: " << nWrong << endl;

	free(afltX);
	free(pdata->afltTRdata);
	pALN->Destroy();
	//aln.Destroy(); which one to use?
}


void DecayWeights(const ALNNODE* pNode, const ALN* pALN,float WeightBound, float WeightDecay)
{
	if (NODE_ISMINMAX(pNode))
	{
		DecayWeights(MINMAX_RIGHT(pNode), pALN, WeightBound, WeightDecay);
		DecayWeights(MINMAX_LEFT (pNode), pALN, WeightBound, WeightDecay);
	}
	else
	{
		ASSERT(NODE_ISLFN(pNode));
		if (NODE_ISCONSTANT(pNode))return;
		float* pW = LFN_W(pNode);
		int nDim = pALN->nDim;
		for (int i = 1; i < nDim - 1; i++)
		{
			pW[i] *= WeightDecay;
			pW[i] = max(min(WeightBound, pW[i]), -WeightBound);
		}
	
	}
}
