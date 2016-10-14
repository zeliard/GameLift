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

#include <aws/gamelift/server/model/GameSession.h>
#include <aws/gamelift/server/LogParameters.h>
#include <functional>

namespace Aws
{
namespace GameLift
{
namespace Server
{
    class ProcessParameters
    {
    public:
        ProcessParameters() {}

        ProcessParameters(const std::function<void(Aws::GameLift::Server::Model::GameSession)> onStartGameSession,
            const std::function<void()> onProcessTerminate,
            const std::function<bool()> onHealthCheck,
            int port,
            const Aws::GameLift::Server::LogParameters logParameters) : m_onStartGameSession(onStartGameSession),
            m_onProcessTerminate(onProcessTerminate),
            m_onHealthCheck(onHealthCheck),
            m_port(port),
            m_logParameters(logParameters){}

        AWS_GAMELIFT_API std::function<void(Aws::GameLift::Server::Model::GameSession)> getOnStartGameSession() const { return m_onStartGameSession; }
        AWS_GAMELIFT_API std::function<void()> getOnProcessTerminate() const { return m_onProcessTerminate; }
        AWS_GAMELIFT_API std::function<bool()> getOnHealthCheck() const { return m_onHealthCheck; }
        AWS_GAMELIFT_API int getPort() const { return m_port; }
        AWS_GAMELIFT_API Aws::GameLift::Server::LogParameters getLogParameters() const { return m_logParameters; }

    private:
        std::function<void(Aws::GameLift::Server::Model::GameSession)> m_onStartGameSession = nullptr;
        std::function<void()> m_onProcessTerminate = nullptr;
        std::function<bool()> m_onHealthCheck = nullptr;
        int m_port = -1;
        Aws::GameLift::Server::LogParameters m_logParameters = LogParameters();
    };
} // namespace Server
} // namespace GameLift
} // namespace Aws

