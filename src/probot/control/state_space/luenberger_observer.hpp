#pragma once
#include <probot/control/state_space/matrix.hpp>

namespace probot::control::state_space {
  class LuenbergerObserver {
  public:
    LuenbergerObserver(const Matrix& A,
                       const Matrix& B,
                       const Matrix& C,
                       const Matrix& L,
                       const Matrix& initialState)
    : A_(A), B_(B), C_(C), L_(L), xHat_(initialState) {
      if (A.rows != A.cols) throw std::runtime_error("A must be square");
      if (B.rows != A.rows) throw std::runtime_error("B dimension mismatch");
      if (C.cols != A.cols) throw std::runtime_error("C dimension mismatch");
      if (L.rows != A.rows || L.cols != C.rows) throw std::runtime_error("L dimension mismatch");
      if (xHat_.rows != A.rows || xHat_.cols != 1) throw std::runtime_error("Initial state dimension mismatch");
    }

    const Matrix& state() const { return xHat_; }

    void update(const Matrix& u, const Matrix& y){
      if (u.rows != B_.cols || u.cols != 1) throw std::runtime_error("Input dimension mismatch");
      if (y.rows != C_.rows || y.cols != 1) throw std::runtime_error("Measurement dimension mismatch");

      Matrix Ax = multiply(A_, xHat_);
      Matrix Bu = multiply(B_, u);
      Matrix yHat = multiply(C_, xHat_);
      Matrix innovation = subtract(y, yHat);
      Matrix correction = multiply(L_, innovation);
      xHat_ = add(add(Ax, Bu), correction);
    }

  private:
    Matrix A_;
    Matrix B_;
    Matrix C_;
    Matrix L_;
    Matrix xHat_;
  };
}
