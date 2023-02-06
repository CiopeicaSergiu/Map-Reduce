#include <filesystem>
#include <iostream>
#include <string>
#include <vector>
namespace fs = std::filesystem;

std::vector<std::string>
getFileNamesInsideDirectory(const std::string pathToDirectory) {

  std::vector<std::string> filesInDirectory;

  for (const auto &entry : fs::directory_iterator(pathToDirectory)) {
    filesInDirectory.emplace_back(entry.path().string());
  }

  return filesInDirectory;
}

void printVec(const std::vector<std::string> &vec) {
  std::cout << "Vec: ";
  for (const auto &element : vec) {
    std::cout << element << ", ";
  }
  std::cout << std::endl;
}

int main(int argc, char **argv) {
  std::string filePath{"intermediary/"};
  const auto fileNames = getFileNamesInsideDirectory(filePath);
  printVec(fileNames);

  return 0;
}