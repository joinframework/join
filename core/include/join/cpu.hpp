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

#ifndef __JOIN_CPU_HPP__
#define __JOIN_CPU_HPP__

// C++.
#include <string>
#include <vector>

// C.
#include <cstdint>

namespace join
{
    /**
     * @brief logical CPU (hardware thread).
     */
    struct LogicalCpu
    {
        int id;                             /**< logical CPU ID */
        int core;                           /**< physical core ID */
        int socket;                         /**< physical socket ID */
        int numa;                           /**< NUMA node ID */
    };

    /**
     * @brief physical CPU core (may have multiple logical CPU).
     */
    struct PhysicalCore
    {
        int id;                             /**< physical CPU ID */
        int socket;                         /**< physical socket ID */
        int numa;                           /**< NUMA node ID */
        std::vector <LogicalCpu> threads;   /**< logical CPUs (SMT/HT) */

        /**
         * @brief get primary thread (first hardware thread, avoids HT).
         * @return logical CPU ID.
         */
        int primaryThread () const noexcept
        {
            return threads.empty () ? -1 : threads[0].id;
        }
    };

    /**
     * @brief NUMA node containing multiple cores.
     */
    struct NumaNode
    {
        int id;                             /**< NUMA node ID */
        std::vector <int> cores;            /**< physical core IDs */
    };

    /**
     * @brief CPU topology detector.
     */
    class CpuTopology
    {
    public:
        /**
         * @brief get instance.
         * @return CpuTopology instance.
         */
        static const CpuTopology* instance ();

        /**
         * @brief copy constructor.
         * @param other other object to copy.
         */
        CpuTopology (const CpuTopology& other) = delete;

        /**
         * @brief copy assignment operator.
         * @param other other object to copy.
         * @return current object.
         */
        CpuTopology& operator= (const CpuTopology& other) = delete;

        /**
         * @brief get all physical cores.
         * @return vector of physical cores.
         */
        const std::vector <PhysicalCore>& cores () const noexcept;

        /**
         * @brief get all NUMA nodes.
         * @return vector of NUMA nodes.
         */
        const std::vector <NumaNode>& nodes () const noexcept;

        /**
         * @brief dump CPU topology to standard output.
         */
    #ifdef DEBUG
        void dump () const;
    #endif

    private:
        /**
         * @brief construct and detect CPU topology.
         */
        CpuTopology ();

        /**
         * @brief destroy instance.
         */
        ~CpuTopology () = default;

        /**
         * @brief read integer from sysfs file.
         * @param path file path.
         * @return integer value, or -1 on error.
         */
        static int readInt (const std::string& path);

        /**
         * @brief find NUMA node for a CPU from sysfs.
         * @param cpuPath path to /sys/devices/system/cpu/cpuX.
         * @return NUMA node id.
         */
        static int findNuma (const std::string& cpuPath);

        /**
         * @brief detect CPU topology from /sys/devices/system/cpu.
         */
        void detect ();

        /// physical cores.
        std::vector <PhysicalCore> _cores;

        /// NUMA nodes.
        std::vector <NumaNode> _nodes;
    };
}

#endif
