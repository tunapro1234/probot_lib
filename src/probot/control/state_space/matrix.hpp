#pragma once
#include <vector>
#include <cstddef>
#include <cmath>
#include <stdexcept>

namespace probot::control::state_space {
  struct Matrix {
    std::size_t rows;
    std::size_t cols;
    std::vector<float> data;

    Matrix() : rows(0), cols(0) {}
    Matrix(std::size_t r, std::size_t c, float initial = 0.0f)
    : rows(r), cols(c), data(r * c, initial) {}

    float& operator()(std::size_t r, std::size_t c){ return data[r * cols + c]; }
    float  operator()(std::size_t r, std::size_t c) const { return data[r * cols + c]; }
  };

  inline Matrix identity(std::size_t n){
    Matrix I(n, n, 0.0f);
    for (std::size_t i=0;i<n;i++) I(i,i) = 1.0f;
    return I;
  }

  inline Matrix transpose(const Matrix& A){
    Matrix T(A.cols, A.rows);
    for (std::size_t r=0;r<A.rows;r++)
      for (std::size_t c=0;c<A.cols;c++)
        T(c,r) = A(r,c);
    return T;
  }

  inline Matrix add(const Matrix& A, const Matrix& B){
    if (A.rows != B.rows || A.cols != B.cols) throw std::runtime_error("Matrix dimension mismatch");
    Matrix C(A.rows, A.cols);
    for (std::size_t i=0;i<A.data.size();i++) C.data[i] = A.data[i] + B.data[i];
    return C;
  }

  inline Matrix subtract(const Matrix& A, const Matrix& B){
    if (A.rows != B.rows || A.cols != B.cols) throw std::runtime_error("Matrix dimension mismatch");
    Matrix C(A.rows, A.cols);
    for (std::size_t i=0;i<A.data.size();i++) C.data[i] = A.data[i] - B.data[i];
    return C;
  }

  inline Matrix scale(const Matrix& A, float factor){
    Matrix C(A.rows, A.cols);
    for (std::size_t i=0;i<A.data.size();i++) C.data[i] = A.data[i] * factor;
    return C;
  }

  inline Matrix multiply(const Matrix& A, const Matrix& B){
    if (A.cols != B.rows) throw std::runtime_error("Matrix dimension mismatch");
    Matrix C(A.rows, B.cols, 0.0f);
    for (std::size_t i=0;i<A.rows;i++){
      for (std::size_t k=0;k<A.cols;k++){
        float a = A(i,k);
        for (std::size_t j=0;j<B.cols;j++){
          C(i,j) += a * B(k,j);
        }
      }
    }
    return C;
  }

  inline Matrix inverse(const Matrix& M){
    if (M.rows != M.cols) throw std::runtime_error("Matrix inverse requires square matrix");
    std::size_t n = M.rows;
    Matrix A = M;
    Matrix I = identity(n);

    for (std::size_t i=0;i<n;i++){
      // Pivot selection
      float pivot = A(i,i);
      std::size_t pivotRow = i;
      for (std::size_t r=i+1;r<n;r++){
        if (std::fabs(A(r,i)) > std::fabs(pivot)){
          pivot = A(r,i);
          pivotRow = r;
        }
      }
      if (std::fabs(pivot) < 1e-9f) throw std::runtime_error("Matrix is singular");
      if (pivotRow != i){
        for (std::size_t c=0;c<n;c++) std::swap(A(i,c), A(pivotRow,c));
        for (std::size_t c=0;c<n;c++) std::swap(I(i,c), I(pivotRow,c));
      }
      // Normalize row
      float invPivot = 1.0f / A(i,i);
      for (std::size_t c=0;c<n;c++){ A(i,c) *= invPivot; I(i,c) *= invPivot; }
      // Eliminate others
      for (std::size_t r=0;r<n;r++){
        if (r == i) continue;
        float factor = A(r,i);
        for (std::size_t c=0;c<n;c++){
          A(r,c) -= factor * A(i,c);
          I(r,c) -= factor * I(i,c);
        }
      }
    }
    return I;
  }

  inline Matrix pseudoInv(const Matrix& M){
    if (M.rows >= M.cols){
      Matrix Mt = transpose(M);
      Matrix MtM = multiply(Mt, M);
      Matrix inv = inverse(MtM);
      return multiply(inv, Mt);
    } else {
      Matrix Mt = transpose(M);
      Matrix MMt = multiply(M, Mt);
      Matrix inv = inverse(MMt);
      return multiply(Mt, inv);
    }
  }

  inline Matrix zeros(std::size_t rows, std::size_t cols){ return Matrix(rows, cols, 0.0f); }
}
