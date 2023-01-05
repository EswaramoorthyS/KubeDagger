/*
Copyright © 2021 GUILLAUME FOURNIER

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

package run

import (
	"fmt"
	"os"

	"github.com/sirupsen/logrus"
)

// CLIOptions are the command line options of ssh-probe
type CLIOptions struct {
	LogLevel logrus.Level
	Target   string
	From     string
	To       string
	// fs_watch options
	InContainer bool
	Active      bool
	Output      string
	// pipe_prog options
	Backup bool
	// docker options
	Override int
	Ping     int
	// postgres options
	Role   string
	Secret string
	// network discovery scan
	IP               string
	Port             string
	Range            string
	ActiveDiscovery  bool
	PassiveDiscovery bool
}

// LogLevelSanitizer is a log level sanitizer that ensures that the provided log level exists
type LogLevelSanitizer struct {
	logLevel *logrus.Level
}

// NewLogLevelSanitizer creates a new instance of LogLevelSanitizer. The sanitized level will be written in the provided
// logrus level
func NewLogLevelSanitizer(sanitizedLevel *logrus.Level) *LogLevelSanitizer {
	*sanitizedLevel = logrus.InfoLevel
	return &LogLevelSanitizer{
		logLevel: sanitizedLevel,
	}
}

func (lls *LogLevelSanitizer) String() string {
	return fmt.Sprintf("%v", *lls.logLevel)
}

func (lls *LogLevelSanitizer) Set(val string) error {
	sanitized, err := logrus.ParseLevel(val)
	if err != nil {
		return err
	}
	*lls.logLevel = sanitized
	return nil
}

func (lls *LogLevelSanitizer) Type() string {
	return "string"
}

// TargetParser parses the target from the environment variables or from the CLI arguments
type TargetParser struct {
	target *string
}

// NewTargetParser returns a new instance of TargetParser
func NewTargetParser(target *string) *TargetParser {
	*target = "http://localhost:8000"
	return &TargetParser{
		target: target,
	}
}

func (tp *TargetParser) Type() string {
	return "string"
}

func (tp *TargetParser) Set(val string) error {
	target := os.Getenv("EBPFKIT_TARGET")
	if len(target) > 0 {
		*tp.target = target
	} else if len(val) > 0 {
		*tp.target = val
	} else {
		*tp.target = "http://localhost:8000"
	}
	return nil
}

func (tp *TargetParser) String() string {
	return fmt.Sprintf("%v", *tp.target)
}
