#include "logger.h"
#include <iostream>

namespace utilities
{
logger::logger () = default;
logger::~logger ()
{
    if (loggingThread.joinable ())
    {
        loggingQueue.emplace ("!!!close");
        clearEmptyFlag ();
        loggingThread.join ();
    }
}
void logger::openFile (const std::string &file)
{
    if (loggingThread.joinable ())
    {
        loggingQueue.emplace ("!!!close");
        clearEmptyFlag ();
        loggingThread.join ();
        outFile.open (file.c_str ());
        startLogging ();
    }
    else
    {
        outFile.open (file.c_str ());
    }
}

void logger::startLogging (int cLevel, int fLevel)
{
    if (loggingThread.joinable ())
    {
        return;
    }
    std::unique_lock<std::mutex> lock (cvMutex);
    consoleLevel = cLevel;
    fileLevel = fLevel;
    lock.unlock ();
    loggingThread = std::thread (&logger::loggerLoop, this);
}

void logger::changeLevels (int cLevel, int fLevel)
{
    std::lock_guard<std::mutex> lock (cvMutex);
    consoleLevel = cLevel;
    fileLevel = fLevel;
}

void logger::log (int level, std::string logMessage)
{
    logMessage.push_back ((level <= consoleLevel) ? 'c' : 'n');
    logMessage.push_back ((level <= fileLevel) ? 'f' : 'n');
    loggingQueue.emplace (std::move (logMessage));
    clearEmptyFlag ();
}

void logger::flush () { loggingQueue.emplace ("!!!flush"); }
bool logger::isRunning () const { return loggingThread.joinable (); }
void logger::clearEmptyFlag ()
{
    if (emptyFlag)
    {
        emptyFlag = false;
        std::unique_lock<std::mutex> lock (cvMutex);
        notifier.notify_one ();
    }
}

void logger::loggerLoop ()
{
    while (true)
    {
        auto msg = loggingQueue.pop ();
        if (msg)
        {
            if (msg->front () == '!')
            {
                if (msg->compare (3, 5, "close") == 0)
                {
                    break;  // break the loop
                }
                if (msg->compare (3, 5, "flush") == 0)
                {
                    if (outFile.is_open ())
                    {
                        outFile.flush ();
                    }
                    std::cout.flush ();
                    continue;
                }
            }
            // if a the file should be written there will be a 'f' at the end
            auto f = msg->back ();
            msg->pop_back ();
            // if a the console should be written there will be a 'c' at the end
            auto c = msg->back ();
            msg->pop_back ();
            if (f == 'f')
            {
                if (outFile.is_open ())
                {
                    outFile << *msg << '\n';
                }
            }
            if (c == 'c')
            {
                std::cout << *msg << '\n';
            }
        }
        else
        {
            emptyFlag = true;
            // on spurious wake up we just check the queue again so no need for a loop around this structure
            std::unique_lock<std::mutex> lock (cvMutex);
            notifier.wait (lock);
        }
    }
}

loggerNoThread::loggerNoThread () = default;

void loggerNoThread::openFile (const std::string &file) { outFile.open (file.c_str ()); }
void loggerNoThread::startLogging (int cLevel, int fLevel)
{
    consoleLevel = cLevel;
    fileLevel = fLevel;
}

void loggerNoThread::changeLevels (int cLevel, int fLevel)
{
    consoleLevel = cLevel;
    fileLevel = fLevel;
}

void loggerNoThread::log (int level, const std::string &logMessage)
{
    if (level < consoleLevel)
    {
        std::cout << logMessage << '\n';
    }
    if (level < fileLevel)
    {
        if (outFile.is_open ())
        {
            outFile << logMessage << '\n';
        }
    }
}

void loggerNoThread::flush ()
{
    if (outFile.is_open ())
    {
        outFile.flush ();
    }
    std::cout.flush ();
}

bool loggerNoThread::isRunning () const { return true; }
}  // namespace utilities