package main

import (
	"fmt"
	"image"
	_ "image/jpeg"
	_ "image/png"
	"io/ioutil"
	"log"
	"os"

	"github.com/nfnt/resize"
)

// pixel brightness level
const asciiChars = " .:-=+*%@#"

func convertToASCII(img image.Image, width, height int, mode string) string {
	bounds := img.Bounds() // top left and bottom right of image
	origWidth, origHeight := bounds.Dx(), bounds.Dy() // width & height

	aspectRatio := float64(origWidth) / float64(origHeight)
	resizedWidth := width
	resizedHeight := int(float64(width) / aspectRatio)

	if resizedHeight > height {
		resizedHeight = height
		resizedWidth = int(float64(height) * aspectRatio)
	}

	var asciiArt string

	// on hack: skip every other line to keep "proper" image ration due to
	// character height distortion
	for y := 0; y < resizedHeight; y++ {
		if (mode == "-h" || mode == "hack" || mode == "--mode=hack") && y%2 == 1 {
			continue
		}
		// iterate each brightness level to map respective char
		for x := 0; x < resizedWidth; x++ {
			r, g, b, _ := img.At(x, y).RGBA()
			grayValue := (r + g + b) / 3 >> 8

			index := int(grayValue * uint32(len(asciiChars)-1) / 255)
			asciiArt += string(asciiChars[index])
		}
		asciiArt += "\n"
	}

	return asciiArt
}

func loadImage(filename string) (image.Image, error) {
	file, err := os.Open(filename)
	if err != nil {
		return nil, fmt.Errorf("error opening image file: %v", err)
	}
	defer file.Close()

	// decode means mapping pixel data to string representing colour values
	img, _, err := image.Decode(file)
	if err != nil {
		return nil, fmt.Errorf("error decoding image file: %v", err)
	}
	return img, nil
}

func resizeImage(img image.Image, maxWidth, maxHeight uint) image.Image {
	bounds := img.Bounds()
	origWidth, origHeight := bounds.Dx(), bounds.Dy()

	// resizing to fit within max bounds while preserving aspect ratio.
	var newWidth, newHeight uint
	if origWidth > origHeight {	// landscape image
		newWidth = maxWidth
		newHeight = uint(origHeight) * maxWidth / uint(origWidth)
		if newHeight > maxHeight {
			newHeight = maxHeight
			newWidth = uint(origWidth) * maxHeight / uint(origHeight)
		}
	} else { // portrait
		newHeight = maxHeight
		newWidth = uint(origWidth) * maxHeight / uint(origHeight)
		if newWidth > maxWidth {
			newWidth = maxWidth
			newHeight = uint(origHeight) * maxWidth / uint(origWidth)
		}
	}

	// lanczos is good algo for retaining info while downscaling
	// see: <https://blog.idrsolutions.com/high-quality-image-downscaling-in-java-lanczos3/>
	return resize.Resize(newWidth, newHeight, img, resize.Lanczos3)
}

func writeToFile(asciiArt string) error {
	err := ioutil.WriteFile("output.txt", []byte(asciiArt), 0644)
	if err != nil {
		return fmt.Errorf("error writing to file: %v", err)
	}

	return nil
}

func main() {
	if len(os.Args) < 2 {
		log.Fatal("provide image's path as argument")
	}
	imagePath := os.Args[1]

	flag := ""
	if len(os.Args) > 2 {
		flag = os.Args[2]
	}

	img, err := loadImage(imagePath)
	if err != nil {
		log.Fatalf("error loading image: %v", err)
	}

	maxWidth, maxHeight := 200, 200
	resizedImg := resizeImage(img, uint(maxWidth), uint(maxHeight))

	asciiArt := convertToASCII(resizedImg, maxWidth, maxHeight, flag)

	if err := writeToFile(asciiArt); err != nil {
		log.Fatalf("error writing ASCII art to file: %v", err)
	}

	log.Println("ASCII art has been written to output.txt!")
}
