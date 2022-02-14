# script to run all experiments (see code for parameters)

# convert to executable using chmod +x script.sh, then run with ./script.sh

project=A1
# set folder to save results
folder=~/temp/results/

export project
export folder

./Main.exe $project 1 0 $folder
./Main.exe $project 1 10 $folder
./Main.exe $project 1 25 $folder
./Main.exe $project 1 50 $folder
./Main.exe $project 1 100 $folder

./Main.exe $project 0 0 $folder
./Main.exe $project 0 10 $folder
./Main.exe $project 0 25 $folder
./Main.exe $project 0 50 $folder
./Main.exe $project 0 100 $folder

