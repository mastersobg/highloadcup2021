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
	best := int64(math.MaxInt64)
	var bestArea [][]int
	//for k := 1500; k > 1000; k-- {
	for i := 51; i > 1; i-- {
		for j := 6; j > 1; j-- {
			if i <= j {
				continue
			}
			areas := [][]int{
				//{500, 1000},
				//{500, 100},
				{500, 10},
				//{k, 1},
				{500, 1},
				{i, 1},
				{j, 1},
				{1, 1},
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
			fmt.Printf("best requests: %v\n", best)
			fmt.Printf("best areas: %v\n", bestArea)
		}
	}
	//}
	return nil
}

func run(areas [][]int) (int64, error) {
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
		if strings.HasPrefix(line, "DEBUG explore requests: ") {
			parsed, err := strconv.ParseInt(strings.TrimPrefix(line, "DEBUG explore requests: "), 10, 32)
			if err != nil {
				return 0, err
			}
			return parsed, nil
		}
	}
	return 0, errors.New("requests number not found")
}
