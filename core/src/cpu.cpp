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
#include <join/cpu.hpp>

// C++.
#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <tuple>
#include <map>

// C.
#include <dirent.h>
#include <cstdlib>
#include <cstring>

using join::LogicalCpu;
using join::PhysicalCore;
using join::NumaNode;
using join::CpuTopology;

// =========================================================================
//   CLASS     : CpuTopology
//   METHOD    : instance
// =========================================================================
const CpuTopology* CpuTopology::instance ()
{
    static CpuTopology topology;
    return &topology;
}

// =========================================================================
//   CLASS     : CpuTopology
//   METHOD    : cores
// =========================================================================
const std::vector <PhysicalCore>& CpuTopology::cores () const noexcept
{
    return _cores;
}

// =========================================================================
//   CLASS     : CpuTopology
//   METHOD    : nodes
// =========================================================================
const std::vector <NumaNode>& CpuTopology::nodes () const noexcept
{
    return _nodes;
}

#ifdef DEBUG
// =========================================================================
//   CLASS     : CpuTopology
//   METHOD    : dump
// =========================================================================
void CpuTopology::dump () const
{
    for (const auto& node : _nodes)
    {
        std::cout << "NUMA " << node.id << ":" << std::endl;

        std::map <int, std::vector <const PhysicalCore*>> sockets;
        for (const auto& core : _cores)
        {
            if (core.numa == node.id)
            {
                sockets[core.socket].push_back (&core);
            }
        }

        for (auto const& it : sockets)
        {
            int socketId = it.first;
            const auto& coresInSocket = it.second;

            std::cout << "  Socket " << socketId << ":" << std::endl;
            for (const auto* c : coresInSocket)
            {
                std::cout << "     Core " << c->id << ": [ ";
                for (size_t i = 0; i < c->threads.size (); ++i) {
                    std::cout << c->threads[i].id << (i == c->threads.size () - 1 ? "" : ", ");
                }
                std::cout << " ]" << std::endl;
            }
        }
    }
}
#endif

// =========================================================================
//   CLASS     : CpuTopology
//   METHOD    : CpuTopology
// =========================================================================
CpuTopology::CpuTopology ()
{
    detect ();
}

// =========================================================================
//   CLASS     : CpuTopology
//   METHOD    : readInt
// =========================================================================
int CpuTopology::readInt (const std::string& path)
{
    int value = -1;

    std::ifstream file (path);
    if (file)
    {
        file >> value;
    }

    return value;
}

// =========================================================================
//   CLASS     : CpuTopology
//   METHOD    : findNuma
// =========================================================================
int CpuTopology::findNuma (const std::string& cpuPath)
{
    int nodeId = 0;

    DIR* dir = opendir (cpuPath.c_str ());
    if (dir)
    {
        struct dirent* entry;
        while ((entry = readdir (dir)))
        {
            if (std::strncmp (entry->d_name, "node", 4) == 0)
            {
                nodeId = std::atoi (entry->d_name + 4);
                break;
            }
        }

        closedir (dir);
    }

    return nodeId;
}

// =========================================================================
//   CLASS     : CpuTopology
//   METHOD    : detect
// =========================================================================
void CpuTopology::detect ()
{
    _cores.clear ();
    _nodes.clear ();

    std::map <std::tuple <int, int, int>, size_t> coreMap;
    std::map <int, size_t> nodeMap;

    DIR* dir = opendir ("/sys/devices/system/cpu");
    if (!dir)
    {
        return;
    }

    struct dirent* entry;
    while ((entry = readdir (dir)))
    {
        if (std::strncmp (entry->d_name, "cpu", 3) != 0)
        {
            continue;
        }

        char* end;
        long cpuId = std::strtol (entry->d_name + 3, &end, 10);
        if (*end != '\0')
        {
            continue;
        }

        std::string cpuPath = "/sys/devices/system/cpu/";
        cpuPath += entry->d_name;

        int coreId = readInt (cpuPath + "/topology/core_id");
        int socket = readInt (cpuPath + "/topology/physical_package_id");
        int numa = findNuma (cpuPath);

        auto coreKey = std::make_tuple (socket, coreId, numa);
        size_t coreIndex;

        auto coreIt = coreMap.find (coreKey);
        if (coreIt == coreMap.end ())
        {
            coreIndex = _cores.size ();
            _cores.push_back ({coreId, socket, numa, {}});
            coreMap[coreKey] = coreIndex;

            size_t nodeIndex;
            auto nodeIt = nodeMap.find (numa);
            if (nodeIt == nodeMap.end ())
            {
                nodeIndex = _nodes.size ();
                _nodes.push_back ({numa, {}});
                nodeMap[numa] = nodeIndex;
            }
            else
            {
                nodeIndex = nodeIt->second;
            }

            _nodes[nodeIndex].cores.push_back (coreId);
        }
        else
        {
            coreIndex = coreIt->second;
        }

        _cores[coreIndex].threads.push_back ({
            static_cast <int> (cpuId),
            coreId,
            socket,
            numa
        });
    }

    std::sort (_cores.begin (), _cores.end (), [] (const PhysicalCore& a, const PhysicalCore& b) {
        if (a.socket != b.socket)
        {
            return a.socket < b.socket;
        }
        return a.id < b.id;
    });

    for (auto& core : _cores)
    {
        std::sort (core.threads.begin (), core.threads.end (), [] (const LogicalCpu& a, const LogicalCpu& b) {
            return a.id < b.id;
        });
    }

    std::sort (_nodes.begin (), _nodes.end (), [] (const NumaNode& a, const NumaNode& b) {
        return a.id < b.id;
    });

    closedir (dir);
}
