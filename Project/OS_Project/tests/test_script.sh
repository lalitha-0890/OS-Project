#!/bin/bash
# Test script for Linux IPC and Systems Programming Demonstrator

echo "==========================================="
echo "  OS Project - Automated Test Script"
echo "==========================================="
echo ""

# Build the project
echo "[1/4] Building project..."
make clean > /dev/null 2>&1
make
if [ $? -ne 0 ]; then
    echo "BUILD FAILED!"
    exit 1
fi
echo "Build successful!"
echo ""

# Test each module
echo "[2/4] Testing CO-1: System Calls..."
echo "1" | ./build/os_project > /dev/null 2>&1
if [ $? -eq 0 ]; then
    echo "CO-1: PASS"
else
    echo "CO-1: FAIL"
fi

echo "[3/4] Testing CO-2: Process Control..."
echo "2" | ./build/os_project > /dev/null 2>&1
if [ $? -eq 0 ]; then
    echo "CO-2: PASS"
else
    echo "CO-2: FAIL"
fi

echo "[4/4] Testing CO-3: IPC..."
echo "3" | ./build/os_project > /dev/null 2>&1
if [ $? -eq 0 ]; then
    echo "CO-3: PASS"
else
    echo "CO-3: FAIL"
fi

echo ""
echo "==========================================="
echo "  Test Complete"
echo "==========================================="
