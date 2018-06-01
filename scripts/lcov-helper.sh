#!/bin/bash

if [[ "$1" ]]; then
    subcmd=$(tr '[:upper:]' '[:lower:]' <<< $1)
    case "${subcmd}" in
        setup-counters)
            lcov --directory . -- zerocounters > /dev/null
            lcov --no-external --capture --initial --directory . --output-file coverage.base > /dev/null
            ;;
        gather-coverage-info)
            GCOV_TOOL=gcov
            if [[ "$2" ]]; then
                GCOV_TOOL=$2
            fi
            lcov --gcov-tool $GCOV_TOOL --no-external --directory . --capture --output-file coverage.info &> /dev/null
            lcov -a coverage.base -a coverage.info --output-file coverage.total > /dev/null
            lcov --remove coverage.total 'test/*' 'tests/*' 'ThirdParty/*' 'dependencies/*' '/usr/*' --output-file coverage.info.cleaned > /dev/null
            ;;
        *)
            echo "Usage: $@ setup-counters|gather-coverage-info"
            ;;
    esac
else
    echo "Usage: $0 setup-counters|gather-coverage-info"
fi

