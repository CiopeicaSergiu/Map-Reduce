#include <algorithm>
#include <filesystem>
#include <fstream>
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

void mapWordsFromFile(const std::string &fileName) {
  std::ifstream file(fileName);

  if (not file.is_open()) {
    std::errc << "Eroare la deschiderea fisierului: " << fileName << "\n";
    return;
  }

  std::string lineFromFile;
  while (std::getline(file, lineFromFile)) {
    std::vector<std::string> result;

    boost::trim(line);
    boost::split(result, line, boost::is_space());
  }
}

int main() {
  const auto filesInDirectory = getFileNamesInsideDirectory("../data");
  std::for_each(filesInDirectory.cbegin(), filesInDirectory.cend(),
                [](const auto &fileName) { std::cout << fileName << "\n"; });
  return 0;
}