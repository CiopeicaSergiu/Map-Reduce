#include "MapReduce.h"
#include "mpi.h"
#include <algorithm>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <thread>

using namespace std::chrono_literals;
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
  int nrProcessesToSpawn;

  if (parentcomm == MPI_COMM_NULL) {
    nrProcessesToSpawn = std::stoi(getProcessNrToSpawn(argv));

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

    MPI_Comm_spawn(argv[0], argv, 2 * nrProcessesToSpawn, MPI_INFO_NULL, 0,
                   MPI_COMM_WORLD, &intercomm, errcodes);

    printf("I'm the parent: %d\n", id);

    for (int i = 0; i < nrProcessesToSpawn; ++i) {
      sendFromMasterToWorkers(i, vecOfVecs[i]);
    }
  } else if (id < count / 2) {
    printf("I'm the spawned: %d\n", id);
    const auto filesToHandle = receiveArrayOfString();
    const auto wordsCountedFromFiles =
        countWordAppearencesFromFiles(filesToHandle);
    writeResult(folderToWriteIn + "P" + std::to_string(id),
                wordsCountedFromFiles);
  }

  if (parentcomm == MPI_COMM_NULL) {

    std::vector<std::string> wordsToCount;
    while (wordsToCount.size() < nrProcessesToSpawn) {
      wordsToCount = getFileNamesInsideDirectory(folderToWriteIn);
    }
    wordsToCount = getFileNamesInsideDirectoryLvl2(folderToWriteIn);

    const auto splitedWordsToCount =
        splitVector(wordsToCount, nrProcessesToSpawn);

    std::cout << "Splited words to count: " << splitedWordsToCount.size()
              << std::endl;
    for (int i = 0; i < splitedWordsToCount.size(); ++i) {

      sendFromMasterToWorkers(2 * splitedWordsToCount.size() - i - 1,
                              splitedWordsToCount[i]);
      std::this_thread::sleep_for(10ms);
    }

  } else {
    MPI_Barrier(MPI_COMM_WORLD);

    if (id >= count / 2) {
      std::cout << "I am worker: " << id << std::endl;
      std::unordered_map<std::string, std::unordered_map<std::string, size_t>>
          reduceResult;

      const auto wordsToCount = receiveArrayOfString();

      // std::this_thread::sleep_for(10ms);
      if (id == 8) {
        printVec(wordsToCount);
      }

      reduceResult = reduce(folderToWriteIn, wordsToCount);

      writeResultReduce(resultFolder, reduceResult);
    }
  }

  MPI_Finalize();
  return 0;
}
