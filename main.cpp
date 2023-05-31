//
// Created by valera on 31.05.23.
//

/**
 * Main task:
 *  1 - Create global number
 *  2 - Increment from main func
 *  3 - Child func (in another thread) will wait value we need via futex
 *  4 - Increment number from child func
 *  5 - int main func will wait for a value we need and finish the program
 *  6 - ???
 *  7 - profit!
 */

#include <iostream>
#include <thread>
#include <linux/futex.h>
#include <syscall.h>
#include <unistd.h>

int *myTestNumber = new int(0);

void futexWaitValue(int *futexAddr, int valueToBlock)
{
    while(1)
    {
        int futexRc = static_cast<int>(syscall(SYS_futex, futexAddr, FUTEX_WAIT, valueToBlock, nullptr, nullptr, 0));

        if (futexRc == FUTEX_OP_CMP_EQ)
        {
            if (*futexAddr == valueToBlock)
                return;
        }
    }
}

void futexWakeBlock(int *addr)
{
    while(1)
    {
        int futexRc = static_cast<int>(syscall(SYS_futex, addr, FUTEX_WAKE, 1, nullptr, nullptr, 0));

        if (futexRc > 0)
            return;
    }
}

void doSomething()
{
    std::cout << "\n{ChildThread} [doSomething]: Wait for 5" << std::endl;
    futexWaitValue(myTestNumber, 5);

    std::cout << "{ChildThread} [doSomething]: I got 5! Writing 10..." << std::endl;

    *myTestNumber = 10;

    // Give a signal that we've changed the number
    futexWakeBlock(myTestNumber);

    std::cout << "{ChildThread} [doSomething]: Number changed to 10" << std::endl;
}

int main()
{
    std::thread myThread(doSomething);

    std::cout << "{MainThread} [main]: Changing number to 5..." << std::endl;
    *myTestNumber = 5;

    futexWakeBlock(myTestNumber);
    std::cout << "{MainThread} [main]: Number changed" << std::endl;

    futexWaitValue(myTestNumber, 10);

    if (*myTestNumber == 10)
        std::cout << "\n{MainThread} [main]: I got 10! OMG, futex works!!!" << std::endl;
    else
        std::cout << "\n{MainThread} [main]: No, futex doesn't work, you're moron (number = " << *myTestNumber << ")" << std::endl;

    myThread.join();
    return 0;
}