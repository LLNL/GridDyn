/*
 * LLNS Copyright Start
 * Copyright (c) 2017, Lawrence Livermore National Security
 * This work was performed under the auspices of the U.S. Department
 * of Energy by Lawrence Livermore National Laboratory in part under
 * Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
 * Produced at the Lawrence Livermore National Laboratory.
 * All rights reserved.
 * For details, see the LICENSE file.
 * LLNS Copyright End
 */

#include "vectorOps.hpp"

double solve2x2 (double v11, double v12, double v21, double v22, double y1, double y2, double &x1, double &x2)
{
    double det = 1.0 / (v11 * v22 - v12 * v21);
    x1 = det * (v22 * y1 - v12 * y2);
    x2 = det * (-v21 * y1 + v11 * y2);
    return det;
}

std::array<double, 3>
solve3x3 (const std::array<std::array<double, 3>, 3> &input, const std::array<double, 3> &vals)
{
    double a11 = input[0][0];
    double a12 = input[0][1];
    double a13 = input[0][2];
    double a21 = input[1][0];
    double a22 = input[1][1];
    double a23 = input[1][2];
    double a31 = input[2][0];
    double a32 = input[2][1];
    double a33 = input[2][2];

    double i11 = a33 * a22 - a32 * a23;
    double i12 = -(a33 * a12 - a32 * a13);
    double i13 = a23 * a12 - a22 * a13;

    double i21 = -(a33 * a21 - a31 * a23);
    double i22 = a33 * a11 - a31 * a13;
    double i23 = -(a23 * a11 - a21 * a13);

    double i31 = a32 * a21 - a31 * a22;
    double i32 = -(a32 * a11 - a31 * a12);
    double i33 = a22 * a11 - a21 * a12;

    double deti = 1.0 / (a11 * i11 + a21 * i12 + a31 * i13);

	std::array<double, 3> Y{{deti * (i11 * vals[0] + i12 * vals[1] + i13 * vals[2]),
							deti * (i21 * vals[0] + i22 * vals[1] + i23 * vals[2]),
							deti * (i31 * vals[0] + i32 * vals[1] + i33 * vals[2])} };

    return Y;
}

// Linear Interpolation function
std::vector<double> interpolateLinear (const std::vector<double> &timeIn,
                                       const std::vector<double> &valIn,
                                       const std::vector<double> &timeOut)
{
    size_t mx = (std::min) (valIn.size (), timeIn.size ());
    std::vector<double> out (timeOut.size (), 0);
    size_t jj = 0;
    size_t kk = 0;
    while (timeOut[jj] <= timeIn[0])
    {
        out[jj] = valIn[0] - (valIn[1] - valIn[0]) / (timeIn[1] - timeIn[0]) * (timeIn[0] - timeOut[jj]);
        ++jj;
    }
    while (jj < timeOut.size ())
    {
        while (timeIn[kk + 1] < timeOut[jj])
        {
            ++kk;
            if (kk + 1 == mx)
            {
                goto breakLoop;  // break out of a double loop
            }
        }
        out[jj] =
          valIn[kk] + (valIn[kk + 1] - valIn[kk]) / (timeIn[kk + 1] - timeIn[kk]) * (timeOut[jj] - timeIn[kk]);
        // out[jj] = std::fma((valIn[kk + 1] - valIn[kk]) / (timeIn[kk + 1] - timeIn[kk]), (timeOut[jj] -
        // timeIn[kk]), valIn[kk]);
        ++jj;
    }
breakLoop:
    while (jj < timeOut.size ())
    {
        out[jj] =
          valIn[mx - 1] +
          (valIn[mx - 1] - valIn[mx - 2]) / (timeIn[mx - 1] - timeIn[mx - 2]) * (timeOut[jj] - timeIn[mx - 1]);
        ++jj;
    }
    return out;
}