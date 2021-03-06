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
	"crypto/hmac"
	"crypto/sha512"
	"encoding/json"
	"fmt"
	"image"
	"image/jpeg"
	"image/png"
	"os"
	"path/filepath"
	"strings"

	"github.com/master-g/texturepacker/pkg/base58"
	"github.com/sirupsen/logrus"
	"golang.org/x/image/bmp"
	"golang.org/x/image/webp"
)

// ImageInfo contains image file info and image properties
type ImageInfo struct {
	ID     string `json:"id"`
	Width  int    `json:"width"`
	Height int    `json:"height"`
	Left   int    `json:"x"`
	Top    int    `json:"y"`
	Name   string `json:"name"`

	absolutePath string
	imgType      string
	padding      int
}

// NewImageInfoParseFrom parse image info from input file
func NewImageInfoParseFrom(imagePath, name string, padding int) *ImageInfo {
	var err error
	var imgFile *os.File
	imgFile, err = os.Open(imagePath)
	if err != nil {
		logrus.Warnf("cannot access image file: %v, err: %v", imagePath, err)
		return nil
	}
	defer func() {
		err := imgFile.Close()
		if err != nil {
			logrus.Warnf("cannot close file: %v, err: %v", imagePath, err)
		}
	}()

	// try standard decode
	var imgData image.Image
	var imgType string
	ext := strings.ToLower(filepath.Ext(imagePath))
	if strings.HasSuffix(ext, "jpg") || strings.HasSuffix(ext, "jpeg") {
		imgData, err = jpeg.Decode(imgFile)
		imgType = "jpg"
	} else if strings.HasSuffix(ext, "png") {
		imgData, err = png.Decode(imgFile)
		imgType = "png"
	} else if strings.HasSuffix(ext, "bmp") {
		imgData, err = bmp.Decode(imgFile)
		imgType = "bmp"
	} else if strings.HasSuffix(ext, "webp") {
		imgData, err = webp.Decode(imgFile)
		imgType = "webp"
	}
	if imgType == "" {
		logrus.Errorf("unsupported format: %v", ext)
		return nil
	}
	if err != nil {
		logrus.Errorf("unable to decode image: %v, err: %v", imagePath, err)
		return nil
	}
	if imgData == nil {
		logrus.Errorf("unknown error while decoding image: %v", imagePath)
		return nil
	}

	// result
	imageInfo := &ImageInfo{
		absolutePath: imagePath,
		Name:         name,
		Width:        imgData.Bounds().Dx(),
		Height:       imgData.Bounds().Dy(),
		imgType:      imgType,
		padding:      padding,
	}

	// generate id
	h := hmac.New(sha512.New512_256, []byte("0x12F0E6D"))
	_, err = h.Write([]byte(imagePath))
	if err != nil {
		logrus.Fatalf("error while calculating hash, err:%v", err)
	}
	imageInfo.ID = base58.Encode(h.Sum(nil)[:8])

	return imageInfo
}

// PaddedWidth returns image width with padding
func (img *ImageInfo) PaddedWidth() int {
	return img.Width + img.padding*2
}

// PaddedHeight returns image height with padding
func (img *ImageInfo) PaddedHeight() int {
	return img.Height + img.padding*2
}

// CopyToImage copies a region of img to the canvas
func (img *ImageInfo) CopyToImage(canvas *image.RGBA, rc Rectangle) {
	// update position
	img.Left = rc.Left + img.padding
	img.Top = rc.Top + img.padding

	// copy to canvas
	imgFile, err := os.Open(img.absolutePath)
	if err != nil {
		logrus.Warnf("cannot open file: %v, err:%v", img.absolutePath, err)
		return
	}
	defer func() {
		err = imgFile.Close()
		if err != nil {
			logrus.Warnf("cannot close file: %v, err: %v", img.absolutePath, err)
		}
	}()

	var imgData image.Image
	switch img.imgType {
	case "png":
		imgData, err = png.Decode(imgFile)
	case "jpg", "jpeg":
		imgData, err = jpeg.Decode(imgFile)
	case "bmp":
		imgData, err = bmp.Decode(imgFile)
	case "webp":
		imgData, err = webp.Decode(imgFile)
	}

	if imgData == nil {
		logrus.Warnf("cannot decode image: %v, unsupported format: %v", img.absolutePath, img.imgType)
		return
	}
	if err != nil {
		logrus.Warnf("cannot decode image: %v, err: %v", img.absolutePath, err)
		return
	}

	for y := 0; y < img.Height; y++ {
		for x := 0; x < img.Width; x++ {
			color := imgData.At(x, y)
			canvas.Set(rc.Left+img.padding+x, rc.Top+img.padding+y, color)
		}
	}
}

func (img *ImageInfo) String() string {
	b, err := json.Marshal(img)
	if err != nil {
		return fmt.Sprintf("ImageInfo{err=%v}", err)
	}

	return string(b)
}
