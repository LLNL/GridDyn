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
#ifndef SBlockTriMatrix_h
#define SBlockTriMatrix_h

#include "DBlockTriMatrix.h"

namespace griddyn {
namespace paradae {
    class SBlockTriMatrix: public DBlockTriMatrix {
      private:
        void operator=(const SBlockTriMatrix& mat){};

      public:
        SBlockTriMatrix(): DBlockTriMatrix(){};
        SBlockTriMatrix(int m_, int s_, bool isdense_ = true);
        SBlockTriMatrix(const SBlockTriMatrix& mat);
        virtual ~SBlockTriMatrix();

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

        virtual void dump(std::ostream& output) const;
    };
}  // namespace paradae
}  // namespace griddyn

#endif
