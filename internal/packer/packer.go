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

package packer

import (
	"errors"
	"image"
	"image/png"
	"os"
	"sort"

	"github.com/sirupsen/logrus"
)

// Config of the texture packer
type Config struct {
	OutputWidth      int
	OutputHeight     int
	Padding          int
	OutputImagePath  string
	OutputSchemaPath string
	IgnoreLargeImage bool
}

type Packer struct {
	cfg    *Config
	root   *Node
	canvas *image.RGBA
}

// NewPacker returns a new packer instance created with config
func NewPacker(cfg *Config) *Packer {
	if cfg == nil {
		return nil
	}

	upLeft := image.Point{X: 0, Y: 0}
	lowRight := image.Point{X: cfg.OutputWidth, Y: cfg.OutputHeight}
	canvas := image.NewRGBA(image.Rectangle{Min: upLeft, Max: lowRight})

	root := &Node{
		rc: Rectangle{
			Left:   0,
			Top:    0,
			Right:  cfg.OutputWidth,
			Bottom: cfg.OutputHeight,
		},
	}

	return &Packer{
		cfg:    cfg,
		root:   root,
		canvas: canvas,
	}
}

func (p *Packer) Pack(images map[string]string) (err error) {
	if len(images) == 0 {
		return errors.New("empty images")
	}

	sortedPath := make([]string, 0, len(images))
	for k := range images {
		sortedPath = append(sortedPath, k)
	}
	sort.Strings(sortedPath)

	for _, absPath := range sortedPath {
		logrus.Debugf("packing %v", absPath)

		// read image file
		imgInfo := NewImageInfoParseFrom(absPath, images[absPath], p.cfg.Padding)
		if imgInfo == nil {
			continue
		}

		err = p.insert(imgInfo)
		if err != nil {
			if !p.cfg.IgnoreLargeImage {
				return
			}
			logrus.Debug("ignore oversize image:", imgInfo.absolutePath)
		}
	}

	var outputFile *os.File
	outputFile, err = os.Create(p.cfg.OutputImagePath)
	if err != nil {
		return
	}
	err = png.Encode(outputFile, p.canvas)
	if err != nil {
		return
	}
	err = outputFile.Close()
	return
}

func (p *Packer) insert(img *ImageInfo) error {
	node := p.root.insert(img)
	if node != nil {
		// copy pixels
		img.CopyToImage(p.canvas, node.rc)

		// TODO: atlas output
		logrus.Info(img.String())
	}
	return nil
}
