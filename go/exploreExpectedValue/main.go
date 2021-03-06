package main

import (
	"fmt"
	"math/rand"
	"time"
)

const maxSize = 3500
const capacity = 600_000_000

var (
	costs = []int{
		0,
		1034,
		1025,
		1021,
		1094,
		1079,
		1089,
		1097,
		1555,
		1551,
		1553,
		1554,
		1556,
		1553,
		1555,
		1561,
		2079,
		2084,
		2056,
		2061,
		2060,
		2045,
		2046,
		2053,
		2056,
		2046,
		2050,
		2050,
		2050,
		2050,
		2050,
		2050,
		2550,
		2550,
		2550,
		2550,
		2550,
		2550,
		2550,
		2550,
		2550,
		2550,
		2550,
		2550,
		2550,
		2550,
		2550,
		2550,
		2550,
		2550,
		2550,
		2550,
		2550,
		2550,
		2550,
		2550,
		2550,
		2550,
		2550,
		2550,
		2550,
		2550,
		2550,
		2550,
	}
)

func simulateRandom() int64 {
	state := generateState()
	totalCapacity := capacity
	foundTreasuries := int64(0)
	for totalCapacity > 0 {
		x := rand.Int31n(maxSize)
		y := rand.Int31n(maxSize)
		totalCapacity -= costs[1]
		if state[x][y] > 0 {
			foundTreasuries += int64(state[x][y])
		}
	}

	return foundTreasuries
}

func simulateSeqArea7() int64 {
	x := 0
	y := 0

	state := generateState()
	totalCapacity := capacity
	foundTreasuries := int64(0)
	for {
		if totalCapacity-costs[7] < 0 {
			break
		}
		totalCapacity -= costs[7]

		trs := int64(0)
		for i := 0; i < 7; i++ {
			if state[x][y+i] > 0 {
				trs += int64(state[x][y+i])
				break
			}
		}
		if trs > 0 {
			foundTreasuries += trs
			totalCapacity -= 7 * costs[1]
		}

		y += 7
		if y == maxSize {
			x++
			y = 0
		}
	}
	return foundTreasuries
}

func simulateSeqArea15() int64 {
	x := 0
	y := 0

	state := generateState()
	totalCapacity := capacity
	foundTreasuries := int64(0)
	for {
		if totalCapacity-costs[15] < 0 {
			break
		}
		totalCapacity -= costs[15]

		trs := int64(0)
		for i := 0; i < 15; i++ {
			if state[x][y+i] > 0 {
				trs += int64(state[x][y+i])
				break
			}
		}
		if trs > 0 {
			foundTreasuries += trs
			totalCapacity -= 15 * costs[1]
		}

		y += 15
		if y+15 >= maxSize {
			x++
			y = 0
		}
	}
	return foundTreasuries
}

func simulateSeqArea15Area7() int64 {
	x := 0
	y := 0

	state := generateState()
	totalCapacity := capacity
	foundTreasuries := int64(0)

	for {
		if totalCapacity-costs[15] < 0 {
			break
		}
		totalCapacity -= costs[15]

		trs := int64(0)
		first7 := int64(0)
		second7 := int64(0)
		for i := 0; i < 15; i++ {
			if state[x][y+i] > 0 {
				if i < 7 {
					first7++
				} else if i < 14 {
					second7++
				}
				trs += int64(state[x][y+i])
				break
			}
		}
		if trs > 0 {
			totalCapacity -= 2 * costs[7]
			if first7 > 0 {
				totalCapacity -= 7 * costs[1]
			}
			if second7 > 0 {
				totalCapacity -= 7 * costs[1]
			}

			if trs > first7+second7 {
				totalCapacity -= costs[1]
			}

			foundTreasuries += trs
		}

		y += 15
		if y+15 >= maxSize {
			x++
			y = 0
		}
	}
	return foundTreasuries
}

func simulateCascade(area int) int64 {
	state := generateState()
	totalCapacity := capacity
	foundTreasuries := int64(0)

	var rec func(x, y, area int) (int64, bool)
	rec = func(x, y, area int) (int64, bool) {
		if totalCapacity-costs[area] < 0 {
			return 0, true
		}

		totalCapacity -= costs[area]

		if area == 1 {
			return int64(state[x][y]), false
		}

		localTrs := 0
		for i := 0; i < area; i++ {
			localTrs += state[x][y+i]
		}

		if localTrs > 0 {
			// 3 7 15 31
			// 31 -> 15
			left, ok1 := rec(x, y, area/2)
			if ok1 {
				return left, ok1
			}
			right, ok2 := rec(x, y+area/2, area/2)
			if ok2 {
				return left + right, ok2
			}

			single, ok3 := rec(x, y+(area/2)*2, 1)
			return left + right + single, ok3
		}

		return 0, false
	}

	x := 0
	y := 0
	for {
		trs, shouldBreak := rec(x, y, area)
		foundTreasuries += trs
		if shouldBreak {
			break
		}

		y += area
		if y+area >= maxSize {
			x++
			y = 0
		}
	}
	return foundTreasuries
}
func simulateCascase7(area int) func() int64 {
	return func() int64 {
		return simulateCascade(area)
	}
}

func generateState() [maxSize][maxSize]int {
	rand.Seed(time.Now().Unix())
	var state [maxSize][maxSize]int
	totalTreasuries := 490_000
	for i := 0; i < totalTreasuries; i++ {
		x := rand.Int31n(maxSize)
		y := rand.Int31n(maxSize)
		state[x][y]++
	}
	return state
}

func simulate(f func() int64) int64 {
	iterations := int64(10)
	total := int64(0)
	for i := 0; i < int(iterations); i++ {
		total += f()
	}
	return total / iterations
}

func main() {
	fmt.Printf("Simple random: %v\n", simulate(simulateRandom))
	fmt.Printf("Area 7: %v\n", simulate(simulateSeqArea7))
	fmt.Printf("Area 15: %v\n", simulate(simulateSeqArea15))
	fmt.Printf("Area 15, area 7: %v\n", simulate(simulateSeqArea15Area7))
	fmt.Printf("Simulate cascade 3: %d\n", simulate(simulateCascase7(3)))
	fmt.Printf("Simulate cascade 7: %d\n", simulate(simulateCascase7(7)))
	fmt.Printf("Simulate cascade 15: %d\n", simulate(simulateCascase7(15)))
	fmt.Printf("Simulate cascade 31: %d\n", simulate(simulateCascase7(31)))
	fmt.Printf("Simulate cascade 63: %d\n", simulate(simulateCascase7(63)))
}
