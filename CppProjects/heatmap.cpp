#include <iostream>
#include <boost/dynamic_bitset.hpp>
#include <thread>
#include <chrono>
#include <random>
#include <queue>
#include <mutex>

using namespace std;

const int len = 100;
std::mutex bitmapMutex;
boost::dynamic_bitset<> bitmap(len);

struct MetaData {
    static int arr[len];   
};

int MetaData::arr[len] = {0};

// const int usablelen = endLBA - beginLBA;


int allocationIndex = 0;
int deallocationIndex = 0;

void allocateBlocksBefore() {
    while (true)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));

        {
            std::lock_guard<std::mutex> lock(bitmapMutex);
            int blocksToWrite = 15;
            allocationIndex %= len;
            int freeBlocks = deallocationIndex - allocationIndex;
            if(freeBlocks <= 0) freeBlocks += len;
            if(freeBlocks < blocksToWrite) continue;
            if (allocationIndex + blocksToWrite < len) {
                MetaData::arr[allocationIndex] = blocksToWrite;
                for (int i = allocationIndex; i < allocationIndex + blocksToWrite; i++) {
                    bitmap.set(i);
                }
                allocationIndex += blocksToWrite;
            } else if (allocationIndex < len) {
                MetaData::arr[allocationIndex] = 1;
                bitmap.set(allocationIndex);
                allocationIndex++;
            }
        }
    }
}

void allocateBlocksAfter() {
    while (true)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));

        {
            std::lock_guard<std::mutex> lock(bitmapMutex);
            int blocksToWrite = 15;
            allocationIndex %= len;
            int freeBlocks = deallocationIndex - allocationIndex;
            if(freeBlocks <= 0) freeBlocks += len;
            if(freeBlocks < blocksToWrite) continue;
            MetaData::arr[allocationIndex] = blocksToWrite;
            if (allocationIndex + blocksToWrite < len) {
                for (int i = allocationIndex; i < allocationIndex + blocksToWrite; i++) {
                    bitmap.set(i);
                }
                allocationIndex += blocksToWrite;
            } else {
                for(int i=allocationIndex; i < len; i++) {
                    bitmap.set(i);
                }
                for(int i=0; i < allocationIndex + blocksToWrite - len; i++) {
                    bitmap.set(i);
                }
                allocationIndex = (allocationIndex + blocksToWrite)%len;
            }
        }
    }
}


void deallocateBlocks() {
    while (true)
    {
        {
            std::lock_guard<std::mutex> lock(bitmapMutex);
            int blocksToDelete = MetaData::arr[deallocationIndex];
            if(deallocationIndex + blocksToDelete < len) {
                for(int i=deallocationIndex; i < deallocationIndex + blocksToDelete; i++) {
                    bitmap.reset(i);
                }
            } else {
                for(int i=deallocationIndex; i < len; i++) {
                    bitmap.reset(i);
                }
                for(int i=0; i < deallocationIndex + blocksToDelete - len; i++) {
                    bitmap.reset(i);
                }
            }
            MetaData::arr[deallocationIndex] = 0;
            deallocationIndex = (deallocationIndex+blocksToDelete)%len;
        }
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
}
void printHeatmap() {
    const std::string colors[] = {
        "\033[42m", // Green (free)
        "\033[41m"  // Red (allocated)
    };

    while (true) {
        std::cout << "\033[H"; // Move cursor to top to overwrite previous output
        {
            std::lock_guard<std::mutex> lock(bitmapMutex);
            for (int i = 0; i < len; i++) {
                std::cout << colors[bitmap[i]] << "  " << "\033[0m"; // Print colored block
            }
        }
        std::cout << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}


int main()
{

    std::thread renderThread([&]()
    {
        printHeatmap();
    });

    std::thread deallocateThread([&]()
    {
        deallocateBlocks();
    });

    std::thread allocateBlocksThread([&]()
    {
        allocateBlocksBefore();
        // allocateBlocksAfter();
    });

    allocateBlocksThread.join();
    deallocateThread.join();
    renderThread.join();

    return 0;
}


// to run

//  clear && g++ -std=c++17 sushantaDemo.cpp -o heatmap -I/opt/homebrew/include -L/opt/homebrew/lib -lboost_system -pthread && ./heatmap