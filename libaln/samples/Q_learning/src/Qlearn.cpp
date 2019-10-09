//File: Qlearn.cpp
// ALN Sample - Reinforcement learning test using the NANO library
// Training routine for pendulum swing-up, using Q-learning
// ALN Library sample
// Licence: LGPL
// 
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
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

#include <stdlib.h>
#include <alnpp.h>
#include <math.h>
#include <fstream>
#include <chrono>
#include <ctime>
#include <iostream>
#include "cmyaln.h"

constexpr auto MSEORF = 0.001;
constexpr auto ULIMIT = 0.02;
constexpr auto BUFFERSIZE = 1000;
constexpr auto NMAXEPOCHS = 2;
constexpr auto DBLLEARNRATE = 0.8;
constexpr auto DELTAT = 0.05;
double dblAlpha = 0.02; // This and the following are used in computing the reinforcement as in Sutton - Barto
double dblGamma = 0.5; // dblAlpha is the fraction of updating of Q and dblGamma is the amount of maximal future Q mixed in.

extern BOOL bStopTraining;
static char szInfo[] = "NANO (Noise-Attenuating Neuron Online) program used for Q_learning\n"
"Copyright (C)  2019 William W. Armstrong\n"
"Licensed under LGPL\n\n";
double dblTrainErr;
extern int nMaxEpochs;
int nNumberLFNs;
double deltat= DELTAT;
double ulimit= ULIMIT;
double dblEps3 = 0.001;
static const double PI = 3.14159265359;
static const double PIt2 = PI * 2.0;
std::ofstream pro("Protocol.txt", std::ios_base::trunc);
class CMyAln;
void dynamics(double *, double *);
void genvector(CMyAln*);

int Qlearn()
{
	int nMaxEpochs = NMAXEPOCHS; // this is the number of epochs before splitting occurs
	int nTRmaxSamples = BUFFERSIZE; // one pass through all these samples is one epoch
	auto start = std::chrono::system_clock::now();
	std::time_t start_time = std::chrono::system_clock::to_time_t(start);
	if (pro.good())
	{
		pro << "This protocol file opened\n"
		    << szInfo
		    << "Started program at " << std::ctime(&start_time)
		    << "\nTime step delta t = " << deltat << "\nThree values of torque for control: 0 or + or - " << ulimit
			<< "\nInitial number of training iterations = " << 1000 << "\n"
			<< "Each iteration does " << nMaxEpochs << " epochs of training \n"
			<< "Each epoch has " << nTRmaxSamples << " samples" << std::endl;
	}
	else
	{
		std::cerr << "Protocol file could not be opened." << std::endl;
	}
	// seed ALN random number generator
	CAln::SRand(231);
	int nDim = 4;
	int nTRcols = 2 * nDim + 1; // The nDim rows of sample data in adblTRdata
	 //are extended to 2 * nDim + 1 total rows for the noise-attenuation tool.
	// create ALN with four variables and make growable
	pro << "Creating ALN, four variables, output variable 3 (zero based)... ";
	CMyAln aln;
	if (!aln.Create(nDim, nDim - 1))
	{
		std::cerr << "ALN creation failed!" << std::endl;
		return 1;
	}
	else
	{
		pro << "succeeded!" << std::endl;
	}
	CMyAln* pALN = &aln;
	ALNNODE* pTree = pALN->GetTree();
	pALN->SetGrowable(pTree);
	ALNREGION* pRegion = pALN->GetRegion(0);
	pRegion->dblSmoothEpsilon = 0;
	//Controlling splitting of linear pieces and stopping training when splitting stops
	double dblMSEorF = MSEORF; // a negative value indicates use of an F-test to stop splitting
	// The F-test is specified in split_ops.cpp. A positive value is used as a threshold on the MSE of a piece (N.B. not the square root, i.e. RMSE).
	if (dblMSEorF > 0)
	{
		pro << "The mean square error below which splitting stops is " << dblMSEorF << std::endl;
	}
	else
	{
		pro << "An F-test based on estimated noise variance is used for stopping splitting." << std::endl;
	}
	// This sets up the training buffer of doubles adblTRbuffer. It is part of every ALN and contains both the training
	// data as well as a tool for determining noise variance. The noise variance values are determined in split_ops.cpp
	ALNDATAINFO* pdata = pALN->GetDataInfo();
	pdata->nTRmaxSamples = nTRmaxSamples;
	pdata->nTRcurrSamples = 0;
	pdata->nTRcols = nTRcols;
	pdata->nTRinsert = 0;
	pdata->dblMSEorF = dblMSEorF;
	//  void SetDataInfo(double* adblTRdata, const int nTRmaxSamples, int nTRcurrSamples, int nTRcols, int nTRinsert, const double dblMSEorF);
	// SetDataInfo(adblTRdata, nTRmaxSamples, nTRcurrSamples, nTRcols, nTRinsert, dblMSEorF); NOT NEEDED
	/* We can set appropriate constraints later
	// Set constraints
	aln.SetEpsilon(0.001, 0);
	aln.SetMin(-10, 0);
	aln.SetMax(10, 0);
	aln.SetWeightMin(-1000, 0);
	aln.SetWeightMax(1000, 0);
	aln.SetEpsilon(0.001, 1);
	aln.SetMin(-10, 1);
	aln.SetMax(10, 1);
	aln.SetWeightMin(-1000, 1);
	aln.SetWeightMax(1000, 1);
	aln.SetEpsilon(0.001, 2);
	aln.SetMin(-0.2, 2);
	aln.SetMax(0.2, 2);
	aln.SetWeightMin(-1000, 2);
	aln.SetWeightMax(1000, 2);
	aln.SetEpsilon(0.05, 3);
	aln.SetMin(-1000, 3);
	aln.SetMax(1000, 3);
	*/

	// train ALN
	double dblLearnRate = DBLLEARNRATE;  // slow learning rate
	double dblMinRMSE = 0.000001; //small so as not to be the reason for stopping training
	BOOL bJitter = FALSE;
	//load the buffer initially; as the buffer gets each new sample, it is compared to existing samples to determine noise variance
	double* adblX = (double*)malloc(nDim * sizeof(double));
	for (int i = 0; i < nTRmaxSamples; i++)
	{
		genvector( pALN);
		// this calls pALN->addTRsample(pALN, adblX, nDim) to generate a training sample in the buffer adblTRdata;
	}
	pro << "Data about the ALN's data buffer after loading: "
	    << "\nnTRmaxSamples = " << pdata->nTRmaxSamples << "  "
		<< "\nnTRcurrSamples = " << pdata->nTRcurrSamples << "  "
		<< "\nnTRcols  = " << pdata->nTRcols << "  "
		<< "\nnTRinsert = " << pdata->nTRinsert << " " 
		<< "MSE or F " << pdata->dblMSEorF << std::endl;
	int nDimt2 = nDim * 2;
	int nDimt2p1 = nDimt2 + 1;
	// Print part of the buffer contents to check
	int nBegin = 0;
	int nPrint = 2;
	for (int ii = nBegin; ii < nBegin + nPrint - 1; ii++) // Just a sampling of the file
	{
		for (int jj = 0; jj < nDimt2p1; jj++)
		{
			if (jj == nDim) pro << "closest is ";
			if (jj == nDimt2) pro << " square dist = ";
			pro << pdata->adblTRdata[nDimt2p1 * ii + jj] << " ";
		}
		pro << std::endl;
	}
	// Compute the average of all noise variance samples in the buffer (neglecting slope)
	double averageNVsample = 0;
	for (int ii = 0; ii < pdata->nTRcurrSamples; ii++) // Just a sampling of the file
	{
		averageNVsample += 0.5 * pow(pdata->adblTRdata[nDimt2p1 * ii + nDimt2 - 1], 2);
	}
	averageNVsample /= pdata->nTRcurrSamples;
	pro << "Estimate of average noise variance (neglecting slope) = " << averageNVsample << std::endl;
	int nNotifyMask = AN_TRAINEND ;
	long iterations = 5;
	do
	{
		pro << "Sequence of " << iterations << " iterations begins." << std::endl;
		pro <<  "MinRMSE = " << dblMinRMSE << " Learning rate = " << dblLearnRate
			<< " Jitter =  " << bJitter << std::endl;
		for (long iteration = 0; iteration < iterations; iteration++)
		{
			std::cerr << "Iteration " << iteration << " ";

			// Compute the average of all noise variance samples in the buffer. This neglects slope
			// however on average, the slope should not contribute much unless the overall function
			// has a large slope.  In that case, we should preprocess the data to get rid of it.
			averageNVsample = 0;
			for (int ii = 0; ii < pdata->nTRcurrSamples; ii++) // Just a sampling of the file
			{
				averageNVsample += 0.5 * pow(pdata->adblTRdata[nDimt2p1 * ii + nDimt2 - 1], 2);
			}
			averageNVsample /= pdata->nTRcurrSamples;
			std::cout << "Average noise variance " << averageNVsample;
			dblMSEorF = averageNVsample * 0.5;

			pro << "Iteration " << iteration+1 << " of " << iterations << " begins." << std::endl;
			if (bStopTraining)
			{
				pro << "At the end of this iteration, no more splitting occurred" << std::endl;
				//break;
			}
			// TRAINING VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
			if (!aln.Train(nMaxEpochs, dblMinRMSE, dblLearnRate, bJitter, nNotifyMask))
			{
				pro << "Training failed!" << std::endl;
				return -1;
			}


			// Phase 2 is switching to a "flywheel" for updating Q, i.e. dblAlpha > 0
			// inject more data into the buffer in phase 3, now with an updated Q function.
			for (int i = 0; i < nTRmaxSamples; i++) // replace the samples at each iteration
			{
				genvector( pALN);
				// this calls pALN->addTRsample(pALN, adblX, nDim) to generate a training sample in the buffer
			}

			}
		//if (bStopTraining) break;
		std::cout << "Would you like to continue training? -- Give number of iterations (0,1,2,...) " << std::endl;
		std::cin >> iterations;
	} while (iterations > 0);
	// save the trained ALN
	if (!aln.Write("Qlearn.aln"))
	{
		aln.Destroy();
		pro << "failed to write ALN!" << std::endl;
		return -1;
	}
	auto end = std::chrono::system_clock::now();
	std::chrono::duration<double> elapsed_seconds = end - start;
	std::time_t end_time = std::chrono::system_clock::to_time_t(end);
	pro << "Finished training at " << std::ctime(&end_time)	<< "elapsed time: " << elapsed_seconds.count() << "s\n";
	return 0;
}

void dynamics(double * adblX, double * adblY)

// This describes a pendulum, initially at an angle 0 in radians to the right as usual
// The goal is at 1.5708 radians (i.e. straight up), complete rotations are penalized
#define gravity 9.80  // acceleration of gravity 9.80 metres/sec^2
#define length 0.17		// length in meters
#define mass 0.1      // mass in kilograms
#define c1    6.92    // deltat /(mass * length^2)
#define c2    1.153		// deltat * gravity / length
{
   adblY[1] = adblX[1] + c1 * adblX[2]- c2 * cos(adblX[0]); // angular velocity -- a small amount of damping can be included 
   adblY[0] = adblX[0] + 0.5 *(adblX[1] + adblY[1])* deltat;// angle
}

// Exploration: creating samples for Q learning, each sample being:
// angle, angular velocity, torque acting on the pendulum u, the Q function value.
void genvector(CMyAln* pALN)
{
	double adblX[4], adblY[4], temp, dblMaxQatY;
	// adblX is the current state (angle, angular velocity), action (torque) and Q function value
	// adbX[0] = angle (radians from the usual 0)
	// adbX[1] = angular velocity (radians/sec)
	// adbX[2] = torque applied (meters * kgm meters/sec^2)
	// adbX[3] = Q (unitless)
	// adblY is the next state, possible action and Q
	for (int i = 0; i < 4; i++)
	{
		adblX[i] = adblY[i] = 0;
	}
	ALNNODE* pActiveLFN;
	// pick a random starting point (radians, radians/sec)
	// the following says don't overshoot by more than PI/2
	adblX[0] = -4.0 * PI + 8.0 * PI * (double)rand() / (RAND_MAX + 1.0); // starting from 0 at the right, up to complete rotation in either direction.
	adblX[1] = -20.0 + 40.0 * (double)rand() / (RAND_MAX + 1.0); // what is a good range of angular velocities?
	// pick a random control (three possibilities)
	adblX[2] = ulimit * (-1.0 + floor(3.0 * ((double)rand() / (RAND_MAX + 1.0))));

	// call dynamics
	dynamics(adblX, adblY);
	// (Y[0] , Y[1]) is the state at the next time step, which is assigned a reward (or penalty).
	// Define what the reward is in terms of the angular difference from straight up and the angular velocity both to a power
	double A = (fabs(adblY[0] - 1.5708) < 0.1 ? 10:0); // if it is more than one quadrant away from straight up at 1.57-8 radians this is negative
	double B = (fabs(adblY[0] - 4.7124) < 0.1 ? 10:0); // same for -4.7124
	double V = (fabs(adblY[1]) < 0.01 ? 10:0); // if it moves faster than one quadrant every second, it gets negative reinforcement.
	// We bound the reinforcements from below by 0
	double dblAngleR =   A + B;// this is maximum at adblY[0] = 1.5708 radians, i.e. straight up, or at
	//  -4.7124 radians, also straight up, representing the shortest angular distances to a goal from the start in (-PI,0).
	double dblVelocityR = V; //(V > 0 ? V:0); // this is maximum at 0 rotational speed and 0 at 1.5708 radians (90 degrees) per second
										 
	// We find the max Q over three possible actions at the *successor* state Y.
	// dblMaxQatY tracks the greatest of the values of q(Y[0],Y[1],u) for the three possible control values of adblY[2]

	// negative torque
	adblY[2] = -ulimit;
	// compute the maximum Q at Y
	dblMaxQatY = pALN->QuickEval(adblY, &pActiveLFN);
	
	// zero torque
	adblY[2] = 0.0;
	// compute the maximum Q at Y
	temp = pALN->QuickEval(adblY, &pActiveLFN);
	if (temp > dblMaxQatY)
	{
		dblMaxQatY = temp;
	}

	// positive torque
	adblY[2] = ulimit;
	// compute the maximum Q at Y
	temp = pALN->QuickEval(adblY, &pActiveLFN);
	if (temp > dblMaxQatY)
	{
		dblMaxQatY = temp;
	}
	//if (dblMaxQatY < 0) dblMaxQatY = 0; // Q must be non-negative by the math.  This is necessary for stability.

	// Compute the current value of Q at the start of the time step
	double dblQ = pALN->QuickEval(adblX, &pActiveLFN); // This is the value of Q at adblX
	//if (dblQ < 0) dblQ = 0;

	// Now we compute the desired value of the Q function for the state action pair adblX
	adblX[3] = dblQ + dblAlpha * ( dblAngleR * dblVelocityR + dblGamma * dblMaxQatY - dblQ); // reward for being near the goal angle with slow rotation
	//if (adblX[3] < 0) adblX[3] = 0;// Q must be non-negative by the math.  This help was necessary in some cases for stability.

	// Now add this sample to the training buffer
	pALN->addTRsample(adblX, 4);
}
