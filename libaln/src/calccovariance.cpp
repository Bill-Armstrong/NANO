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

// Some matrix routines are taken from Eigen and are under the Mozilla Public License MPL2

// calccovariance.cpp

#ifdef ALNDLL
#define ALNIMP __declspec(dllexport)
#endif
#include <aln.h>
#include <alnpriv.h>
#include "Eigen/Dense"


using namespace std;
using namespace Eigen;

#define TOL 1.0e-12f

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
// helpers for filling and dumping matrices 
//void fillRMA(MatrixXd &X, float* afltX);
//void ALNAPI dumpRMA(const MatrixXd &X, float* afltX);
//void ALNAPI fillCMA(MatrixXd &X, float* afltX);
//void ALNAPI dumpCMA(const MatrixXd &X, float* afltX);

void fillRMA(MatrixXd& X, float* afltX)
{
    // The array afltX is organized in row-major order
    Index nRows, nr, nCols, nc;
    nRows = X.rows();
    nCols = X.cols();
    for (nr = 0; nr < nRows; ++nr)
    {
        for (nc = 0; nc < nCols; ++nc)
        {
            X(nr, nc) = afltX[nr * nCols + nc];
        }
    }
}

void ALNAPI dumpRMA(const MatrixXd& X, float* afltX)
{
    // The array afltX is organized in row-major order
    Index nRows, sr, nCols, sc;
    nRows = X.rows();
    nCols = X.cols();
    for (sr = 0; sr < nRows; ++sr)
    {
        for (sc = 0; sc < nCols; ++sc)
        {
            afltX[sr * nCols + sc] = (float)X(sr, sc);
        }
    }
}

void ALNAPI fillCMA(MatrixXd& X, float* afltX)
{
    // The array afltX is organized in column-major order
    Index nRows, nr, nCols, nc;
    nRows = X.rows();
    nCols = X.cols();
    for (nc = 0; nc < nCols; ++nc)
    {
        for (nr = 0; nr < nRows; ++nr)
        {
            X(nr, nc) = afltX[nc * nRows + nr];
        }
    }
}

void ALNAPI dumpCMA(const MatrixXd& X, float* afltX)
{
    // The array afltX is organized in column-major order
    Index nRows, nr, nCols, nc;
    nRows = X.rows();
    nCols = X.cols();
    for (nc = 0; nc < nCols; ++nc)
    {
        for (nr = 0; nr < nRows; ++nr)
        {
            afltX[nc * nRows + nr] = (float)X(nr, nc);
        }
    }
}


BOOL ALNAPI CalcCovariance(int nCols, // number of input vars = number of ALN inputs - 1
    int nRows,        // number of rows, i.e. input vectors for a given LFN
    float* afltX,    // RMA based input vectors (nRows * nCols)
    float* afltY,    // RMA based result vector (nRows)
    float* afltC,    // RMA covariance matrix as array (nCols * nCols), is returned
    float* afltA,    // RMA fitted parameter vector (nCols)
    float* afltS,    // RMA std dev vector (nRows) (a weighting on points) 
    float* afltU,    // RMA U matrix (nRows * nRows)
    float* afltV,    // RMA V matrix (nCols * nCols)
    float* afltW,    // RMA W array (singvals = min(nRows,nCols))containing singular values
    float& fltChiSq) // chi square of fit
{
    // implementation based on SVD in Eigen
    // note that matrices below are stored in row-major order (RMA)
    ASSERT(afltX && afltY && afltC && afltA && afltS && afltU && afltV && afltW);
    BOOL bSuccess = TRUE;
    MatrixXd X(nRows, nCols);
    fillRMA(X, afltX);
    MatrixXd U(nRows, nRows);
    MatrixXd V(nCols, nCols);
    int singvals = std::min(nRows, nCols);
    VectorXd  s(singvals);  // the singular values s
    VectorXd  sinv(nCols);
    // inverses of the singular values s, but inverse of a very small s is 0 
    // We should solve for fitting parameters A then compare it to the ALNresult
    // We get A = V W^(-1)U^T Y.  This works for square X but here
    // we would get UWV^TA = UWV^TVW^(-1)U^T Y =Y except for the zeroed  W entries

    try
    {
        float smax, tmp, thresh;
        // in a try/catch block to free b in case exception occurs
        // now we do the SVD on X
        JacobiSVD<MatrixXd> svd(X, Eigen::ComputeFullU | Eigen::ComputeFullV);
        U = svd.matrixU();
        V = svd.matrixV();
        s = svd.singularValues();
        // put result of the SVD into the arrays in the call
        dumpRMA(U, afltU);
        dumpRMA(V, afltV);
        // find the maximal singular value
        smax = 0.0f;
        for (int j = 0; j < singvals; j++) {
            if (s(j) > smax) smax = (float)s(j);
        }
        // if any (non-negative) singular value is less than a threshold, set it to zero
        thresh = TOL * smax;
        for (int j = 0; j < singvals; j++) {
            if (s(j) < thresh)	s(j) = 0.0;
            afltW[j] = (float)s(j);
        }
        // Create the inverses, extended by 0 where necessary to make nCols
        for (int j = 0; j < nCols; j++) {
            if (j >= singvals || s(j) == 0.0) {
                sinv(j) = 0.0;  // surprising but the right way to do it
            }
            else
            {
                sinv(j) = 1.0 / s(j);
            }
        }

        // compute the fitting parameters for rNows of output values
        // First make a vector out of Y
        VectorXd Y(nRows);
        for (int sr = 0; sr < nRows; sr++)
        {
            Y(sr) = afltY[sr]; // this Y is incorrect for the entries j = 0 to j = 4  Something is overwriting the Y array!!!!
        }
        MatrixXd UT(nRows, nRows);
        UT = U.transpose();  // This is the inverse of U
        VectorXd tempY(nRows);
        tempY = UT * Y; // This should be the output of the diagonal matrix in the middle of the svd
        VectorXd tempV(nCols);
        // Instead of taking the inverse of the diagonal matrix and multiplying by tempY we do the components
        for (int sr = 0; sr < nCols; sr++) {
            if (sr < singvals) {
                tempV(sr) = sinv(sr) * tempY(sr);
            }
            else {
                tempV(sr) = 0;
            }
        }
        VectorXd A(nCols); // A is the solution of the least-squares problem, i.e. the ideal weights on the LFN
        A = V * tempV; // V is the inverse of V^T
        // return the values in A to the pointer structure
        for (int i = 0; i < nCols; i++)
        {
            afltA[i] = (float)A(i);
        }
        // finding the measure of error of the fit by chi-squared 
        float fltChiSq = 0.0;
        for (int i = 0; i < nRows; i++)
        {
            float sum = 0.0;
            for (int j = 0; j < nCols; j++)
            {
                sum += (float)(X(i, j) * A(j));
            }
            tmp = (float)((Y(i) - sum) / afltS[i]); // afltS[i] is a weight on input vector i, usually 1.0
            fltChiSq += tmp * tmp;
        }

        // calc covariance matrix C
        for (int k = 0; k < nCols; k++)
        {
            for (int i = 0; i <= k; i++)		// use symmetry
            {
                float sum = 0.0;
                for (int j = 0; j < nCols; j++)
                {
                    sum += (float)(V(k, j) * V(i, j) * sinv(j) * sinv(j));
                }
                afltC[i * nCols + k] = afltC[k * nCols + i] = sum;
            }
        }
    }

    catch (string SVDdecomposition)
    {
        bSuccess = FALSE; // something failed!
    }
    return bSuccess;
}
