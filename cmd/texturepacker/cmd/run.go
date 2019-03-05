// Copyright © 2019 Master.G
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

package cmd

import (
	"encoding/json"

	"github.com/master-g/texturepacker/pkg/config"
	"github.com/master-g/texturepacker/pkg/log"
	"github.com/master-g/texturepacker/pkg/signal"
	"github.com/oklog/run"
	"github.com/spf13/cobra"
	"github.com/spf13/viper"
	"go.uber.org/zap"
)

var paramList = []config.Flag{
	{Name: "log.encode-logs-as-json", Type: config.Bool, Shorthand: "", Value: true, Usage: "EncodeLogsAsJson makes the log framework log JSON"},
	{Name: "log.file-logging-enabled", Type: config.Bool, Shorthand: "", Value: false, Usage: "FileLoggingEnabled makes the framework log to a file"},
	{Name: "log.directory", Type: config.String, Shorthand: "", Value: "", Usage: "Directory to log to to when file-logging-enabled is enabled"},
	{Name: "log.filename", Type: config.String, Shorthand: "", Value: "agent.log", Usage: "Filename is the name of the logfile which will be placed inside the directory"},
	{Name: "log.max-size", Type: config.Int, Shorthand: "", Value: 20, Usage: "MaxSize the max size in MB of the logfile before it's rolled"},
	{Name: "log.max-backups", Type: config.Int, Shorthand: "", Value: 1, Usage: "MaxBackups the max number of rolled files to keep"},
	{Name: "log.max-age", Type: config.Int, Shorthand: "", Value: 7, Usage: "MaxAge the max age in days to keep a logfile"},
}

// serveCmd represents the serve command
var serveCmd = &cobra.Command{
	Use:   "run",
	Short: "run texturepacker",
	Long:  `run texturepacker`,
	Run: func(cmd *cobra.Command, args []string) {
		initLogger()
		configApplication()
		runApplication()
	},
}

func init() {
	rootCmd.AddCommand(serveCmd)

	// Here you will define your flags and configuration settings.

	// Cobra supports Persistent Flags which will work for this command
	// and all subcommands, e.g.:
	// serveCmd.PersistentFlags().String("foo", "", "A help for foo")

	// Cobra supports local flags which will only run when this command
	// is called directly, e.g.:
	// bind flags with viper
	err := config.BindFlags(serveCmd, paramList)
	if err != nil {
		log.Fatal("unable to config texturepacker", zap.Error(err))
	}
}

func initLogger() {
	j, _ := json.Marshal(viper.AllSettings())
	cfg := log.Config{
		EncodeLogsAsJson:   viper.GetBool("log.encode-logs-as-json"),
		FileLoggingEnabled: viper.GetBool("log.file-logging-enabled"),
		Directory:          viper.GetString("log.directory"),
		Filename:           viper.GetString("log.filename"),
		MaxSize:            viper.GetInt("log.max-size"),
		MaxBackups:         viper.GetInt("log.max-backups"),
		MaxAge:             viper.GetInt("log.max-age"),
	}
	log.Configure(cfg)
	log.Info(string(j))
}

func configApplication() {

}

func runApplication() {
	var g run.Group

	// signal handler
	{
		// cancel channel for interruption
		cancel := make(chan struct{})
		g.Add(func() error {
			signal.Start(cancel)
			return nil
		}, func(error) {
			// interrupt from other goroutine
			close(cancel)
		})
	}

	err := g.Run()
	log.Info("program exit", zap.Error(err))
}
