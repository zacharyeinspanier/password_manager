source ../.env

g++ unit_tests/userAccountTests.cpp -o ./output/userAccountTests -std=c++20 -lsqlite3

echo "=================="
echo "User Account Tests"
echo "=================="
./output/userAccountTests
echo "=================="

g++ -g unit_tests/displayContentTests.cpp -o ./output/displayContentTests -std=c++20 -lsqlite3 

echo "=================="
echo "Display Content Tests"
echo "=================="
./output/displayContentTests
echo "=================="