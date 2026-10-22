/*
 * Copyright (c) 2014-2026, Lawrence Livermore National Security
 * See the top-level NOTICE for additional details. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 */

// test case for Value Predictors

#include "utilities/valuePredictor.hpp"
#include <gtest/gtest.h>

TEST(ValuePredictorTests, InitialValueAndSlopeDefineLinearPrediction)
{
    const utilities::valuePredictor<double> predictor{2.0, 10.0, 3.0};

    EXPECT_DOUBLE_EQ(predictor.getKnownInput(), 2.0);
    EXPECT_DOUBLE_EQ(predictor.getKnownOutput(), 10.0);
    EXPECT_DOUBLE_EQ(predictor.getSlope(), 3.0);
    EXPECT_DOUBLE_EQ(predictor.predict(4.0), 16.0);
    EXPECT_DOUBLE_EQ(predictor(0.0), 4.0);
}

TEST(ValuePredictorTests, UpdateRecomputesSlopeAndSetSlopeOverridesIt)
{
    utilities::valuePredictor<double> predictor{1.0, 5.0};

    predictor.update(3.0, 11.0);
    EXPECT_DOUBLE_EQ(predictor.getKnownInput(), 3.0);
    EXPECT_DOUBLE_EQ(predictor.getKnownOutput(), 11.0);
    EXPECT_DOUBLE_EQ(predictor.getSlope(), 3.0);
    EXPECT_DOUBLE_EQ(predictor.predict(5.0), 17.0);

    predictor.setSlope(-2.0);
    EXPECT_DOUBLE_EQ(predictor.getSlope(), -2.0);
    EXPECT_DOUBLE_EQ(predictor.predict(5.0), 7.0);
}
