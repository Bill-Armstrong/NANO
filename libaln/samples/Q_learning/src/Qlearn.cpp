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

constexpr auto MSEORF = 5.0;
constexpr auto ULIMIT = 0.06;
constexpr auto BUFFERSIZE = 1600;
constexpr auto NMAXEPOCHS = 20;
constexpr auto DBLLEARNRATE = 0.1;
constexpr auto DELTAT = 0.1;
double dblAlpha = 0.3; // This and the following are used in computing the reinforcement as in Sutton - Barto
double dblGamma = 0.85; // dblAlpha is the fraction of updating of Q and dblGamma is the amount of maximal future Q mixed in.

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
			<< "\nInitial number of training iterations = " << 5 << "\n"
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
	double dblLearnRate = DBLLEARNRATE;  // slow learning rate
	double dblMinRMSE = 0.000001; //small so as not to be the reason for stopping training
	BOOL bJitter = FALSE;
	//load the buffer initially with zero samples to initialize the ALN to constant zero
	double* adblX = (double*)malloc(nDim * sizeof(double));
	for (int i = 0; i < nTRmaxSamples; i++)
	{
		adblX[0] = -4.0 * PI + 8.0 * PI * (double)rand() / (RAND_MAX + 1.0); // starting from 0 at the right, up to complete rotation in either direction.
		adblX[1] = -20.0 + 40.0 * (double)rand() / (RAND_MAX + 1.0); // range of angular velocities?
		adblX[2] = ulimit * (-1.0 + floor(3.0 * ((double)rand() / (RAND_MAX + 1.0))));		// pick a random control (three possibilities)
		adblX[3] = 0.0;
		pALN->addTRsample(adblX, 4);
	}
	pro << "Data about the ALN's data buffer after loading: "
	    << "\nnTRmaxSamples = " << pdata->nTRmaxSamples << "  "
		<< "\nnTRcurrSamples = " << pdata->nTRcurrSamples << "  "
		<< "\nnTRcols  = " << pdata->nTRcols << "  "
		<< "\nnTRinsert = " << pdata->nTRinsert << " " 
		<< "MSE or F " << pdata->dblMSEorF << std::endl;
	int nDimt2 = nDim * 2;
	int nDimt2p1 = nDimt2 + 1;
	int nNotifyMask = AN_TRAINEND ;
	long initIterations = 5;
	long iterations = initIterations;
	do
	{
		pro << "Sequence of " << iterations << " iterations begins." << std::endl;
		pro <<  "MinRMSE = " << dblMinRMSE << " Learning rate = " << dblLearnRate
			<< " Jitter =  " << bJitter << std::endl;
		for (long iteration = 0; iteration < iterations; iteration++)
		{
			std::cerr << "Iteration " << iteration << " ";

			pro << "Iteration " << iteration+1 << " of " << iterations << " begins." << std::endl;
			// TRAINING VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
			if (!aln.Train(nMaxEpochs, dblMinRMSE, dblLearnRate, bJitter, nNotifyMask))
			{
				pro << "Training failed!" << std::endl;
				return -1;
			}
			if (iteration > initIterations && bStopTraining)
			{
				pro << "At the end of this iteration, no more splitting occurred" << std::endl;
				//break;
			}
			// replace a fraction of the data in the buffer, now with an updated Q function.
			for (int i = 0; i < nTRmaxSamples; i++) // replace samples at each iteration
			{
				genvector( pALN);
				// this calls pALN->addTRsample(pALN, adblX, nDim) to generate a training sample in the buffer
			}
		}
		// save the trained ALN
		if (!aln.Write("Qlearn.aln"))
		{
			aln.Destroy();
			pro << "failed to write ALN!" << std::endl;
			return -1;
		}
		std::cout << "Would you like to continue training? -- Give number of iterations (0,1,2,...) " << std::endl;
		std::cin >> iterations;
	} while (iterations > 0);

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
	// for timestep 0.01 second, the damping applied to adblX[1] is 0.9939; for 0.1 second 0.94 works to damp oscillations completely in about 100sycles.
   adblY[1] = 0.98 * adblX[1] + c1 * adblX[2]- c2 * cos(adblX[0]); // angular velocity -- a small amount of damping is included 
   adblY[0] = adblX[0] + 0.5 *(adblX[1] + adblY[1])* deltat;// angle
}

// Exploration: creating samples for Q learning, each sample being:
// angle, angular velocity, torque acting on the pendulum u, the Q function value.
void genvector(CMyAln* pALN)
{
	double adblX[4], adblY[4], temp, dblMaxQatY;
	// adblX is the current state (angle, angular velocity), action (torque) and Q function value
	// adbX[0] = angle (radians counter-clockwise from the usual 0 to the right)
	// adbX[1] = angular velocity (radians/sec)
	// adbX[2] = torque applied (meters * kgm meters/sec^2)
	// adbX[3] = Q (unitless, or same unit as reinforcement)
	// adblY is the next state, possible action and Q
	for (int i = 0; i < 4; i++)
	{
		adblX[i] = adblY[i] = 0;
	}
	ALNNODE* pActiveLFN;
	// pick a random starting point (radians, radians/sec) -- check output for out of range values
	adblX[0] = -2.0 * PI + 3.0 * PI * (double)rand() / (RAND_MAX + 1.0); // starting from 0 at the right, up to complete rotation in either direction.
	adblX[1] = -10.0 + 20.0 * (double)rand() / (RAND_MAX + 1.0); // what is a good range of angular velocities?
	// pick a random control (three possibilities)
	adblX[2] = ulimit * (-1.0 + floor(3.0 * ((double)rand() / (RAND_MAX + 1.0))));

	// call dynamics
	dynamics(adblX, adblY);
	// (Y[0] , Y[1]) is the state at the next time step; the action of passage to the new state is assigned a reward (or penalty).
	// Define what the reward is in terms of the angular difference from straight up and the angular velocity in the new state

	double A = 200.0 * (fabs(30.0 * adblY[0] - 1.5708) > 0 ? 1 : 0); // reinforce if within 3 degrees of straight up after counter-clockwise turn
	double B = 100.0 * (fabs(30.0 * adblY[0] - 1.5708) > 0 ? 1 : 0); // second target, clockwise turn
	// V, if implemented,works against too high speeds.
	double V = 300.0 * (fabs(adblY[1]) < 9.8 ? 1 : 0); // is the absolute angular speed below 5 radians per second?
	double dblAngleR = A + B;// this is maximum at adblY[0] = 1.5708 radians, i.e. straight up, or also, optionally, at
	     //  -4.7124 radians, also straight up, representing the shortest angular distances to a goal from the start in (-PI,0).
	double dblVelocityR = V;
	// We find the max Q over the three possible actions at the *successor* state Y.
	// dblMaxQatY tracks the greatest of the values of Q(Y[0],Y[1],u) for the three possible control values of adblY[2]

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
	// Compute the current value of Q at the start of the time step
	double dblQ = pALN->QuickEval(adblX, &pActiveLFN); // This is the value of Q at adblX
	// Now we compute the desired value of the Q function for the state action pair adblX
	adblX[3] = dblQ + dblAlpha * ((dblAngleR + dblVelocityR) + dblGamma * dblMaxQatY - dblQ);
	// Now add this sample to the training buffer
	pALN->addTRsample(adblX, 4);
}
