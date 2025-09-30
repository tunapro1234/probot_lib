#include "test_harness.hpp"

#include <cmath>

#include <probot/control/state_space/kalman_filter.hpp>
#include <probot/control/state_space/lqr.hpp>
#include <probot/control/state_space/luenberger_observer.hpp>

TEST_CASE(matrix_basic_ops){
  using probot::control::state_space::Matrix;
  Matrix A(2,2); A(0,0)=1.0f; A(0,1)=2.0f; A(1,0)=3.0f; A(1,1)=4.0f;
  Matrix B(2,2); B(0,0)=5.0f; B(0,1)=6.0f; B(1,0)=7.0f; B(1,1)=8.0f;
  Matrix C = probot::control::state_space::add(A,B);
  EXPECT_NEAR(C(0,0), 6.0f, 1e-5f);
  Matrix D = probot::control::state_space::subtract(C,B);
  EXPECT_NEAR(D(1,1), 4.0f, 1e-5f);

  Matrix E = probot::control::state_space::multiply(A,B);
  EXPECT_NEAR(E(0,0), 19.0f, 1e-5f);
}

TEST_CASE(luenberger_observer_tracks_state){
  using namespace probot::control::state_space;
  Matrix A(1,1); A(0,0)=0.8f;
  Matrix B(1,1); B(0,0)=1.0f;
  Matrix C(1,1); C(0,0)=1.0f;
  Matrix L(1,1); L(0,0)=1.0f;
  Matrix x0(1,1); x0(0,0)=0.0f;
  LuenbergerObserver observer(A,B,C,L,x0);

  Matrix u(1,1); u(0,0)=1.0f;
  float trueState = 0.0f;
  for (int i=0;i<20;i++){
    trueState = 0.8f * trueState + 1.0f;
    Matrix y(1,1); y(0,0)=trueState;
    observer.update(u, y);
  }
  EXPECT_NEAR(observer.state()(0,0), trueState, 1e-1f);
}

TEST_CASE(kalman_filter_converges){
  using namespace probot::control::state_space;
  Matrix A(1,1); A(0,0)=1.0f;
  Matrix B(1,1); B(0,0)=1.0f;
  Matrix C(1,1); C(0,0)=1.0f;
  Matrix Q(1,1); Q(0,0)=0.01f;
  Matrix R(1,1); R(0,0)=0.1f;
  Matrix x0(1,1); x0(0,0)=0.0f;
  Matrix P0(1,1); P0(0,0)=1.0f;
  KalmanFilter kf(A,B,C,Q,R,x0,P0);

  Matrix u(1,1); u(0,0)=1.0f;
  float trueState=0.0f;
  const float noise[5] = {0.05f,-0.02f,0.03f,-0.04f,0.01f};
  for (int i=0;i<5;i++){
    trueState += 1.0f;
    Matrix y(1,1); y(0,0)=trueState + noise[i];
    kf.predict(u);
    kf.correct(y);
  }
  EXPECT_NEAR(kf.state()(0,0), trueState, 0.25f);
}

TEST_CASE(lqr_gain_positive){
  using namespace probot::control::state_space;
  const float dt = 0.1f;
  Matrix A(2,2);
  A(0,0)=1.0f; A(0,1)=dt;
  A(1,0)=0.0f; A(1,1)=1.0f;
  Matrix B(2,1);
  B(0,0)=0.5f*dt*dt;
  B(1,0)=dt;
  Matrix Q = identity(2);
  Matrix R(1,1); R(0,0)=1.0f;
  Matrix K = computeLQR(A,B,Q,R,200,1e-5f);
  EXPECT_TRUE(K.rows == 1 && K.cols == 2);
  EXPECT_TRUE(K(0,0) > 0.0f);
  EXPECT_TRUE(K(0,1) > 0.0f);
}
