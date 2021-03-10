package main

import (
	"container/heap"
	"fmt"
	"math"
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

func NewArea(x1, y1, x2, y2, cnt, depth int) area {
	return area{
		x1:    x1,
		y1:    y1,
		x2:    x2,
		y2:    y2,
		cnt:   cnt,
		depth: depth,
		val:   float64(cnt) / float64((x2-x1+1)*(y2-y1+1)),
	}
}

type area struct {
	x1, y1, x2, y2 int
	cnt            int
	depth          int
	val            float64
}

type AreaSlice []area

func (a *AreaSlice) Len() int {
	return len(*a)
}

func (a *AreaSlice) Less(i, j int) bool {
	a1 := (*a)[i]
	a2 := (*a)[j]
	if math.Abs(a1.val-a2.val) < 1e-6 {
		sq1 := (a1.x2 - a1.x1 + 1) * (a1.y2 - a1.y1 + 1)
		sq2 := (a2.x2 - a2.x1 + 1) * (a2.y2 - a2.y1 + 1)
		// TODO test different
		return sq1 < sq2
	} else {
		return a1.val > a2.val
	}
}

func (a *AreaSlice) Swap(i, j int) {
	arr := *a
	arr[i], arr[j] = arr[j], arr[i]
}
func (a *AreaSlice) Push(x interface{}) {
	*a = append(*a, x.(area))
}
func (a *AreaSlice) Pop() interface{} {
	arr := *a
	n := len(arr)
	x := arr[n-1]
	*a = arr[0 : n-1]
	return x
}

func simulateSubAreas() int64 {
	areas := [][]int{
		{50, 20},
		{5, 1},
		{1, 1},
	}
	state := generateState()

	capacity := maxCapacity

	var rec func(x1, y1, x2, y2, cnt, dept int) int
	rec = func(x1, y1, x2, y2, cnt, depth int) int {
		//fmt.Println(capacity)
		if x1 == x2 && y1 == y2 {
			return state[x1][y1]
		}
		if capacity <= 0 {
			return 0
		}
		rawQ := make(AreaSlice, 0, 1<<20)
		q := &rawQ
		h := areas[depth][0]
		w := areas[depth][1]

		subCnt := 0
	global:
		for i := x1; i <= x2; i += h {
			for j := y1; j <= y2; j += w {
				capacity -= costs[h*w]
				if capacity <= 0 || cnt-subCnt <= 0 {
					// TODO fix counting 1x1 regions
					break global
				}
				cnt := calcTreasuries(state, i, j, i+h-1, j+w-1)
				if cnt > 0 {
					*q = append(*q, area{
						x1:  i,
						y1:  j,
						x2:  i + h - 1,
						y2:  j + w - 1,
						cnt: cnt,
					})
				}
				subCnt += cnt
			}
		}

		sort.Sort(q)
		ret := 0
		for _, v := range *q {
			ret += rec(v.x1, v.y1, v.x2, v.y2, v.cnt, depth+1)
			if capacity <= 0 {
				break
			}
		}
		return ret
	}

	return int64(rec(0, 0, maxSize-1, maxSize-1, 1<<30, 0))
}

var state = generateState()

func simulateSubAreasSharedQueue(areas [][]int) int64 {
	capacity := maxCapacity

	rawq := make(AreaSlice, 0, 1<<20)
	q := &rawq
	heap.Init(q)
	heap.Push(q, NewArea(0, 0, maxSize-1, maxSize-1, maxSize*maxSize, 0))

	ret := 0
gl:
	for {
		a := heap.Pop(q).(area)
		x1 := a.x1
		y1 := a.y1
		x2 := a.x2
		y2 := a.y2

		h := areas[a.depth][0]
		w := areas[a.depth][1]

		foundSubCnt := 0
	innerLoop:
		for i := x1; i <= x2; i += h {
			for j := y1; j <= y2; j += w {
				capacity -= costs[h*w]
				if capacity <= 0 {
					break gl
				}
				cnt := calcTreasuries(state, i, j, i+h-1, j+w-1)
				foundSubCnt += cnt
				if cnt > 0 {
					if h == 1 && w == 1 {
						ret += state[i][j]
					} else {
						next := NewArea(i, j, i+h-1, j+w-1, cnt, a.depth+1)
						heap.Push(q, next)
					}
				}

				if a.cnt-foundSubCnt <= 0 {
					break innerLoop
				}
			}
		}
	}

	return int64(ret)
}

type Area struct {
	parent         *Area
	children       []*Area
	x1, y1, x2, y2 int
	actualCount    int
	explored       bool
	depth          int
	expectedCount  float64
	heapIndex      int
}

func (a Area) getHeight() int {
	return a.x2 - a.x1 + 1
}

func (a Area) getWidth() int {
	return a.y2 - a.y1 + 1
}

type AreaArray []*Area

func (a *AreaArray) Len() int {
	return len(*a)
}

func (a *AreaArray) Less(i, j int) bool {
	v1 := (*a)[i]
	v2 := (*a)[j]
	//if math.Abs(v1.expectedCount-v2.expectedCount) < 1e-6 {
	//	sq1 := (v1.x2 - v1.x1 + 1) * (v1.y2 - v1.y1 + 1)
	//	sq2 := (v2.x2 - v2.x1 + 1) * (v2.y2 - v2.y1 + 1)
	//	return sq1 < sq2
	//}
	return v1.expectedCount > v2.expectedCount
}

func (a *AreaArray) Swap(i, j int) {
	arr := *a
	arr[i], arr[j] = arr[j], arr[i]
	arr[i].heapIndex = i
	arr[j].heapIndex = j
}

func (a *AreaArray) Push(x interface{}) {
	n := len(*a)
	item := x.(*Area)
	item.heapIndex = n
	*a = append(*a, item)
}

func (a *AreaArray) Pop() interface{} {
	arr := *a
	n := len(arr)
	x := arr[n-1]
	arr[n-1] = nil
	x.heapIndex = -1
	*a = arr[0 : n-1]
	return x
}

var _ heap.Interface = (*AreaArray)(nil)

func simulateSmartAreasSharedQueue(areas [][]int) int64 {
	//state := generateState()
	root := &Area{
		parent:        nil,
		x1:            0,
		y1:            0,
		x2:            maxSize - 1,
		y2:            maxSize - 1,
		actualCount:   calcTreasuries(state, 0, 0, maxSize-1, maxSize-1),
		explored:      true,
		expectedCount: 0,
	}

	rawq := make(AreaArray, 0, 1<<20)
	q := &rawq
	heap.Init(q)
	capacity := maxCapacity

	for i := 0; i < maxSize; i += areas[0][0] {
		for j := 0; j < maxSize; j += areas[0][1] {
			a := &Area{
				parent:        root,
				x1:            i,
				y1:            j,
				x2:            i + areas[0][0] - 1,
				y2:            j + areas[0][1] - 1,
				depth:         1,
				explored:      false,
				expectedCount: float64(root.actualCount) / float64(maxSize*maxSize),
			}
			root.children = append(root.children, a)
			heap.Push(q, a)
		}
	}

	foundTreasuries := int64(0)
	moreThan1 := 0
	for {
		//fmt.Println(capacity)
		if capacity <= 0 {
			break
		}

		c := heap.Pop(q).(*Area)
		if math.Abs(c.expectedCount-1.0) < 1e-6 || c.expectedCount >= 1.0 {
			if c.getHeight() > 1 || c.getWidth() > 1 {
				fmt.Printf("%#v\n", *c)
			}
			moreThan1++
		} else {
			capacity -= costs[(c.x2-c.x1+1)*(c.y2-c.y1+1)]
		}
		trs := calcTreasuries(state, c.x1, c.y1, c.x2, c.y2)
		c.actualCount = trs
		c.explored = true
		if trs > 0 {
			if c.x1 == c.x2 && c.y1 == c.y2 {
				foundTreasuries += int64(trs)
			} else {
				h := areas[c.depth][0]
				w := areas[c.depth][1]
				for i := c.x1; i <= c.x2; i += h {
					for j := c.y1; j <= c.y2; j += w {
						a := &Area{
							parent:        c,
							x1:            i,
							y1:            j,
							x2:            i + h - 1,
							y2:            j + w - 1,
							depth:         c.depth + 1,
							explored:      false,
							expectedCount: float64(c.actualCount) / float64(c.getWidth()*c.getHeight()),
						}
						heap.Push(q, a)
						c.children = append(c.children, a)
					}
				}
			}
		}

		exploredTreasuries := 0
		nonExploredAreas := 0
		for i := 0; i < len(c.parent.children); i++ {
			child := c.parent.children[i]
			if child.explored {
				exploredTreasuries += child.actualCount
			} else {
				nonExploredAreas += (child.x2 - child.x1 + 1) * (child.y2 - child.y1 + 1)
			}
		}

		leftTreasuries := c.parent.actualCount - exploredTreasuries
		for i := 0; i < len(c.parent.children); i++ {
			if !c.parent.children[i].explored {
				c.parent.children[i].expectedCount = float64(leftTreasuries) / float64(nonExploredAreas)
				heap.Fix(q, c.parent.children[i].heapIndex)
			}
		}
	}

	fmt.Println("More that 1: ", moreThan1)
	return foundTreasuries
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
	//fmt.Printf("Simulate recursive sub areas: %d\n", simulate(simulateSubAreas))
	fmt.Printf("Simulate smart sub areas shared queue: %v\n", simulateSmartAreasSharedQueue([][]int{
		{50, 20},
		{5, 1},
		{1, 1},
	}))

	//rec(maxSize, maxSize, [][]int{})
	//fmt.Println("Best", maxSimulation, maxAreas)
}

var maxSimulation int64
var maxAreas [][]int

func rec(h, w int, areas [][]int) {
	if h == 1 && w == 1 {
		//fmt.Printf("Run simulation: %v\n", areas)
		ret := simulateSmartAreasSharedQueue(areas)
		if ret > maxSimulation {
			maxAreas = make([][]int, len(areas))
			for i, arr := range areas {
				maxAreas[i] = make([]int, len(arr))
				copy(maxAreas[i], arr)
			}
			maxSimulation = ret
		}
		fmt.Printf("Simulation: %v best: %v\n", ret, maxSimulation)
		return
	}
	for i := 1; i*i <= h; i++ {
		if h%i == 0 {
			for j := 1; j*j <= w; j++ {
				if w%j == 0 {
					areas = append(areas, []int{i, j})
					rec(i, j, areas)
					areas = areas[0 : len(areas)-1]
				}
			}
		}
	}
}
