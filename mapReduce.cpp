// #include <iostream>
// #include <mpi.h>

// const std::string wordToMap = "ana";

// void map(const std::string filePath) {
//   std::ifstream file(filePath, ios::in);

//   if (not file.is_open()) {
//     return;
//   }

//   std::string bufferLine;

//   while (std::getline(file, bufferLine)) {
//     // use boost
//   }
// }

// int main(int argc, char **argv) {
//   int count, id;
//   MPI_Init(&argc, &argv);
//   MPI_Comm_size(MPI_COMM_WORLD, &count);
//   MPI_Comm_rank(MPI_COMM_WORLD, &id);

//   MPI_Status status;

//   if (not id) {
//     std::cout << "HelloWorld!\n";
//   } else {
//     std::cout << "Hello back!\n";
//   }

//   MPI_Finalize();
//   return 0;
// }

#include <iostream>

int main() {
  std::cout << "Hello World!!" << std::endl;
  return 0;
}