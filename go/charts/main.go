package main

import (
	"encoding/json"
	"errors"
	"fmt"
	"io/ioutil"
	"math/rand"
	"net/http"
	"sort"
	"strconv"
	"time"

	"github.com/go-echarts/go-echarts/v2/charts"
	"github.com/go-echarts/go-echarts/v2/components"
	"github.com/go-echarts/go-echarts/v2/opts"
)

// generate random data for line chart
func generateLineItems() []opts.LineData {
	items := make([]opts.LineData, 0)
	for i := 0; i < 7; i++ {
		items = append(items, opts.LineData{Value: rand.Intn(300)})
	}
	return items
}

func createGrafanaQuery(metricType, sessionId, resource, from, to string) string {
	return fmt.Sprintf(`https://grafana.cups.online/api/datasources/proxy/1/api/v1/query_range?query=%s%%7BtankId%%3D%%22%s%%22%%2C%%20resource%%3D%%22%s%%22%%7D&start=%v&end=%s&step=10`,
		metricType, sessionId,
		resource, from[:len(from)-3], to[:len(to)-3],
	)
}

func buildSecondsSumChart(responses []*Response) (*charts.Line, error) {

	line := charts.NewLine()
	line.SetGlobalOptions(
		charts.WithTitleOpts(opts.Title{
			Title: "All sum seconds",
		}),
		charts.WithLegendOpts(opts.Legend{Show: true}),
		charts.WithTooltipOpts(opts.Tooltip{Show: true}),
	)

	dataMap := make(map[time.Time]float64)
	for _, r := range responses {
		if r != nil && len(r.Data.Result) > 0 {
			for it := range r.Data.Result {
				result := r.Data.Result[it]
				for i := range result.Values {
					v := result.Values[i]
					ts := time.Unix((int64)(v[0].(float64)), 0)
					val, err := strconv.ParseFloat(v[1].(string), 64)
					if err != nil {
						return nil, err
					}
					dataMap[ts] += val
				}
			}
		}
	}
	var values []Value
	for k, v := range dataMap {
		values = append(values, Value{
			Timestamp: k,
			Value:     v,
		})
	}

	sort.Slice(values, func(i, j int) bool {
		return values[i].Timestamp.Unix() < values[j].Timestamp.Unix()
	})

	var xAxis []string
	var yAxis []opts.LineData
	for i, v := range values {
		xAxis = append(xAxis, v.Timestamp.Format("15:04:05"))
		if i == 0 {
			yAxis = append(yAxis, opts.LineData{Value: v.Value})
		} else {
			yAxis = append(yAxis, opts.LineData{Value: v.Value - values[i-1].Value})
		}
	}

	line.SetXAxis(xAxis).AddSeries("Sum", yAxis)
	return line, nil
}

func buildRequestLatencyChart(title string, count, seconds *Response) (*charts.Line, error) {
	if count == nil || len(count.Data.Result) == 0 {
		return nil, errors.New("%s empty data")
	}

	line := charts.NewLine()
	line.SetGlobalOptions(
		charts.WithTitleOpts(opts.Title{
			Title: title,
		}),
		charts.WithLegendOpts(opts.Legend{Show: true}),
		charts.WithTooltipOpts(opts.Tooltip{Show: true}),
	)
	for it := range count.Data.Result {
		countResult := count.Data.Result[it]
		secondsResult := seconds.Data.Result[it]
		if countResult.Metric.Failed != secondsResult.Metric.Failed {
			panic("metric not equal")
		}
		var values []Value
		for i := range countResult.Values {
			count := countResult.Values[i]
			seconds := secondsResult.Values[i]
			tsCount := time.Unix((int64)(count[0].(float64)), 0)
			tsSeconds := time.Unix((int64)(seconds[0].(float64)), 0)
			if tsCount.Unix() != tsSeconds.Unix() {
				panic("timestamps not equal")
			}
			countVal, err := strconv.ParseFloat(count[1].(string), 64)
			if err != nil {
				return nil, err
			}
			secondsVal, err := strconv.ParseFloat(seconds[1].(string), 64)
			if err != nil {
				return nil, err
			}
			values = append(values, Value{
				Timestamp: tsCount,
				Value:     secondsVal / countVal * 1000.0,
			})
		}

		sort.Slice(values, func(i, j int) bool {
			return values[i].Timestamp.Unix() < values[j].Timestamp.Unix()
		})

		var xAxis []string
		var yAxis []opts.LineData
		for _, v := range values {
			xAxis = append(xAxis, v.Timestamp.Format("15:04:05"))
			yAxis = append(yAxis, opts.LineData{Value: v.Value})
		}

		line.SetXAxis(xAxis).AddSeries(fmt.Sprintf("Failed=%v", countResult.Metric.Failed), yAxis)
	}

	return line, nil
}
func buildChart(url string, title string) (*charts.Line, *Response, error) {
	resp, err := http.Get(url)
	if err != nil {
		return nil, nil, err
	}
	data, err := ioutil.ReadAll(resp.Body)
	if err != nil {
		return nil, nil, err
	}

	var response Response
	if err := json.Unmarshal(data, &response); err != nil {
		return nil, nil, err
	}
	if len(response.Data.Result) == 0 {
		return nil, nil, errors.New(fmt.Sprintf("%s:%s", url, string(data)))
	}

	line := charts.NewLine()
	line.SetGlobalOptions(
		charts.WithTitleOpts(opts.Title{
			Title: title,
		}),
		charts.WithLegendOpts(opts.Legend{Show: true}),
		charts.WithTooltipOpts(opts.Tooltip{Show: true}),
	)
	for _, result := range response.Data.Result {
		var values []Value
		for _, v := range result.Values {
			ts := time.Unix((int64)(v[0].(float64)), 0)
			val, err := strconv.ParseFloat(v[1].(string), 64)
			if err != nil {
				return nil, nil, err
			}
			values = append(values, Value{
				Timestamp: ts,
				Value:     val,
			})
		}

		sort.Slice(values, func(i, j int) bool {
			return values[i].Timestamp.Unix() < values[j].Timestamp.Unix()
		})

		var xAxis []string
		var yAxis []opts.LineData
		for i, v := range values {
			xAxis = append(xAxis, v.Timestamp.Format("15:04:05"))
			if i == 0 {
				yAxis = append(yAxis, opts.LineData{Value: v.Value})
			} else {
				yAxis = append(yAxis, opts.LineData{Value: v.Value - values[i-1].Value})
			}
		}

		line.SetXAxis(xAxis).AddSeries(fmt.Sprintf("Failed=%v", result.Metric.Failed), yAxis)
	}

	return line, &response, nil
}
func httpserver(w http.ResponseWriter, r *http.Request) {
	sessionId, ok := r.URL.Query()["sessionId"]
	if !ok {
		w.Write([]byte("sessionId not found"))
		return
	}
	from, ok := r.URL.Query()["from"]
	if !ok {
		w.Write([]byte("from not found"))
		return
	}

	to, ok := r.URL.Query()["to"]
	if !ok {
		w.Write([]byte("to not found"))
		return
	}

	page := components.NewPage()
	exploreCount, exploreCountResponse, err := buildChart(createGrafanaQuery("task_openapi_http_request_duration_seconds_count", sessionId[0], "explore", from[0], to[0]), "Explore count")
	if err != nil {
		fmt.Println(err)
	} else {
		page.AddCharts(exploreCount)
	}

	digCount, digCountResponse, err := buildChart(createGrafanaQuery("task_openapi_http_request_duration_seconds_count", sessionId[0], "dig", from[0], to[0]), "Dig count")
	if err != nil {
		fmt.Println(err)
	} else {
		page.AddCharts(digCount)
	}

	licensesCount, licensesCountResponse, err := buildChart(createGrafanaQuery("task_openapi_http_request_duration_seconds_count", sessionId[0], "licenses", from[0], to[0]), "Licenses count")
	if err != nil {
		fmt.Println(err)
	} else {
		page.AddCharts(licensesCount)
	}

	cashCount, cashCountResponse, err := buildChart(createGrafanaQuery("task_openapi_http_request_duration_seconds_count", sessionId[0], "cash", from[0], to[0]), "Cash count")
	if err != nil {
		fmt.Println(err)
	} else {
		page.AddCharts(cashCount)
	}

	exploreSeconds, exploreSecondsResponse, err := buildChart(createGrafanaQuery("task_openapi_http_request_duration_seconds_sum", sessionId[0], "explore", from[0], to[0]), "Explore seconds")
	if err != nil {
		fmt.Println(err)
	} else {
		page.AddCharts(exploreSeconds)
	}

	digSeconds, digSecondsResponse, err := buildChart(createGrafanaQuery("task_openapi_http_request_duration_seconds_sum", sessionId[0], "dig", from[0], to[0]), "Dig seconds")
	if err != nil {
		fmt.Println(err)
	} else {
		page.AddCharts(digSeconds)
	}

	licensesSeconds, licencesSecondsResponse, err := buildChart(createGrafanaQuery("task_openapi_http_request_duration_seconds_sum", sessionId[0], "licenses", from[0], to[0]), "Licenses seconds")
	if err != nil {
		fmt.Println(err)
	} else {
		page.AddCharts(licensesSeconds)
	}

	cashSeconds, cashSecondsResponse, err := buildChart(createGrafanaQuery("task_openapi_http_request_duration_seconds_sum", sessionId[0], "cash", from[0], to[0]), "Cash seconds")
	if err != nil {
		fmt.Println(err)
	} else {
		page.AddCharts(cashSeconds)
	}

	exploreLatencyChart, err := buildRequestLatencyChart("Explore latency ms", exploreCountResponse, exploreSecondsResponse)
	if err != nil {
		fmt.Println(err)
	} else {
		page.AddCharts(exploreLatencyChart)
	}

	digLatencyChart, err := buildRequestLatencyChart("Dig latency ms", digCountResponse, digSecondsResponse)
	if err != nil {
		fmt.Println(err)
	} else {
		page.AddCharts(digLatencyChart)
	}

	licensesLatencyChart, err := buildRequestLatencyChart("Licenses latency ms", licensesCountResponse, licencesSecondsResponse)
	if err != nil {
		fmt.Println(err)
	} else {
		page.AddCharts(licensesLatencyChart)
	}
	cashLatencyChart, err := buildRequestLatencyChart("Cash latency ms", cashCountResponse, cashSecondsResponse)
	if err != nil {
		fmt.Println(err)
	} else {
		page.AddCharts(cashLatencyChart)
	}

	totalSecondsSum, err := buildSecondsSumChart([]*Response{exploreSecondsResponse, digSecondsResponse, cashSecondsResponse, licencesSecondsResponse})
	if err != nil {
		fmt.Println(err)
	} else {
		page.AddCharts(totalSecondsSum)
	}

	if err := page.Render(w); err != nil {
		panic(err)
	}

}
func main() {
	http.HandleFunc("/", httpserver)
	if err := http.ListenAndServe(":8082", nil); err != nil {
		panic(err)
	}
}

type Response struct {
	Data Data `json:"data"`
}

type Data struct {
	Result []Result `json:"result"`
}

type Result struct {
	Metric Metric          `json:"metric"`
	Values [][]interface{} `json:"values"`
}

type Metric struct {
	Failed string `json:"failed"`
}

type Value struct {
	Timestamp time.Time
	Value     float64
}
