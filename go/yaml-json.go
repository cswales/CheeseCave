//
// Brian Bulkowski copywrite 2015
//
// I found out that when we built the cheesecave project, we tried to use YAML,
// but the YAML we built was broken. And YAML is far out of favor now.
// We wanted to use YAML because it seemed to have better streaming support.
// json seems to have decent streaming support by taking your objects and putting them all on one line.
// This is OK for an embedded system because sometimes a power fault gives you corrupt data.
// a system that will simply skip data points when it hits a bad line then move on
// to the next seems much better.

// Thus, then, I worked to convert the YAML into streaming JSON.
// This file takes one of our old cheesecave history.yaml files and converts it into a pleasant JSON
// file. 
// If the line is bad, it skips it, so you might see files with different numbers of lines.

package main

import "flag"
import "fmt"
import "log"
import "gopkg.in/yaml.v2"
import "os"
import "bufio"
import "strings"


var testData = `
{ sensor: "sensor1" , time: "2015-02-03T06:13:08UTC" , epoch: 1422943988, temperature: 42.7, celsius: 5.9, humidity: 78.6 }
`


type T struct {
	Sensor string
	Time string
	Epoch int 
	Temperature float32
	Celsius float32
	Humidity float32
}

// read all the lines in the input file

func process(iFilename string, oFilename string, highLimitT float32, lowLimitT float32) {

	// Input file
	in_file, err := os.Open(iFilename)
	if err != nil {
	    log.Fatal(err)
	}
	defer in_file.Close()

	// output file
	out_file, err2 := os.Create(oFilename)
	if err2 != nil {
	    log.Fatal(err2)
	}
	defer out_file.Close()

	//
	var lineNum int = 0
	var outlines int = 0

	// scan through the input file, look at the lines

	scanner := bufio.NewScanner(in_file)
	for scanner.Scan() {

		lineNum++

		var line string = scanner.Text()

		// in_file was written in wrong form, wanted streaming
		// objects, not a huge array
		line = strings.TrimPrefix(line, "- ")
		//fmt.Println(Line)

		t := T{Sensor: "null", Time: "null"}

		err := yaml.Unmarshal([]byte(line), &t)
		if err != nil {
			// found cases of bad characters, want to just skip to next line
			log.Printf("error: %v: continuing\n",err)
		} 
//		else {
//			fmt.Printf("success: %v\n",t)
//		}

		// clean data. If abover or below limits, skip
		if t.Temperature > highLimitT {
			continue
		}
		if t.Temperature < lowLimitT {
			continue
		}

		// for each line, drop a well represented JSON line
		fmt.Fprintf(out_file, "{ \"sensor\": \"%s\", \"time\": \"%s\", \"epoch\": %d \"temperature\": %.2f, \"celsius\": %.2f, \"humidity\": %.2f }\n",
			t.Sensor, t.Time, t.Epoch, t.Temperature, t.Celsius, t.Humidity )

	}

	if err := scanner.Err(); err != nil {
	    log.Fatal(err)
	}

	fmt.Printf(" found and parsed %v lines\n", lineNum)

}

func test() {
	// try some testdata
	t := T{Sensor: "null", Time: "null"}

	log.Printf(" try this testdata %v\n",testData)

	err := yaml.Unmarshal([]byte(testData), &t)
	if err != nil {
			log.Fatalf("testdata unmarshal error: %v\n", err)
	}
	fmt.Printf("--- testdata t:\n%v\nsensorname %s\n", t, t.Sensor)
}

func main() {

	iFilenamePtr := flag.String("i", "/Users/brian/CheeseCave/pi1/sensor1-history.yaml", "inputFile")
	oFilenamePtr := flag.String("o","/Users/brian/CheeseCave/pi1/sensor1-history.csv", "outputFile")
	tempHighLimitPtr := flag.Int("hl", 100, "high temp limit in F to filter point out")
	tempLowLimitPtr := flag.Int("ll", 32, "low temp limit in F to filter point out")

	flag.Parse()

	fmt.Println("input: ", *iFilenamePtr)
	fmt.Println("output: ", *oFilenamePtr)
	fmt.Println("tempHighLimit", *tempHighLimitPtr)
	fmt.Println("tempLowLimit ", *tempLowLimitPtr)

	process(*iFilenamePtr, *oFilenamePtr, float32(*tempHighLimitPtr), float32(*tempLowLimitPtr))

}