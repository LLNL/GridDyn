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
#pragma once

#include "../common/def.h"
#include <fstream>
#include <iomanip>
#include <iostream>

namespace griddyn {
namespace paradae {
    class Vector;

    class VirtualMatrix {
      private:
        void operator=(const VirtualMatrix& /*mat*/){};

      protected:
        int m;
        bool isfacto;
        bool isrankdef;

      public:
        VirtualMatrix(): m(0), isfacto(false), isrankdef(false){};
        VirtualMatrix(int m_): m(m_), isfacto(false), isrankdef(false){};
        VirtualMatrix(const VirtualMatrix& mat):
            m(mat.m), isfacto(mat.isfacto), isrankdef(mat.isrankdef){};
        virtual ~VirtualMatrix(){};

        inline int& GetM() { return m; };
        inline int GetM() const { return m; };
        inline bool& IsFacto() { return isfacto; };
        inline bool IsFacto() const { return isfacto; };
        inline bool& IsRankDeficient() { return isrankdef; };
        inline bool IsRankDeficient() const { return isrankdef; };

        virtual void Clone(const VirtualMatrix& mat) = 0;
        virtual Real& operator()(int i, int j) = 0;
        virtual Real operator()(int i, int j) const = 0;
        virtual void operator*=(Real alpha) = 0;
        virtual void Fill(Real fill_ = 0) = 0;
        virtual void SetSubMat(int i, int j, const VirtualMatrix& mat, Real multcoeff = 1.0) = 0;

        virtual void AXPBY(Real alpha, Real beta, const VirtualMatrix& mat) = 0;
        virtual void MatMult(Vector& vec, bool transpose = false) const = 0;
        virtual void Factorize() = 0;
        virtual void Solve(Vector& vec, bool transpose = false) const = 0;
        virtual void ClearFacto() = 0;

        void dump() const { this->dump(std::cout); };
        void dump(std::string filename) const;
        virtual void dump(std::ostream& output) const = 0;
    };

    std::ostream& operator<<(std::ostream& output, const VirtualMatrix& mat);

    class Matrix: public VirtualMatrix {
      private:
        void operator=(const Matrix& mat){};

      public:
        Matrix(): VirtualMatrix(){};
        Matrix(int m_): VirtualMatrix(m_){};
        Matrix(const Matrix& mat): VirtualMatrix(mat){};
        virtual ~Matrix(){};
        virtual void SetIJV(int n_, int nnz_, int* ival, int* jval, Real* vval) = 0;
    };

}  // namespace paradae
}  // namespace griddyn
