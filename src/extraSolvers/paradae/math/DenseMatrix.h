/*
 * LLNS Copyright Start
 * Copyright (c) 2018, Lawrence Livermore National Security
 * This work was performed under the auspices of the U.S. Department
 * of Energy by Lawrence Livermore National Laboratory in part under
 * Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
 * Produced at the Lawrence Livermore National Laboratory.
 * All rights reserved.
 * For details, see the LICENSE file.
 * LLNS Copyright End
 */
#ifndef DenseMatrix_h
#define DenseMatrix_h

#include "VirtualMatrix.h"

#define HAVE_LAPACK_CONFIG_H
#define LAPACK_COMPLEX_CPP

namespace griddyn {
namespace paradae {
    typedef int lapack_int;

    class DenseMatrix: public Matrix {
      protected:
        Real* data;
        Real* f_data;
        lapack_int* f_ipiv;

      private:
        void operator=(const DenseMatrix& mat){};

      public:
        DenseMatrix(): Matrix(), data(NULL), f_data(NULL), f_ipiv(NULL){};
        DenseMatrix(int m_, Real fill_ = 0);
        DenseMatrix(const DenseMatrix& mat);
        virtual ~DenseMatrix();

        virtual void Clone(const VirtualMatrix& mat);
        virtual Real& operator()(int i, int j);
        virtual Real operator()(int i, int j) const;
        virtual void operator*=(Real alpha);
        virtual void Fill(Real fill_ = 0);
        virtual void SetSubMat(int i, int j, const VirtualMatrix& mat, Real multcoeff = 1.0);

        virtual void AXPBY(Real alpha, Real beta, const VirtualMatrix& mat);
        virtual void MatMult(Vector& vec, bool transpose = false) const;
        virtual void Factorize();
        virtual void Solve(Vector& vec, bool transpose = false) const;
        virtual void ClearFacto();
        virtual void SetIJV(int m_, int nnz_, int* ival, int* jval, Real* vval);

        virtual void dump(std::ostream& output) const;
    };
}  // namespace paradae
}  // namespace griddyn

#endif
