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

#include <aws/gamelift/internal/GameLiftCommonState.h>
#include <aws/gamelift/server/GameLiftServerAPI.h>
#include <aws/gamelift/internal/network/Network.h>

namespace Aws
{
namespace GameLift
{
namespace Internal
{
    using namespace Aws::GameLift::Server::Model;

    class GameLiftServerState : public GameLiftCommonState, public Network::AuxProxyMessageHandler
    {
    public:

        static Server::InitSDKOutcome CreateInstance();

        virtual GAMELIFT_INTERNAL_STATE_TYPE GetStateType() { return GAMELIFT_INTERNAL_STATE_TYPE::SERVER; };

        // Singleton constructors should be private, but we are using a custom allocator that needs to be 
        // able to see them. Don't use these.
        GameLiftServerState();

        ~GameLiftServerState();

        GenericOutcome ProcessReady(const Aws::GameLift::Server::ProcessParameters &processParameters);

        GenericOutcome ProcessEnding();

        GenericOutcome InitializeNetworking();

        GenericOutcome ActivateGameSession();

        GenericOutcome UpdatePlayerSessionCreationPolicy(PlayerSessionCreationPolicy newPlayerSessionPolicy);

        GenericOutcome TerminateGameSession();

        std::string GetGameSessionId();

        GenericOutcome AcceptPlayerSession(const std::string& playerSessionId);

        GenericOutcome RemovePlayerSession(const std::string& playerSessionId);

        bool IsProcessReady() { return m_processReady; }

        //From Network::AuxProxyMessageHandler
        void OnStartGameSession(GameSession& gameSession, sio::message::list& ack_resp) override;
        void OnTerminateProcess() override;

    private:
        bool AssertNetworkInitialized();

        bool CreateSDKDetectionDirectory(std::string dir);

        void WaitForAndPopulateAckResponse(sio::message::list &ack_resp);

        void ReportHealth();
        void HealthCheck();
        bool DefaultHealthCheck() { return true; }

        std::function<void(Aws::GameLift::Server::Model::GameSession)> m_onStartGameSession;
        std::function<void()> m_onProcessTerminate;
        std::function<bool()> m_onHealthCheck;

        bool m_processReady;

        //Only one game session per process.
        std::string m_gameSessionId;

        Aws::GameLift::Internal::Network::Network* m_network;
    };

} // namespace Internal
} // namespace GameLift
} // namespace Aws
