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
	return fmt.Sprintf(`https://grafana.cups.online/api/datasources/proxy/1/api/v1/query_range?query=%s%%7BtankId%%3D%%22%s%%22%%2C%%20resource%%3D%%22%s%%22%%7D&start=%v&end=%s&step=15`,
		metricType, sessionId,
		resource, from[:len(from)-3], to[:len(to)-3],
	)
}

func buildChart(url string, title string) (*charts.Line, error) {
	resp, err := http.Get(url)
	if err != nil {
		return nil, err
	}
	data, err := ioutil.ReadAll(resp.Body)
	if err != nil {
		return nil, err
	}

	var response Response
	if err := json.Unmarshal(data, &response); err != nil {
		return nil, err
	}
	if len(response.Data.Result) == 0 {
		return nil, errors.New(fmt.Sprintf("%s:%s", url, string(data)))
	}
	var values []Value
	for _, v := range response.Data.Result[0].Values {
		ts := time.Unix((int64)(v[0].(float64)), 0)
		val, err := strconv.ParseFloat(v[1].(string), 64)
		if err != nil {
			return nil, err
		}
		values = append(values, Value{
			Timestamp: ts,
			Value:     val,
		})
	}

	sort.Slice(values, func(i, j int) bool {
		return values[i].Timestamp.Unix() < values[j].Timestamp.Unix()
	})

	line := charts.NewLine()
	line.SetGlobalOptions(
		charts.WithTitleOpts(opts.Title{
			Title: title,
		}),
		charts.WithLegendOpts(opts.Legend{Show: true}),
		charts.WithTooltipOpts(opts.Tooltip{Show: true}),
	)

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
	// Put data into instance
	line.SetXAxis(xAxis).
		AddSeries(title, yAxis)
	return line, nil
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
	exploreCount, err := buildChart(createGrafanaQuery("task_openapi_http_request_duration_seconds_count", sessionId[0], "explore", from[0], to[0]), "Explore count")
	if err != nil {
		fmt.Println(err)
	} else {
		page.AddCharts(exploreCount)
	}

	digCount, err := buildChart(createGrafanaQuery("task_openapi_http_request_duration_seconds_count", sessionId[0], "dig", from[0], to[0]), "Dig count")
	if err != nil {
		fmt.Println(err)
	} else {
		page.AddCharts(digCount)
	}

	licensesCount, err := buildChart(createGrafanaQuery("task_openapi_http_request_duration_seconds_count", sessionId[0], "licenses", from[0], to[0]), "Licenses count")
	if err != nil {
		fmt.Println(err)
	} else {
		page.AddCharts(licensesCount)
	}

	cashCount, err := buildChart(createGrafanaQuery("task_openapi_http_request_duration_seconds_count", sessionId[0], "cash", from[0], to[0]), "Cash count")
	if err != nil {
		fmt.Println(err)
	} else {
		page.AddCharts(cashCount)
	}

	exploreSeconds, err := buildChart(createGrafanaQuery("task_openapi_http_request_duration_seconds_sum", sessionId[0], "explore", from[0], to[0]), "Explore seconds")
	if err != nil {
		fmt.Println(err)
	} else {
		page.AddCharts(exploreSeconds)
	}

	digSeconds, err := buildChart(createGrafanaQuery("task_openapi_http_request_duration_seconds_sum", sessionId[0], "dig", from[0], to[0]), "Dig seconds")
	if err != nil {
		fmt.Println(err)
	} else {
		page.AddCharts(digSeconds)
	}

	licensesSeconds, err := buildChart(createGrafanaQuery("task_openapi_http_request_duration_seconds_sum", sessionId[0], "licenses", from[0], to[0]), "Licenses seconds")
	if err != nil {
		fmt.Println(err)
	} else {
		page.AddCharts(licensesSeconds)
	}

	cashSeconds, err := buildChart(createGrafanaQuery("task_openapi_http_request_duration_seconds_sum", sessionId[0], "cash", from[0], to[0]), "Cash seconds")
	if err != nil {
		fmt.Println(err)
	} else {
		page.AddCharts(cashSeconds)
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
	Values [][]interface{} `json:"values"`
}

type Value struct {
	Timestamp time.Time
	Value     float64
}
