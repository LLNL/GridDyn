/*
 * Copyright (c) 2014-2026, Lawrence Livermore National Security
 * See the top-level NOTICE for additional details. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 */

// test case for CoreObject object

#include "core/CoreExceptions.h"
#include "fileInput/GridParameter.h"
#include "griddyn/gridDynDefinitions.hpp"
#include <gtest/gtest.h>

TEST(GridModelParameterTests, ConstructorsPreserveNumericAndStringTypes)
{
    const griddyn::GridParameter invalid;
    EXPECT_FALSE(invalid.valid);

    const griddyn::GridParameter numeric{"activePower", 12.5};
    EXPECT_TRUE(numeric.valid);
    EXPECT_FALSE(numeric.stringType);
    EXPECT_EQ(numeric.field, "activePower");
    EXPECT_DOUBLE_EQ(numeric.value, 12.5);

    const griddyn::GridParameter stringValue{"model", "GENROU"};
    EXPECT_TRUE(stringValue.valid);
    EXPECT_TRUE(stringValue.stringType);
    EXPECT_EQ(stringValue.field, "model");
    EXPECT_EQ(stringValue.strVal, "GENROU");
}

TEST(GridModelParameterTests, FromStringParsesNumericValueAndUnits)
{
    const griddyn::GridParameter parameter{"activePower(MW)=12.5"};

    ASSERT_TRUE(parameter.valid);
    EXPECT_FALSE(parameter.stringType);
    EXPECT_EQ(parameter.field, "activePower");
    EXPECT_EQ(parameter.strVal, "12.5");
    EXPECT_DOUBLE_EQ(parameter.value, 12.5);
    EXPECT_EQ(parameter.paramUnits, units::MW);
}

TEST(GridModelParameterTests, FromStringPreservesStringValue)
{
    const griddyn::GridParameter parameter{"model=GENROU"};

    ASSERT_TRUE(parameter.valid);
    EXPECT_TRUE(parameter.stringType);
    EXPECT_EQ(parameter.field, "model");
    EXPECT_EQ(parameter.strVal, "GENROU");
}

TEST(GridModelParameterTests, MissingAssignmentIsRejected)
{
    griddyn::GridParameter parameter;
    EXPECT_THROW(parameter.fromString("activePower"), griddyn::InvalidParameterValue);
    EXPECT_FALSE(parameter.valid);
}

TEST(GridModelParameterTests, ResetClearsValidityUnitsAndApplyIndices)
{
    griddyn::GridParameter parameter{"activePower(MW)=12.5"};
    parameter.applyIndex = {1, 3};

    parameter.reset();

    EXPECT_FALSE(parameter.valid);
    EXPECT_FALSE(parameter.stringType);
    EXPECT_EQ(parameter.paramUnits, units::defunit);
    EXPECT_TRUE(parameter.applyIndex.empty());
}
