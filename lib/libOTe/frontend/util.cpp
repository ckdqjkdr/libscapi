#include "util.h"

using namespace osuCrypto;
#include <cryptoTools/Common/Log.h>
#include <cryptoTools/Common/Timer.h>
#include <chrono>
#include <thread>
#define tryCount 2


void sync(Channel& chl, Role role)
{
	if (role == Role::Receiver)
	{

		u8 dummy[1];
		chl.recv(dummy, 1);
		Timer timer;


		auto start = timer.setTimePoint("");
		chl.asyncSend(dummy, 1);
		chl.recv(dummy, 1);
		auto mid = timer.setTimePoint("");

		chl.asyncSend(dummy, 1);


		auto rrt = mid - start;
		auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(rrt).count();
		if (ms > 4)
			std::this_thread::sleep_for(rrt / 2);

	}
	else
	{
		u8 dummy[1];
		chl.asyncSend(dummy, 1);
		chl.recv(dummy, 1);
		chl.asyncSend(dummy, 1);
		chl.recv(dummy, 1);
	}
}

void senderGetLatency(Channel& chl)
{

    u8 dummy[1];

    chl.asyncSend(dummy, 1);



    chl.recv(dummy, 1);
    chl.asyncSend(dummy, 1);


    std::vector<u8> oneMbit((1 << 20) / 8);
    for (u64 i = 0; i < tryCount; ++i)
    {
        chl.recv(dummy, 1);

        for(u64 j =0; j < (1<<10); ++j)
            chl.asyncSend(oneMbit.data(), oneMbit.size());
    }
    chl.recv(dummy, 1);

}

void recverGetLatency(Channel& chl)
{

    u8 dummy[1];
    chl.recv(dummy, 1);
    Timer timer;
    auto start = timer.setTimePoint("");
    chl.asyncSend(dummy, 1);


    chl.recv(dummy, 1);

    auto mid = timer.setTimePoint("");
    auto recvStart = mid;
    auto recvEnd = mid;

    auto rrt = mid - start;
    std::cout << "latency:   " << std::chrono::duration_cast<std::chrono::milliseconds>(rrt).count() << " ms" << std::endl;
                 
	std::vector<u8> oneMbit((1 << 20) / 8);
    for (u64 i = 0; i < tryCount; ++i)
    {
        recvStart = timer.setTimePoint("");
        chl.asyncSend(dummy, 1);

        for (u64 j = 0; j < (1 << 10); ++j)
            chl.recv(oneMbit);

        recvEnd = timer.setTimePoint("");

        // nanoseconds per GegaBit
        auto uspGb = std::chrono::duration_cast<std::chrono::nanoseconds>(recvEnd - recvStart - rrt / 2).count();

        // nanoseconds per second
        auto usps = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::seconds(1)).count();

        // MegaBits per second
        auto Mbps = usps * 1.0 / uspGb *  (1 << 10);

        std::cout << "bandwidth: " << Mbps << " Mbps" << std::endl;
    }

    chl.asyncSend(dummy, 1);

}
