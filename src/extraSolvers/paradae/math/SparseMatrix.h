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
#ifndef SparseMatrix_h
#define SparseMatrix_h

#include "../math/VirtualMatrix.h"

namespace griddyn {
namespace paradae {
    class SparseMatrix: public Matrix {
        int nnz;
        int* rowptr;
        int* col;
        Real* data;
        void* numeric;

        Real& InsertVal(int i, int j, Real val);

      private:
        void operator=(const SparseMatrix& mat){};

      public:
        SparseMatrix(): Matrix(), nnz(0), rowptr(NULL), col(NULL), data(NULL), numeric(NULL){};
        SparseMatrix(int m_, int nnz_ = 0);
        SparseMatrix(const SparseMatrix& mat);
        virtual ~SparseMatrix();

        virtual void Clone(const VirtualMatrix& mat);
        virtual Real& operator()(int i, int j);
        virtual Real operator()(int i, int j) const;
        virtual void operator*=(Real alpha);
        virtual void Fill(Real fill_ = 0);
        virtual void FillKeepingStruct(Real fill_ = 0);
        virtual void SetSubMat(int i, int j, const VirtualMatrix& mat, Real multcoeff = 1.0);

        virtual void AXPBY(Real alpha, Real beta, const VirtualMatrix& mat);
        virtual void MatMult(Vector& vec, bool transpose = false) const;
        virtual void Factorize();
        virtual void Solve(Vector& vec, bool transpose = false) const;
        virtual void ClearFacto();
        virtual void SetIJV(int m_, int nnz_, int* ival, int* jval, Real* vval);

        virtual void dump(std::ostream& output) const;
        int GetNNZ() const { return nnz; };
        int GetRowIndex(int idx_nnz) const;
        int GetColIndex(int idx_nnz) const;
        Real GetValue(int idx_nnz) const;
    };
}  // namespace paradae
}  // namespace griddyn

#endif
