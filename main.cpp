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

int futex(int *uaddr, int futex_op, int val, const struct timespec *timeout, int *uaddr2, int val3)
{
    return syscall(SYS_futex, uaddr, futex_op, val, timeout, uaddr, val3);
}

int futexWaitValue(int *futexAddr, int valueToBlock)
{
    int futex_rc = 0;
    while (1)
    {
        futex_rc = futex(futexAddr, FUTEX_WAIT, valueToBlock, nullptr, nullptr, 0);
        if (futex_rc == -1)
        {
            if(errno != EAGAIN)
                return -1;
        }
        else
            return futex_rc;

    }
}

void futexWakeBlock(int *addr)
{
    while(1)
    {
        int futexResponseCode = futex(addr, FUTEX_WAKE, 1, nullptr, nullptr, 0);

        if (futexResponseCode > 0)
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

    std::cout << "\n{MainThread} [main]: Changing number to 5..." << std::endl;
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