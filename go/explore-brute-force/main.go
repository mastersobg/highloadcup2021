package main

import (
	"errors"
	"fmt"
	"math"
	"os/exec"
	"strconv"
	"strings"
)

func main() {
	if err := bruteForce(); err != nil {
		panic(err)
	}
}

func bruteForce() error {
	best := float64(math.MaxFloat64)
	var bestArea [][]int
	//for k := 1500; k > 1000; k-- {
	for i := 51; i > 1; i-- {
		for j := 6; j > 1; j-- {
			if i <= j {
				continue
			}
			areas := [][]int{
				{3500, 1},
				//{1750, 1},
				{875, 1},
				//{438, 1},
				{219, 1},
				//{110, 1},
				{55, 1},
				//{28, 1},
				{14, 1},
				{7, 1},
				{4, 1},
				{2, 1},
				{1, 1},
				//{256, 256},
				//{256, 128}
				//{256, 64},
				//{256, 32},
				//{100, 16},
				//{256, 4},
				//{512, 1},
				//{256, 1},
				//{64, 1},
				//{4, 1},
				//{2, 1},
				//{1, 1},

				//{1025, 10},
				////{2049, 1},
				//{1025, 1},
				//{513, 1},
				//{257, 1},
				//{129, 1},
				//{65, 1},
				//{33, 1},
				//{17, 1},
				//{9, 1},
				//{5, 1},
				//{3, 1},
				//{2, 1},
				//{1, 1},
				//{3500, 1},
				////{350, 1},
				////{35, 1},
				//{3, 1},
				//{1, 1},
				////{500, 1000},
				//{400, 100},
				////{400, 10},
				////{400, 1},
				//{40, 1},
				//{4, 1},
				//{1, 1},
			}
			ret, err := run(areas)
			if err != nil {
				fmt.Println(err)
				continue
			}
			if ret < best {
				best = ret
				bestArea = make([][]int, len(areas))
				for i, v := range areas {
					bestArea[i] = make([]int, len(v))
					for j := range v {
						bestArea[i][j] = v[j]
					}
				}
			}
			fmt.Printf("best cost per treasure: %v\n", best)
			fmt.Printf("best areas: %v\n", bestArea)
		}
	}
	//}
	return nil
}

func run(areas [][]int) (float64, error) {
	var args []string
	for _, area := range areas {
		for _, p := range area {
			args = append(args, fmt.Sprintf("%v", p))
		}
	}

	cmd := exec.Command("./highloadcup2021", args...)
	cmd.Env = append(cmd.Env, "ADDRESS=localhost")
	out, err := cmd.CombinedOutput()
	fmt.Println(string(out))
	if err != nil {
		fmt.Println(err.Error())
		if !strings.Contains(err.Error(), "abort trap") {
			return 0, err
		}
	}

	for _, line := range strings.Split(string(out), "\n") {
		fmt.Println(line)
		if strings.HasPrefix(line, "DEBUG Avg cost per treasure: ") {
			parsed, err := strconv.ParseFloat(strings.TrimPrefix(line, "DEBUG Avg cost per treasure: "), 64)
			if err != nil {
				return 0, err
			}
			return parsed, nil
		}
	}
	return 0, errors.New("requests number not found")
}
