/*
 * Copyright (c) 2014-2020, Lawrence Livermore National Security
 * See the top-level NOTICE for additional details. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 */
#pragma once

#include "griddyn/measurement/collector.h"

namespace griddyn {
namespace tcpLib {
    class TcpConnection;

    class tcpCollector: public collector {
      private:
        std::string server;
        std::string port;
        std::shared_ptr<TcpConnection> connection;

      public:
        tcpCollector(coreTime time0 = timeZero, coreTime period = timeOneSecond);
        explicit tcpCollector(const std::string& name);
        ~tcpCollector();

        virtual std::unique_ptr<collector> clone() const override;

        virtual void cloneTo(collector* col) const override;
        virtual change_code trigger(coreTime time) override;

        void set(const std::string& param, double val) override;
        void set(const std::string& param, const std::string& val) override;

        virtual const std::string& getSinkName() const override;
    };

}  // namespace tcpLib
}  // namespace griddyn
