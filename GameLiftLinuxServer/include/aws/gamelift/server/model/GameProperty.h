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
namespace Model
{

/**
    * <p>Set of key-value pairs containing information your game server requires to
    * set up sessions. This object allows you to pass in any set of data needed for
    * your game. For more information, see the <a
    * href="http://docs.aws.amazon.com/gamelift/latest/developerguide/">Amazon
    * GameLift Developer Guide</a>.</p>
    */
class AWS_GAMELIFT_API GameProperty
{
public:
    GameProperty(){}


    inline const std::string& GetKey() const{ return m_key; }


    inline void SetKey(const std::string& value) { m_key = value; }


    inline void SetKey(std::string&& value) { m_key = value; }


    inline void SetKey(const char* value) { m_key.assign(value); }


    inline GameProperty& WithKey(const std::string& value) { SetKey(value); return *this; }


    inline GameProperty& WithKey(std::string&& value) { SetKey(value); return *this; }


    inline GameProperty& WithKey(const char* value) { SetKey(value); return *this; }


    inline const std::string& GetValue() const{ return m_value; }


    inline void SetValue(const std::string& value) { m_value = value; }


    inline void SetValue(std::string&& value) { m_value = value; }


    inline void SetValue(const char* value) { m_value.assign(value); }


    inline GameProperty& WithValue(const std::string& value) { SetValue(value); return *this; }


    inline GameProperty& WithValue(std::string&& value) { SetValue(value); return *this; }


    inline GameProperty& WithValue(const char* value) { SetValue(value); return *this; }

private:
    std::string m_key;
    std::string m_value;
};

} // namespace Model
} // namespace Server
} // namespace GameLift
} // namespace Aws
