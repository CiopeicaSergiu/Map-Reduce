#include "MapReduce.h"
#include "mpi.h"
#include <algorithm>
#include <cstring>
#include <filesystem>
#include <iostream>

using namespace MapReduce;
namespace fs = std::filesystem;

void printVec(const std::vector<std::string> &vec) {
  std::cout << "Vec: ";
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

Words getWordsToCount(const char *const *words, const int nrOfWords) {
  Words wordsToCount;
  if (nrOfWords < 3) {
    return {};
  }

  for (int i = 4; i < nrOfWords; ++i) {
    wordsToCount.emplace_back(words[i]);
  }

  return wordsToCount;
}

std::string getProcessNrToSpawn(const char *const *argv) { return argv[1]; }

std::string getDataFolder(const char *const *argv) { return argv[2]; }

int main(int argc, char *argv[]) {

  MPI_Comm parentcomm, intercomm;

  MPI_Init(&argc, &argv);
  MPI_Comm_get_parent(&parentcomm);

  int count, id;
  MPI_Status status;

  MPI_Comm_size(MPI_COMM_WORLD, &count);
  MPI_Comm_rank(MPI_COMM_WORLD, &id);
  auto sendFromMasterToWorkers =
      [&](const int idReceiver, const std::vector<std::string> &arrayToSend) {
        int numberOfMessages = arrayToSend.size();
        MPI_Send(&numberOfMessages, 1, MPI_INT, idReceiver, 99, intercomm);
        for (const auto &message : arrayToSend) {
          MPI_Send(message.c_str(), message.size() + 1, MPI_CHAR, idReceiver,
                   99, intercomm);
        }
      };

  auto receiveArrayOfString = [&]() {
    int numberOfMessages;
    MPI_Recv(&numberOfMessages, 1, MPI_INT, 0, 99, parentcomm, &status);
    std::vector<std::string> messages;
    for (size_t i = 0; i < numberOfMessages; ++i) {
      char buffer[50];
      MPI_Recv(buffer, 50, MPI_CHAR, 0, 99, parentcomm, &status);
      messages.emplace_back(buffer);
    }
    return messages;
  };

  if (argc <= 2) {
    std::cerr << "No word was selected for counting!\n";
    return -1;
  }

  const std::string folderToWriteIn{"../../intermediary/"};
  const std::string resultFolder{"../../result"};

  if (parentcomm == MPI_COMM_NULL) {
    int nrProcessesToSpawn = std::stoi(getProcessNrToSpawn(argv));

    int errcodes[nrProcessesToSpawn];

    WordsBucket wordsBucket;

    auto fileNames =
        getFileNamesInsideDirectory("../../" + getDataFolder(argv));
    const auto vecOfVecs = splitVector(fileNames, nrProcessesToSpawn);

    if (vecOfVecs.size() < nrProcessesToSpawn) {
      nrProcessesToSpawn = vecOfVecs.size();
    }

    removeFolder(folderToWriteIn);
    fs::create_directory(folderToWriteIn);

    removeFolder(resultFolder);
    fs::create_directory(resultFolder);

    MPI_Comm_spawn(argv[0], argv, nrProcessesToSpawn, MPI_INFO_NULL, 0,
                   MPI_COMM_WORLD, &intercomm, errcodes);

    printf("I'm the parent: %d\n", id);

    for (int i = 0; i < nrProcessesToSpawn; ++i) {
      sendFromMasterToWorkers(i, vecOfVecs[i]);
    }
  } else {

    printf("I'm the spawned: %d\n", id);
    const auto filesToHandle = receiveArrayOfString();
    const auto wordsCountedFromFiles =
        countWordAppearencesFromFiles(filesToHandle);
    writeResult(folderToWriteIn + "P" + std::to_string(id),
                wordsCountedFromFiles);
  }

  if (parentcomm != MPI_COMM_NULL) {
    MPI_Barrier(MPI_COMM_WORLD);
    Words wordsToCount = getWordsToCount(argv, argc);
    printVec(wordsToCount);
    const auto splitedWordsToCount = splitVector(wordsToCount, count);

    std::unordered_map<std::string, std::unordered_map<std::string, size_t>>
        reduceResult;

    if (id < splitedWordsToCount.size()) {
      reduceResult = reduce(folderToWriteIn, splitedWordsToCount[id]);

      writeResultReduce(resultFolder, reduceResult);
    }
  }

  MPI_Finalize();
  return 0;
}
