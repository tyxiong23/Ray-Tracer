echo "g++ -fopenmp -o main main.cpp -std=c++14"
g++ -fopenmp -o main main.cpp -std=c++14

echo "time ./main testcases/smoke.txt smoke.png 1000"
time ./main testcases/smoke.txt smoke.png 1000

echo "time ./main testcases/smallpt.txt smallpt1.png 800"
time ./main testcases/smallpt.txt smallpt1.png 800

echo "time ./main testcases/Bezier.txt Bezier.png 1200"
time ./main testcases/Bezier.txt Bezier.png 1200

echo "time ./main testcases/mix_scene.txt mix_scene.png 1200"
time ./main testcases/mix_scene.txt mix_scene.png 1200 