echo "Test script start!"
echo "##  Build started at $(date)"
make clean
make all
./project ./test/parse_inout_file/eingabe/eingabe.csv
