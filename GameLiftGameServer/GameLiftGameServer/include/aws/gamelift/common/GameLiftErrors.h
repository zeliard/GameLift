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
    enum class AWS_GAMELIFT_API GAMELIFT_ERROR_TYPE
    {
        ALREADY_INITIALIZED,            // The GameLift Server or Client has already been initialized with Initialize().
        FLEET_MISMATCH,                 // The target fleet does not match the fleet of a gameSession or playerSession.
        GAMELIFT_CLIENT_NOT_INITIALIZED,// The GameLift client has not been initialized.
        GAMELIFT_SERVER_NOT_INITIALIZED,//The GameLift server has not been initialized.
        GAME_SESSION_ENDED_FAILED,      // The GameLift Server SDK could not contact the service to report the game session ended.
        GAME_SESSION_NOT_READY,         // The GameLift Server Game Session was not activated.
        GAME_SESSION_READY_FAILED,      // The GameLift Server SDK could not contact the service to report the game session is ready.
        INITIALIZATION_MISMATCH,        // A client method was called after Server::Initialize(), or vice versa.
        NOT_INITIALIZED,                // The GameLift Server or Client has not been initialized with Initialize().
        NO_TARGET_ALIASID_SET,          // A target aliasId has not been set.
        NO_TARGET_FLEET_SET,            // A target fleet has not been set.
        PROCESS_ENDING_FAILED,          // The GameLift Server SDK could not contact the service to report the process is ending.
        PROCESS_NOT_ACTIVE,             // The server process is not yet active, not bound to a GameSession, and cannot accept or process PlayerSessions.
        PROCESS_NOT_READY,              // The server process is not yet ready to be activated.
        PROCESS_READY_FAILED,           // The GameLift Server SDK could not contact the service to report the process is ready.
        SDK_VERSION_DETECTION_FAILED,   // SDK version detection failed.
        SERVICE_CALL_FAILED,            // A call to an AWS service has failed.
        STX_CALL_FAILED,                // A call to the XStx server backend component has failed.
        STX_INITIALIZATION_FAILED,      // The XStx server backend component has failed to initialize.
        UNEXPECTED_PLAYER_SESSION       // An unregistered player session was encountered by the server.
    };

    class AWS_GAMELIFT_API GameLiftError
    {

    public:
        const GAMELIFT_ERROR_TYPE GetErrorType() const { return m_errorType; }

        const std::string& GetErrorName() const { return m_errorName; }
        void SetErrorName(const std::string& errorName) { m_errorName = errorName; }

        const std::string& GetErrorMessage() const { return m_errorMessage; }
        void SetErrorMessage(const std::string& errorMessage) { m_errorMessage = errorMessage; }

        static std::string GetDefaultNameForErrorType(GAMELIFT_ERROR_TYPE errorType);

        static std::string GetDefaultMessageForErrorType(GAMELIFT_ERROR_TYPE errorType);

        GameLiftError();

        virtual ~GameLiftError();
            
        GameLiftError(GAMELIFT_ERROR_TYPE errorType);

        GameLiftError(GAMELIFT_ERROR_TYPE errorType, std::string errorName, std::string message);
            
        GameLiftError(const GameLiftError& rhs);

    private:
        GAMELIFT_ERROR_TYPE m_errorType;
        std::string m_errorName;
        std::string m_errorMessage;
    };
} //namespace GameLift
} //namespace Aws