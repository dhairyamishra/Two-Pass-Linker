#!/bin/bash

echo "---- Starting Lab1 Linker Setup and Run ----"

# 1️⃣ Extract the archive
echo "Extracting lab1_assign.tar.Z..."
tar -xzvf lab1_assign.tar.Z

# 2️⃣ Move into the lab1_assign directory
cd lab1_assign || { echo "Could not find lab1_assign directory! Exiting."; exit 1; }

# 3️⃣ Clean and compile
echo "Compiling linker.cpp..."
make clean
make

# 4️⃣ Create output directory
echo "Creating output directory: outdir"
mkdir -p outdir

# 5️⃣ Run the linker on all inputs
echo "Running runit.sh to generate outputs..."
chmod +x runit.sh gradeit.sh
./runit.sh outdir ./linker

# 6️⃣ Grade the outputs
echo "Grading outputs using gradeit.sh..."
./gradeit.sh . outdir

# 7️⃣ Generate logs for submission
echo "Generating make.log and gradeit.log..."
(hostname; make clean; make 2>&1) > make.log
./gradeit.sh . outdir > gradeit.log

# 8️⃣ Final report
echo "---- All steps completed! ----"
echo "Check make.log and gradeit.log for details."
echo "Zip the following for submission:"
echo "  - linker.cpp"
echo "  - makefile"
echo "  - make.log"
echo "  - gradeit.log"
echo "Zip it like this:"
echo "zip lab1_submit.zip linker.cpp makefile make.log gradeit.log"

echo "Good luck and happy linking! 🚀"

