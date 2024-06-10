#include "PipelinedCPU.h"

int main(int argc, char **argv) {
  if (argc != 6) {
    fprintf(stderr, "[%s] Usage: %s initialPC regFileName instMemFileName dataMemFileName numCycles\n", __func__, argv[0]);
    fflush(stdout);
  }

  const std::int32_t initialPC = (std::int32_t)(atoll(argv[1])) - 4;
  const char *regFileName = argv[2];
  const char *instMemFileName = argv[3];
  const char *dataMemFileName = argv[4];
  const std::uint64_t numCycles = (std::uint64_t)atoll(argv[5]);

  PipelinedCPU *cpu = new PipelinedCPU(
    "PipelinedCPU",
    initialPC,
    Memory::LittleEndian,
    regFileName, instMemFileName, dataMemFileName);

  cpu->printPVS();
  for (size_t i = 0; i < numCycles; i++) {
    cpu->advanceCycle();
    cpu->printPVS();
  }

  delete cpu;

  return 0;
}

