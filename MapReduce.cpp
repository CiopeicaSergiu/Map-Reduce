
#include "MapReduce.h"
#include <algorithm>
#include <boost/algorithm/string.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>

namespace MapReduce {
namespace fs = std::filesystem;

Words getFileNamesInsideDirectory(const std::string pathToDirectory) {

  Words filesInDirectory;

  for (const auto &entry : fs::directory_iterator(pathToDirectory)) {
    filesInDirectory.emplace_back(entry.path().string());
  }

  return filesInDirectory;
}

WordsBucket countWordAppearences(const std::string &fileName) {

  WordsBucket wordsFrequencies;

  std::ifstream file(fileName);
  if (not file.is_open()) {
    std::cerr << "Eroare la deschiderea fisierului: " << fileName << "\n";
    return WordsBucket{};
  }

  std::string lineFromFile;

  while (std::getline(file, lineFromFile)) {

    boost::trim(lineFromFile);

    Words words;
    boost::split(words, lineFromFile, boost::is_space());

    for (const auto &word : words) {

      const auto wordsFrequenciesIt = wordsFrequencies.find(word);

      if (wordsFrequenciesIt == wordsFrequencies.end()) {
        wordsFrequencies.insert({word, 0});

      } else {
        ++wordsFrequenciesIt->second;
      }
    }
  }

  file.close();

  return wordsFrequencies;
}

FilesWordsBucket
countWordAppearencesFromFiles(const std::vector<std::string> &fileNames) {
  FilesWordsBucket filesWordsBucket;

  for (const auto &fileName : fileNames) {
    std::string fileNameCopy(fileName);

    filesWordsBucket.insert(
        {fileNameCopy.erase(0, fileNameCopy.find_last_of('/') + 1)
             .erase(fileNameCopy.find(".txt"), fileName.size() - 1),
         std::move(countWordAppearences(fileName))});
  }

  return filesWordsBucket;
}

std::string repeatString(const std::string &stringToRepeat,
                         size_t timesToRepeat) {
  std::ostringstream buffer;
  buffer << stringToRepeat << ",";
  for (size_t i = 0; i < timesToRepeat; ++i) {
    buffer << stringToRepeat << ",";
  }
  return buffer.str();
}

void writeResult(const std::string folderToWriteIn,
                 const FilesWordsBucket &filesWordsBucket) {

  fs::create_directory(folderToWriteIn);

  for (const auto &[fileName, wordsBucket] : filesWordsBucket) {
    for (const auto &[word, nrOfAppereances] : wordsBucket) {

      std::ofstream outFile(folderToWriteIn + "/" + word + ".txt",
                            std::ios_base::app);
      outFile << repeatString(fileName, nrOfAppereances);
      outFile.close();
    }
  }
}

void removeFolder(const std::string &folder) {
  if (const fs::path pathToSearchFor{folder}; fs::exists(pathToSearchFor)) {
    fs::remove_all(folder);
  }
}

std::vector<std::vector<std::string>>
splitVector(const std::vector<std::string> &vectorToBeSplited,
            const size_t numberOfParts) {

  const size_t sizeOfPart = vectorToBeSplited.size() / numberOfParts;
  const size_t rest = vectorToBeSplited.size() % numberOfParts;

  std::vector<std::vector<std::string>> splitedWorkload;
  splitedWorkload.reserve(numberOfParts);

  auto vectorToBeSplitedIt = vectorToBeSplited.begin();

  for (int i = 0; i < numberOfParts - 1; ++i) {
    std::vector<std::string> partition;
    partition.reserve(sizeOfPart);
    splitedWorkload.push_back(std::move(partition));

    std::copy_n(vectorToBeSplitedIt, sizeOfPart,
                std::back_inserter(splitedWorkload[i]));
    vectorToBeSplitedIt = std::next(vectorToBeSplitedIt, sizeOfPart);
  }

  std::vector<std::string> partition;
  partition.reserve(sizeOfPart);
  splitedWorkload.push_back(std::move(partition));

  std::copy_n(vectorToBeSplitedIt, rest ? sizeOfPart + rest : sizeOfPart,
              std::back_inserter(splitedWorkload[numberOfParts - 1]));

  return splitedWorkload;
}

std::vector<std::string> readFromFile(const std::string &fileToReadFrom) {
  std::ifstream file(fileToReadFrom);

  if (not file.is_open()) {
    std::cerr << "Eroare la deschiderea fisierului: " << fileToReadFrom << "\n";
    return {};
  }

  std::string line;
  std::vector<std::string> text;
  while (std::getline(file, line)) {

    text.push_back(std::move(line));
  }

  return text;
}

void printVec(const std::vector<std::string> &vec) {
  for (const auto &element : vec) {
    std::cout << element << ", ";
  }
  std::cout << std::endl;
}

void printVecOfVecs(const std::vector<std::vector<std::string>> &vecOfVecs) {
  for (const auto &vec : vecOfVecs) {
    printVec(vec);
  }
}

std::unordered_map<std::string, std::unordered_map<std::string, size_t>>
reduce(const std::string &folderPath,
       const std::vector<std::string> &wordsToCount) {
  const auto fileNamesLvl1 = getFileNamesInsideDirectory(folderPath);

  std::unordered_map<std::string, std::unordered_map<std::string, size_t>>
      countedWordsFromFile;

  for (const auto &fileInLvl1 : fileNamesLvl1) {
    const auto fileNamesLvl2 = getFileNamesInsideDirectory(fileInLvl1);

    for (const auto &fileInLvl2 : fileNamesLvl2) {

      std::string fileInLvl2Copy(fileInLvl2);
      fileInLvl2Copy.erase(0, fileInLvl2Copy.find_last_of('/') + 1);
      fileInLvl2Copy.erase(fileInLvl2Copy.find(".txt"), fileInLvl2Copy.size());
      if (wordsToCount.cend() == std::find(wordsToCount.cbegin(),
                                           wordsToCount.cend(),
                                           fileInLvl2Copy)) {
        continue;
      }

      if (countedWordsFromFile.cend() ==
          countedWordsFromFile.find(fileInLvl2Copy)) {
        countedWordsFromFile.insert({fileInLvl2Copy, {}});
      }

      auto text = readFromFile(fileInLvl2);
      std::for_each(text.begin(), text.end(), [&](auto &lineOfFile) {
        boost::trim_if(lineOfFile, boost::is_any_of(","));
        Words wordsFromFile;
        boost::split(wordsFromFile, lineOfFile, boost::is_any_of(","));
        std::for_each(wordsFromFile.begin(), wordsFromFile.end(),
                      [&](const auto &word) {
                        const auto countedWordsIt =
                            countedWordsFromFile[fileInLvl2Copy].find(word);

                        if (countedWordsIt ==
                            countedWordsFromFile[fileInLvl2Copy].cend()) {
                          countedWordsFromFile[fileInLvl2Copy].insert(
                              {std::move(word), 1});
                        } else {
                          ++countedWordsIt->second;
                        }
                      });
      });
    }
  }
  return countedWordsFromFile;
}

void writeResultReduce(const std::string &folderToWriteIn,
                       const FilesWordsBucket &filesWordsBucket) {
  fs::create_directory(folderToWriteIn);

  for (const auto &[word, wordsBucket] : filesWordsBucket) {
    std::ofstream outFile(folderToWriteIn + "/" + word + ".txt",
                          std::ios_base::app);

    for (const auto &[fileName, nrOfAppereances] : wordsBucket) {

      outFile << '(' << fileName << ',' << nrOfAppereances << ')' << ", ";
    }
    outFile.close();
  }
}

void printMap(const std::unordered_map<std::string, size_t> &myMap) {
  std::cout << "MyMap: ";
  for (const auto &[key, value] : myMap) {
    std::cout << "(" << key << "," << value << ");";
  }
  std::cout << std::endl;
}

Words getFileNamesInsideDirectoryLvl2(const std::string &path) {
  Words result;
  const auto subfolders = getFileNamesInsideDirectory(path);
  for (const auto &subfolder : subfolders) {
    auto fileNames = getFileNamesInsideDirectory(subfolder);
    std::transform(fileNames.begin(), fileNames.end(), fileNames.begin(),
                   [](const auto &file) {
                     const auto auxFile =
                         file.substr(file.find_last_of("/") + 1);
                     return auxFile.substr(0, auxFile.find(".txt"));
                   });
    std::copy(fileNames.cbegin(), fileNames.cend(), std::back_inserter(result));
  }

  return result;
}

} // namespace MapReduce
