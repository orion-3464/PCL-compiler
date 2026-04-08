#!/bin/bash

# This is a PCL parser unit test

TEST_DIR="../../ntua_compilers/pcl/programs"
PARSER="./../pcl"

GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m'

PASSED=0
FAILED=0
TOTAL=0

for file in "$TEST_DIR"/*.pcl; do
    ((TOTAL++))
    filename=$(basename "$file")
    
    STATUS="$($PARSER < $file)"
    if [ "$STATUS" == "Success" ]; then
        echo -e "FILE:$(basename $file) -> PARSING STATUS: ${GREEN}$STATUS${NC}\n"
        ((PASSED++))
    else
        echo -e "FILE:$(basename $file) -> PARSING STATUS: ${RED}$STATUS${NC}\n"
        ((FAILED++))
    fi
done

echo "TOTAL FILES: $TOTAL"
echo "PASSED: $PASSED/$TOTAL"
echo "FAILED: $FAILED/$TOTAL"
