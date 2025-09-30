#include "test_harness.hpp"

#include <probot/control/estimation/pose_estimator.hpp>

TEST_CASE(pose_estimator_prediction_and_vision){
  probot::control::estimation::PoseEstimator estimator(0.5f);
  estimator.reset(probot::control::Pose2d(), 0.0f);

  for (int i=1;i<=10;i++){
    estimator.predict(i * 0.1f, probot::control::ChassisSpeeds(1.0f, 0.0f, 0.0f));
  }
  EXPECT_NEAR(estimator.pose().x, 1.0f, 0.05f);

  estimator.addVisionMeasurement(probot::control::Pose2d(2.0f, 0.5f, 0.1f), 1.2f, 1.0f);
  estimator.predict(1.2f, probot::control::ChassisSpeeds());
  EXPECT_TRUE(estimator.pose().x > 1.0f);
  EXPECT_TRUE(estimator.pose().y > 0.0f);
}
