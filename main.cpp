#include <iostream>
#include <memory>
#include <limits>
#include <cmath>
#include <mpi.h>

#include "Timer.hpp"

static void P2pBenchmark(const std::size_t loop)
{
	int n;
	MPI_Comm_size(MPI_COMM_WORLD, &n);

	int me;
	MPI_Comm_rank(MPI_COMM_WORLD, &me);

	using TYPE = std::uint64_t;
	constexpr auto PROBLEM_SIZE = std::numeric_limits<int>::max()/sizeof(TYPE); // MPI's count of send/recv = int
	auto buffer = std::make_unique<std::uint64_t[]>(PROBLEM_SIZE);

	for(auto problemSize = decltype(PROBLEM_SIZE)(1); problemSize < PROBLEM_SIZE; problemSize *= 2)
	{
		const auto bufferCount = problemSize - 1;

		for(auto src = decltype(n)(0); src < n; ++src)
		{
			for(auto dst = decltype(n)(0); dst < n; ++dst)
			{
				if(dst != src)
				{
					constexpr auto TAG = int(0);

					MPI_Barrier(MPI_COMM_WORLD); // wait all for benchmarking
					if(me == src)
					{
						LWisteria::Timer timer;

						MPI_Send(buffer.get(), sizeof(TYPE) * bufferCount, MPI_UINT8_T, dst, TAG, MPI_COMM_WORLD);

						const auto t = timer.Time();

						std::cout << loop << ", " << std::log2(problemSize) << ", " << src << ", " << dst << ", " << t << std::endl;
					}
					else if(me == dst)
					{
						MPI_Status status;
						MPI_Recv(buffer.get(), sizeof(TYPE) * bufferCount, MPI_UINT8_T, src, TAG, MPI_COMM_WORLD, &status);
					}
				}
			}
		}
	}
}

int main(int argc, char* argv[])
{
	MPI_Init(&argc, &argv);

	// preprocess
	{
		int me;
		MPI_Comm_rank(MPI_COMM_WORLD, &me);

		{
			char name[MPI_MAX_PROCESSOR_NAME];
			int length;
			MPI_Get_processor_name(name, &length);

			std::cout << me << ", " << name << std::endl;
		}

		MPI_Barrier(MPI_COMM_WORLD);
		if(me == 0)
		{
			// csv header
			std::cout << "loop, problemSize, sender rank, recevier rank, time [ms]" << std::endl;
		}
	}

	for(auto i = std::size_t(0); true; ++i)
	{
		P2pBenchmark(i);
	}

	MPI_Finalize();
	return 0;
}
