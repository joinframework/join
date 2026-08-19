/**
 * MIT License
 *
 * Copyright (c) 2026 Mathieu Rabine
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

// libjoin.
#include <join/interface_manager.hpp>
#include <join/condition.hpp>
#include <join/version.hpp>
#include <join/mutex.hpp>
#include <join/timer.hpp>

// C++.
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <string>

// C.
#include <unistd.h>

using join::InterfaceChangeType;
using join::InterfaceManager;
using join::ScopedLock;
using join::Condition;
using join::Interface;
using join::Monotonic;
using join::LinkInfo;
using join::Mutex;

/// guards the state shared with the reactor thread.
static Mutex mutex;

/// signalled when the reactor thread has an action to hand over.
static Condition condition;

/// an action is waiting to be run.
static bool pending = false;

/// the monitored interface was removed.
static bool removed = false;

// =========================================================================
//   CLASS     :
//   METHOD    : version
// =========================================================================
void version ()
{
    std::cout << "ifplugd version " << JOIN_VERSION << "\n";
}

// =========================================================================
//   CLASS     :
//   METHOD    : usage
// =========================================================================
void usage ()
{
    std::cout << "usage: ifplugd [options] -I device\n";
    std::cout << "\n";
    std::cout << "  -d delay    delay in ms before acting on link down (default: 5000)\n";
    std::cout << "  -h          display this help and exit\n";
    std::cout << "  -I device   interface to monitor\n";
    std::cout << "  -r command  command to run on link change, called as: command device up|down\n";
    std::cout << "  -u delay    delay in ms before acting on link up (default: 100)\n";
    std::cout << "  -v          display version information and exit\n";
}

// =========================================================================
//   CLASS     :
//   METHOD    : changed
// =========================================================================
bool changed (InterfaceChangeType flags, InterfaceChangeType type)
{
    return static_cast<uint32_t> (flags & type) != 0;
}

// =========================================================================
//   CLASS     :
//   METHOD    : main
// =========================================================================
int main (int argc, char* argv[])
{
    int downDelay = 5000, upDelay = 100, opt = 0;
    std::string device, command;

    while ((opt = getopt (argc, argv, "d:hI:r:u:v")) != -1)
    {
        switch (opt)
        {
            case 'd':
                downDelay = std::stoi (optarg);
                break;
            case 'I':
                device = optarg;
                break;
            case 'r':
                command = optarg;
                break;
            case 'u':
                upDelay = std::stoi (optarg);
                break;
            case 'v':
                version ();
                return 0;
            case 'h':
                usage ();
                return 0;
            default:
                usage ();
                return 1;
        }
    }

    if (device.empty ())
    {
        usage ();
        return 1;
    }

    InterfaceManager& interfaces = InterfaceManager::instance ();

    Interface::Ptr interface = interfaces.findByName (device);
    if (interface == nullptr)
    {
        std::cerr << "ifplugd: " << device << ": no such device" << std::endl;
        return 1;
    }

    std::cout << "ifplugd: monitoring " << device;
    std::cout << ", link is " << (interface->isRunning () ? "up" : "down") << std::endl;

    Monotonic::Timer timer;

    uint64_t id = interfaces.addLinkListener ([&] (const LinkInfo& info) {
        if ((info.interface == nullptr) || (info.interface->index () != interface->index ()))
        {
            return;
        }

        if (changed (info.flags, InterfaceChangeType::Deleted))
        {
            ScopedLock<Mutex> lock (mutex);
            removed = true;
            pending = true;
            condition.signal ();
            return;
        }

        if (changed (info.flags, InterfaceChangeType::OperStateChanged))
        {
            int delay = std::max (1, interface->isRunning () ? upDelay : downDelay);

            timer.setOneShot (std::chrono::milliseconds (delay), [] {
                ScopedLock<Mutex> lock (mutex);
                pending = true;
                condition.signal ();
            });
        }
    });

    bool reported = interface->isRunning ();

    for (;;)
    {
        bool up = false, gone = false;

        {
            ScopedLock<Mutex> lock (mutex);

            condition.wait (lock, [] {
                return pending;
            });

            pending = false;
            gone = removed;
            up = !gone && interface->isRunning ();
        }

        if (up != reported)
        {
            reported = up;

            std::cout << "ifplugd: " << device << " link " << (up ? "up" : "down") << std::endl;

            if (!command.empty ())
            {
                std::string action = command + " " + device + (up ? " up" : " down");
                [[maybe_unused]] int result = std::system (action.c_str ());
            }
        }

        if (gone)
        {
            std::cout << "ifplugd: " << device << " was removed" << std::endl;
            break;
        }
    }

    interfaces.removeLinkListener (id);

    return 0;
}
