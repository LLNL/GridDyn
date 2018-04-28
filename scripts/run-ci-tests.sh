#!/bin/bash

# Include quicktest, nightlytest, or releasetest in the branch name to run a particular set of tests
export CTEST_OUTPUT_ON_FAILURE=true

TEST_CONFIG="CI"
if [[ "$1" ]]; then
    test_label=$(tr '[:upper:]' '[:lower:]' <<< $1)
    case "${test_label}" in
        *quick*)
            TEST_CONFIG="Quick"
            ;;
        *nightly*)
            TEST_CONFIG="Nightly"
            ;;
        *release*)
            TEST_CONFIG="Release"
            ;;
        *)
            TEST_CONFIG="CI"
            ;;
    esac

# If no argument was given, but we are running in Travis, check the branch name for tests to run
elif [[ "$TRAVIS" == "true" ]]; then
    case "${TRAVIS_BRANCH}" in
        *quicktest*)
            TEST_CONFIG="Quick"
            ;;
        *nightlytest*)
            TEST_CONFIG="Nightly"
            ;;
        *releasetest*)
            TEST_CONFIG="Release"
            ;;
        *)
            TEST_CONFIG="CI"
            ;;
    esac
fi

echo "Running ${TEST_CONFIG} tests"

ctest -L ${TEST_CONFIG}
