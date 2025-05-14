/// TODO:
/// - normalizing method for accuracy maybe hack mode temporarily like go code

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize.h"

const std::string asciiChars = "#@%*+=-:. ";

std::vector<uint8_t> loadImage(
  const std::string& filename,
  int& width,
  int& height,
  int& channels
) {
  uint8_t* data = stbi_load(filename.c_str(), &width, &height, &channels, 3);
  if (!data) {
    throw std::runtime_error("Failed to load image.");
  }

  std::vector<uint8_t> imageData(data, data + (width * height * 3));
  stbi_image_free(data);

  return imageData;
}

std::string convertToASCII(
  const std::vector<uint8_t>& img,
  int width,
  int height,
  int imgWidth,
  const std::string& mode
) {
  std::string asciiArt;

  for (int y = 0; y < height; ++y) {
    if (mode == "-h" && y % 2 == 1) {
      continue;
    }
  
    for (int x = 0; x < width; ++x) {
      int idx = (y * imgWidth + x) * 3;
      uint8_t r = img[idx];
      uint8_t g = img[idx + 1];
      uint8_t b = img[idx + 2];
      int gray = (r + g + b) / 3;
      size_t charIndex = gray * (asciiChars.size() - 1) / 255;
      asciiArt += asciiChars[charIndex];
    }
    asciiArt += '\n';
  }
  return asciiArt;
}

void writeToFile(const std::string& output) {
  std::ofstream outFile("output.txt");
  if (!outFile) {
    throw std::runtime_error("Failed to open output file.");
  }
  outFile << output;
}

std::pair<int, int> calculateResize(
  int originalWidth,
  int originalHeight,
  int maxWidth,
  int maxHeight,
  bool allowUpscale = false  // not completely sure.
) {
  if (
    originalWidth <= 0 ||
    originalHeight <= 0 ||
    maxWidth <= 0 ||
    maxHeight <= 0
  ) {
    return {0, 0};
  }

  double aspectRatio = static_cast<double>(originalWidth) / originalHeight;
  double targetWidth = static_cast<double>(maxWidth);
  double targetHeight = targetWidth / aspectRatio;

  if (targetHeight > maxHeight) {
    targetHeight = static_cast<double>(maxHeight);
    targetWidth = targetHeight * aspectRatio;
  }

  if (!allowUpscale) {
    targetWidth = std::min(targetWidth, static_cast<double>(originalWidth));
    targetHeight = std::min(targetHeight, static_cast<double>(originalHeight));
  }

  return {
    static_cast<int>(std::round(targetWidth)),
    static_cast<int>(std::round(targetHeight))
  };
}

int main(int argc, char* argv[]) {
  if (argc < 2) {
    std::cerr << "provide an image path.\n";
    return 1;
  }

  std::string imagePath = argv[1];
  std::string mode = (argc > 2) ? argv[2] : "";

  try {
    int origWidth, origHeight, channels;
    auto imageData = loadImage(imagePath, origWidth, origHeight, channels);

    const int maxWidth = 200;
    const int maxHeight = 200;
    auto [resizedWidth, resizedHeight] =
      calculateResize(origWidth, origHeight, maxWidth, maxHeight);

    std::vector<uint8_t> resizedData(resizedWidth * resizedHeight * 3);
    int success =
      stbir_resize_uint8(imageData.data(), origWidth, origHeight, 0,
        resizedData.data(), resizedWidth, resizedHeight, 0, 3);

    if (!success) {
      throw std::runtime_error("Failed to resize image.");
    }

    std::string asciiArt =
      convertToASCII(resizedData, resizedWidth, resizedHeight, resizedWidth, mode);

    writeToFile(asciiArt);
    std::cout << "ASCII art in output.txt\n";
  } catch (const std::exception& ex) {
    std::cerr << ex.what() << "\n";
    return 1;
  }

  return 0;
}

