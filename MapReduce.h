#include <string>
#include <unordered_map>
#include <vector>

namespace MapReduce {
using WordsBucket = std::unordered_map<std::string, size_t>;
using FilesWordsBucket = std::unordered_map<std::string, WordsBucket>;
using Words = std::vector<std::string>;

Words getFileNamesInsideDirectory(const std::string pathToDirectory);

WordsBucket countWordAppearences(const std::string &fileName);
void writeResult(const std::string folderToWriteIn,
                 const FilesWordsBucket &frequencyOfApparitionInFiles);
FilesWordsBucket
countWordAppearencesFromFiles(const std::vector<std::string> &fileNames);
void removeFolder(const std::string &folder);
std::vector<std::vector<std::string>>
splitVector(const std::vector<std::string> &vectorToBeSplited,
            const size_t numberOfParts);
std::unordered_map<std::string, std::unordered_map<std::string, size_t>>
reduce(const std::string &folderPath,
       const std::vector<std::string> &wordsToCount);
void writeResultReduce(const std::string &folderToWriteIn,
                       const FilesWordsBucket &filesWordsBucket);

void printMap(const std::unordered_map<std::string, size_t> &myMap);
Words getFileNamesInsideDirectoryLvl2(const std::string &path);
} // namespace MapReduce
