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

package log

import (
	"os"
	"path"

	"go.uber.org/zap"
	"go.uber.org/zap/zapcore"
	lumberjack "gopkg.in/natefinch/lumberjack.v2"
)

// Configuration for logging
type Config struct {
	// EncodeLogsAsJson makes the log framework log JSON
	EncodeLogsAsJson bool
	// FileLoggingEnabled makes the framework log to a file
	// the fields below can be skipped if this value is false!
	FileLoggingEnabled bool
	// Directory to log to to when filelogging is enabled
	Directory string
	// Filename is the name of the logfile which will be placed inside the directory
	Filename string
	// MaxSize the max size in MB of the logfile before it's rolled
	MaxSize int
	// MaxBackups the max number of rolled files to keep
	MaxBackups int
	// MaxAge the max age in days to keep a logfile
	MaxAge int
}

// How to log, by example:
// logger.Info("Importing new file, zap.String("source", filename), zap.Int("size", 1024))
// To log a stacktrace:
// logger.Error("It went wrong, zap.Stack())

// DefaultZapLogger is the default logger instance that should be used to log
// It's assigned a default value here for tests (which do not call log.Configure())
var DefaultZapLogger = newZapLogger(false, os.Stdout)

// Debug Log a message at the debug level. Messages include any context that's
// accumulated on the logger, as well as any fields added at the log site.
//
// Use zap.String(key, value), zap.Int(key, value) to log fields. These fields
// will be marshalled as JSON in the logfile and key value pairs in the console!
func Debug(msg string, fields ...zapcore.Field) {
	DefaultZapLogger.Debug(msg, fields...)
}

// Info log a message at the info level. Messages include any context that's
// accumulated on the logger, as well as any fields added at the log site.
//
// Use zap.String(key, value), zap.Int(key, value) to log fields. These fields
// will be marshalled as JSON in the logfile and key value pairs in the console!
func Info(msg string, fields ...zapcore.Field) {
	DefaultZapLogger.Info(msg, fields...)
}

// Warn log a message at the warn level. Messages include any context that's
// accumulated on the logger, as well as any fields added at the log site.
//
// Use zap.String(key, value), zap.Int(key, value) to log fields. These fields
// will be marshalled as JSON in the logfile and key value pairs in the console!
func Warn(msg string, fields ...zapcore.Field) {
	DefaultZapLogger.Warn(msg, fields...)
}

// Error Log a message at the error level. Messages include any context that's
// accumulated on the logger, as well as any fields added at the log site.
//
// Use zap.String(key, value), zap.Int(key, value) to log fields. These fields
// will be marshalled as JSON in the logfile and key value pairs in the console!
func Error(msg string, fields ...zapcore.Field) {
	DefaultZapLogger.Error(msg, fields...)
}

// Panic Log a message at the Panic level. Messages include any context that's
// accumulated on the logger, as well as any fields added at the log site.
//
// Use zap.String(key, value), zap.Int(key, value) to log fields. These fields
// will be marshalled as JSON in the logfile and key value pairs in the console!
func Panic(msg string, fields ...zapcore.Field) {
	DefaultZapLogger.Panic(msg, fields...)
}

// Fatal Log a message at the fatal level. Messages include any context that's
// accumulated on the logger, as well as any fields added at the log site.
//
// Use zap.String(key, value), zap.Int(key, value) to log fields. These fields
// will be marshalled as JSON in the logfile and key value pairs in the console!
func Fatal(msg string, fields ...zapcore.Field) {
	DefaultZapLogger.Fatal(msg, fields...)
}

// AtLevel logs the message at a specific log level
func AtLevel(level zapcore.Level, msg string, fields ...zapcore.Field) {
	switch level {
	case zapcore.DebugLevel:
		Debug(msg, fields...)
	case zapcore.PanicLevel:
		Panic(msg, fields...)
	case zapcore.ErrorLevel:
		Error(msg, fields...)
	case zapcore.WarnLevel:
		Warn(msg, fields...)
	case zapcore.InfoLevel:
		Info(msg, fields...)
	case zapcore.FatalLevel:
		Fatal(msg, fields...)
	default:
		Warn("Logging at unknown level", zap.Any("level", level))
		Warn(msg, fields...)
	}
}

// Configure sets up the logging framework
//
// In production, the container logs will be collected and file logging should be disabled. However,
// during development it's nicer to see logs as text and optionally write to a file when debugging
// problems in the containerized pipeline
//
// The output log file will be located at /var/log/{service name}/{service-name}.log and
// will be rolled when it reaches 20MB with a maximum of 1 backup.
func Configure(config Config) {
	writers := []zapcore.WriteSyncer{os.Stdout}
	if config.FileLoggingEnabled {
		writers = append(writers, newRollingFile(config))
	}

	DefaultZapLogger = newZapLogger(config.EncodeLogsAsJson, zapcore.NewMultiWriteSyncer(writers...))
	zap.RedirectStdLog(DefaultZapLogger)
	Info("logging configured",
		zap.Bool("fileLogging", config.FileLoggingEnabled),
		zap.Bool("jsonLogOutput", config.EncodeLogsAsJson),
		zap.String("logDirectory", config.Directory),
		zap.String("fileName", config.Filename),
		zap.Int("maxSizeMB", config.MaxSize),
		zap.Int("maxBackups", config.MaxBackups),
		zap.Int("maxAgeInDays", config.MaxAge))
}

func newRollingFile(config Config) zapcore.WriteSyncer {
	if err := os.MkdirAll(config.Directory, 0744); err != nil {
		Error("failed create log directory", zap.Error(err), zap.String("path", config.Directory))
		return nil
	}

	return zapcore.AddSync(&lumberjack.Logger{
		Filename:   path.Join(config.Directory, config.Filename),
		MaxSize:    config.MaxSize,    // megabytes
		MaxAge:     config.MaxAge,     // days
		MaxBackups: config.MaxBackups, // files
	})
}

func newZapLogger(encodeAsJSON bool, output zapcore.WriteSyncer) *zap.Logger {
	encCfg := zapcore.EncoderConfig{
		TimeKey:        "timestamp",
		LevelKey:       "level",
		NameKey:        "logger",
		CallerKey:      "caller",
		MessageKey:     "msg",
		StacktraceKey:  "stacktrace",
		EncodeLevel:    zapcore.LowercaseLevelEncoder,
		EncodeTime:     zapcore.ISO8601TimeEncoder,
		EncodeDuration: zapcore.NanosDurationEncoder,
	}
	if !encodeAsJSON {
		encCfg.EncodeLevel = zapcore.CapitalColorLevelEncoder
	}

	encoder := zapcore.NewConsoleEncoder(encCfg)
	if encodeAsJSON {
		encoder = zapcore.NewJSONEncoder(encCfg)
	}

	return zap.New(zapcore.NewCore(encoder, output, zap.NewAtomicLevel()))
}
