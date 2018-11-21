#!/bin/bash

if [[ "$1" ]]; then
    subcmd=$(tr '[:upper:]' '[:lower:]' <<< $1)
    case "${subcmd}" in
        setup-counters)
            lcov --directory . --zerocounters > /dev/null
            lcov --no-external --capture --initial --directory . --output-file coverage.base > /dev/null
            ;;
        gather-coverage-info)
            shift
            GCOV_TOOL=gcov
            while [[ $# -gt 0 ]]
            do
                key="$1"
                case $key in
                    -g|--gcov)
                        GCOV_TOOL="$2"
                        shift
                        shift
                        ;;
                    --coveralls)
                        SUBMIT_COVERALLS=true
                        shift
                        ;;
                    --codecov)
                        SUBMIT_CODECOV=true
                        shift
                        ;;
                    *)
                        shift
                        ;;
            esac
            done

            if [[ "$SUBMIT_CODECOV" != "true" || "$SUBMIT_COVERALLS" == "true" ]]; then
                # Get the coverage info from the test runs into a file
                lcov --gcov-tool $GCOV_TOOL --no-external --directory . --capture --output-file coverage.info &> /dev/null
                # Combine the base coverage info with info from running programs
                lcov -a coverage.base -a coverage.info --output-file coverage.total > /dev/null
                # Clean-up the coverage info
                lcov --remove coverage.total 'test/*' 'tests/*' 'ThirdParty/*' 'dependencies/*' '/usr/*' --output-file coverage.info.cleaned > /dev/null
            fi

            # Submit coverage info to dashboardv
            if [[ "$SUBMIT_COVERALLS" == "true" ]]; then
                coveralls --gcov ${GCOV_TOOL} --lcov-file coverage.info.cleaned > /dev/null
            fi
            if [[ "$SUBMIT_CODECOV" == "true" ]]; then
                curl -sf https://codecov.io/bash > /tmp/codecov.sh
                STATUS=$?
                if [[ "$STATUS" == "0" ]]; then
                    bash /tmp/codecov.sh -x ${GCOV_TOOL} > /dev/null
                else
                    echo "Codecov is currently down, skipping submission"
                fi
                rm /tmp/codecov.sh
            fi
            ;;
        *)
            echo "Usage: $@ setup-counters|gather-coverage-info"
            ;;
    esac
else
    echo "Usage: $0 setup-counters|gather-coverage-info"
fi
