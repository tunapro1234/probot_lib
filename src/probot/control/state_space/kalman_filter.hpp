#pragma once
#include <probot/control/state_space/matrix.hpp>

namespace probot::control::state_space {
  class KalmanFilter {
  public:
    KalmanFilter(const Matrix& A,
                 const Matrix& B,
                 const Matrix& C,
                 const Matrix& Q,
                 const Matrix& R,
                 const Matrix& initialState,
                 const Matrix& initialCovariance)
    : A_(A), B_(B), C_(C), Q_(Q), R_(R), xHat_(initialState), P_(initialCovariance) {
      if (A.rows != A.cols) throw std::runtime_error("A must be square");
      if (B.rows != A.rows) throw std::runtime_error("B dimension mismatch");
      if (C.cols != A.cols) throw std::runtime_error("C dimension mismatch");
      if (Q.rows != Q.cols || Q.rows != A.rows) throw std::runtime_error("Q dimension mismatch");
      if (R.rows != R.cols || R.rows != C.rows) throw std::runtime_error("R dimension mismatch");
      if (xHat_.rows != A.rows || xHat_.cols != 1) throw std::runtime_error("Initial state dimension mismatch");
      if (P_.rows != A.rows || P_.cols != A.rows) throw std::runtime_error("Initial covariance dimension mismatch");
    }

    const Matrix& state() const { return xHat_; }
    const Matrix& covariance() const { return P_; }

    void predict(const Matrix& u){
      if (u.rows != B_.cols || u.cols != 1) throw std::runtime_error("Input dimension mismatch");
      xHat_ = add(multiply(A_, xHat_), multiply(B_, u));
      Matrix AT = transpose(A_);
      P_ = add(multiply(multiply(A_, P_), AT), Q_);
    }

    void correct(const Matrix& y){
      if (y.rows != C_.rows || y.cols != 1) throw std::runtime_error("Measurement dimension mismatch");
      Matrix CT = transpose(C_);
      Matrix S = add(multiply(multiply(C_, P_), CT), R_);
      Matrix K = multiply(multiply(P_, CT), inverse(S));
      Matrix innovation = subtract(y, multiply(C_, xHat_));
      xHat_ = add(xHat_, multiply(K, innovation));
      Matrix I = identity(P_.rows);
      P_ = multiply(subtract(I, multiply(K, C_)), P_);
    }

  private:
    Matrix A_;
    Matrix B_;
    Matrix C_;
    Matrix Q_;
    Matrix R_;
    Matrix xHat_;
    Matrix P_;
  };
}
