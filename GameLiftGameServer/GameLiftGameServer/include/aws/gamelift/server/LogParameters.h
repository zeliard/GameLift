/*
* All or portions of this file Copyright (c) Amazon.com, Inc. or its affiliates or
* its licensors.
*
* For complete copyright and license terms please see the LICENSE at the root of this
* distribution (the "License"). All use of this software is governed by the License,
* or, if provided, by the license below or the license accompanying this file. Do not
* remove or modify any license notices. This file is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*
*/
#pragma once
#include <aws/gamelift/common/GameLift_EXPORTS.h>

namespace Aws
{
namespace GameLift
{
namespace Server
{
    class LogParameters
    {
    public:
        AWS_GAMELIFT_API LogParameters() {}
        AWS_GAMELIFT_API LogParameters(const std::vector<std::string> &logPaths) : m_logPaths(logPaths){}

        AWS_GAMELIFT_API std::vector<std::string> getLogPaths() const { return m_logPaths; }

    private:
        std::vector<std::string> m_logPaths;
    };
} // namespace Server
} // namespace GameLift
} // namespace Aws

