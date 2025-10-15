#pragma once
#include <algorithm>
#include <probot/control/state_space/matrix.hpp>

namespace probot::control::state_space {
inline Matrix solveDiscreteRiccati(const Matrix& A,
                                   const Matrix& B,
                                   const Matrix& Q,
                                   const Matrix& R,
                                   std::size_t maxIterations = 100,
                                   float tolerance = 1e-4f){
    if (A.rows != A.cols) throw std::runtime_error("A must be square");
    if (B.rows != A.rows) throw std::runtime_error("B dimension mismatch");
    Matrix P = Q;
    for (std::size_t iter=0; iter<maxIterations; ++iter){
      Matrix BT = transpose(B);
      Matrix AT = transpose(A);
      Matrix term = add(R, multiply(multiply(BT, P), B));
      Matrix termInv = inverse(term);
      Matrix AP = multiply(AT, multiply(P, A));
      Matrix middle = multiply(multiply(AT, P), B);
      Matrix correction = multiply(multiply(middle, termInv), transpose(middle));
      Matrix Pnext = subtract(add(AP, Q), correction);
      float diff = 0.0f;
      for (std::size_t i=0;i<P.data.size();i++) diff = std::max(diff, std::fabs(Pnext.data[i] - P.data[i]));
      P = Pnext;
      if (diff < tolerance) break;
    }
    return P;
  }

  inline Matrix computeLQR(const Matrix& A,
                           const Matrix& B,
                           const Matrix& Q,
                           const Matrix& R,
                           std::size_t maxIterations = 100,
                           float tolerance = 1e-4f){
  Matrix P = solveDiscreteRiccati(A, B, Q, R, maxIterations, tolerance);
  Matrix BT = transpose(B);
  Matrix term = add(R, multiply(multiply(BT, P), B));
  Matrix termInv = inverse(term);
  return multiply(termInv, multiply(BT, multiply(P, A)));
  }
}
