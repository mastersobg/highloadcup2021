package main

import (
	"fmt"
	"math/rand"
	"sort"
	"time"
)

const maxSize = 3500
const maxCapacity = 600_000_000

type State [][]int

var (
	costs []int

	//costs = []int{
	//	0,
	//	1034,
	//	1025,
	//	1021,
	//	1094,
	//	1079,
	//	1089,
	//	1097,
	//	1555,
	//	1551,
	//	1553,
	//	1554,
	//	1556,
	//	1553,
	//	1555,
	//	1561,
	//	2079,
	//	2084,
	//	2056,
	//	2061,
	//	2060,
	//	2045,
	//	2046,
	//	2053,
	//	2056,
	//	2046,
	//	2050,
	//	2050,
	//	2050,
	//	2050,
	//	2050,
	//	2050,
	//	2550,
	//	2550,
	//	2550,
	//	2550,
	//	2550,
	//	2550,
	//	2550,
	//	2550,
	//	2550,
	//	2550,
	//	2550,
	//	2550,
	//	2550,
	//	2550,
	//	2550,
	//	2550,
	//	2550,
	//	2550,
	//	2550,
	//	2550,
	//	2550,
	//	2550,
	//	2550,
	//	2550,
	//	2550,
	//	2550,
	//	2550,
	//	2550,
	//	2550,
	//	2550,
	//	2550,
	//	2550,
	//}
)

func generateCosts() {
	costs = make([]int, 3500*3500+1)
	b := 8
	d := 1000
	for i := 0; i < len(costs); i++ {
		if i >= b {
			d += 500
			b = b * 2
		}
		costs[i] = d
	}
}

func simulateRandom() int64 {
	state := generateState()
	totalCapacity := maxCapacity
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
	totalCapacity := maxCapacity
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
	totalCapacity := maxCapacity
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
	totalCapacity := maxCapacity
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
	totalCapacity := maxCapacity
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

func generateState() State {
	rand.Seed(time.Now().Unix())
	state := make([][]int, maxSize)
	for i := 0; i < maxSize; i++ {
		state[i] = make([]int, maxSize)
	}
	totalTreasuries := 490_000
	for i := 0; i < totalTreasuries; i++ {
		x := rand.Int31n(maxSize)
		y := rand.Int31n(maxSize)
		state[x][y]++
	}
	return state
}

func simulate(f func() int64) int64 {
	iterations := int64(1)
	total := int64(0)
	for i := 0; i < int(iterations); i++ {
		total += f()
	}
	return total / iterations
}

func simulateBinSearch() int64 {
	state := generateState()
	capacity := maxCapacity

	var rec func(x1, y1, x2, y2 int) int
	rec = func(x1, y1, x2, y2 int) int {
		if capacity <= 0 {
			return 0
		}
		if x1 == x2 && y1 == y2 {
			return state[x1][y1]
		}
		if calcTreasuries(state, x1, y1, x2, y2) < 1 {
			return 0
		}

		ret := 0
		if x2-x1 > y2-y1 {
			mid := x1 + (x2-x1)/2
			l := calcTreasuries(state, x1, y1, mid, y2)
			r := calcTreasuries(state, mid+1, y1, x2, y2)
			area1 := (mid - x1 + 1) * (y2 - y1 + 1)
			area2 := (x2 - mid) * (y2 - y1 + 1)
			capacity = capacity - costs[area1] - costs[area2]
			if l > r {
				ret += rec(x1, y1, mid, y2)
				ret += rec(mid+1, y1, x2, y2)
			} else {
				ret += rec(mid+1, y1, x2, y2)
				ret += rec(x1, y1, mid, y2)
			}
		} else {
			mid := y1 + (y2-y1)/2
			l := calcTreasuries(state, x1, y1, x2, mid)
			r := calcTreasuries(state, x1, mid+1, x2, y2)
			area1 := (x2 - x1 + 1) * (mid - y1 + 1)
			area2 := (x2 - x1 + 1) * (y2 - mid)
			capacity = capacity - costs[area1] - costs[area2]
			if l > r {
				ret += rec(x1, y1, x2, mid)
				ret += rec(x1, mid+1, x2, y2)
			} else {
				ret += rec(x1, mid+1, x2, y2)
				ret += rec(x1, y1, x2, mid)
			}
		}
		return ret
	}
	return int64(rec(0, 0, maxSize-1, maxSize-1))
}

type area struct {
	x1, y1, x2, y2 int
	cnt            int
	depth          int
}

type AreaSlice []area

func (a AreaSlice) Len() int {
	return len(a)
}

func (a AreaSlice) Less(i, j int) bool {
	a1 := a[i]
	a2 := a[j]
	val1 := float64(a1.cnt) / float64((a1.x2-a1.x1+1)*(a1.y2-a1.y1+1))
	val2 := float64(a2.cnt) / float64((a2.x2-a2.x1+1)*(a2.y2-a2.y1+1))
	return val1 > val2
}

func (a AreaSlice) Swap(i, j int) {
	a[i], a[j] = a[j], a[i]
}

var _ sort.Interface = AreaSlice{}

func simulateSubAreas() int64 {
	areas := [][]int{
		{20, 50},
		{5, 10},
		{1, 1},
	}
	state := generateState()

	capacity := maxCapacity

	var rec func(x1, y1, x2, y2, dept int) int
	rec = func(x1, y1, x2, y2, depth int) int {
		fmt.Println(capacity)
		if x1 == x2 && y1 == y2 {
			return state[x1][y1]
		}
		q := make(AreaSlice, 0, 1<<20)
		h := areas[depth][0]
		w := areas[depth][1]

		for i := x1; i <= x2; i += h {
			for j := y1; j <= y2; j += w {
				capacity -= costs[h*w]
				if capacity <= 0 {
					return 0
				}
				cnt := calcTreasuries(state, i, j, i+h-1, j+w-1)
				if cnt > 0 {
					q = append(q, area{
						x1:  i,
						y1:  j,
						x2:  i + h - 1,
						y2:  j + w - 1,
						cnt: cnt,
					})
				}
			}
		}

		sort.Sort(q)
		ret := 0
		for _, v := range q {
			ret += rec(v.x1, v.y1, v.x2, v.y2, depth+1)
			if capacity <= 0 {
				break
			}
		}
		return ret
	}

	return int64(rec(0, 0, maxSize-1, maxSize-1, 0))
}

func calcTreasuries(state State, x1, y1, x2, y2 int) int {
	cnt := 0
	for i := x1; i <= x2; i++ {
		for j := y1; j <= y2; j++ {
			cnt += state[i][j]
		}
	}
	return cnt
}

func main() {
	generateCosts()
	//fmt.Printf("Simple random: %v\n", simulate(simulateRandom))
	//fmt.Printf("Area 7: %v\n", simulate(simulateSeqArea7))
	//fmt.Printf("Area 15: %v\n", simulate(simulateSeqArea15))
	//fmt.Printf("Area 15, area 7: %v\n", simulate(simulateSeqArea15Area7))
	//fmt.Printf("Simulate cascade 3: %d\n", simulate(simulateCascase7(3)))
	//fmt.Printf("Simulate cascade 7: %d\n", simulate(simulateCascase7(7)))
	//fmt.Printf("Simulate cascade 15: %d\n", simulate(simulateCascase7(15)))
	//fmt.Printf("Simulate cascade 31: %d\n", simulate(simulateCascase7(31)))
	//fmt.Printf("Simulate cascade 63: %d\n", simulate(simulateCascase7(63)))
	//fmt.Printf("Simulate bin search: %d\n", simulate(simulateBinSearch))
	fmt.Printf("Simulate recursive sub areas: %d\n", simulate(simulateSubAreas))

}
