#!/bin/bash

build_flag="O3"

while getopts 'd' opt; do
    case "$opt" in
    d)
        build_flag="g"
        ;;
    esac
done

inital_dir=$(pwd)
prev_dir="$(dirname "$0")/.."
cd $prev_dir
project_dir="$(pwd)"
build_dir="$project_dir/build"

if ! [ -d "$build_dir" ] ; then
    mkdir $build_dir
fi
cd $build_dir

exec_file="$build_dir/riichi"
dsym_file="$build_dir/riichi.dSYM"

if [ -f "$exec_file" ] ; then
    rm "$exec_file"
fi

if [ -d "$dsym_file" ] ; then
    rm -rf "$dsym_file"
fi


main_cfile="$project_dir/libriichigym/main.cpp"
cheater_cfile="$project_dir/libriichigym/riichi/auxs/riichi_cheater.cpp"
ui_cfile="$project_dir/libriichigym/riichi/auxs/riichi_ui.cpp"
gym_cfile="$project_dir/libriichigym/riichi/riichi_gym.cpp"
algo_cfile="$project_dir/libriichigym/riichi/riichi_algo.cpp"
algopattern_cfile="$project_dir/libriichigym/riichi/riichi_algo_pattern.cpp"
board_cfile="$project_dir/libriichigym/riichi/riichi_board.cpp"
display_cfile="$project_dir/libriichigym/riichi/riichi_display.cpp"
tile_cfile="$project_dir/libriichigym/riichi/riichi_tile.cpp"
util_cfile="$project_dir/libriichigym/riichi/riichi_util.cpp"
score_cfile="$project_dir/libriichigym/riichi/riichi_score.cpp"
yaku_cfile="$project_dir/libriichigym/riichi/riichi_yaku.cpp"


riichi_folder="$project_dir/libriichigym/riichi"
include_folder="$project_dir/libriichigym/include"


clang++ -std=gnu++2b -o riichi $main_cfile -$build_flag -Wall \
$cheater_cfile \
$ui_cfile \
$gym_cfile \
$algo_cfile \
$algopattern_cfile \
$board_cfile \
$display_cfile \
$tile_cfile \
$util_cfile \
$score_cfile \
$yaku_cfile \
-I $riichi_folder \
-I $include_folder

cd $inital_dir