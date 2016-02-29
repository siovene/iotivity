//******************************************************************
//
// Copyright 2016 Samsung Electronics All Rights Reserved.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

#include "RemoteSceneAction.h"

#include <cassert>

#include "SceneCommons.h"
#include "SceneMemberResourceRequestor.h"

namespace OIC
{
    namespace Service
    {

        RemoteSceneAction::RemoteSceneAction(
            SceneMemberResourceRequestor::Ptr requestor,
            const std::string &sceneName, const RCSResourceAttributes &attrs)
                : m_sceneName{ sceneName }, m_attributes{ attrs }, m_requestor{ requestor }
        {
            assert(requestor);
        }

        RemoteSceneAction::RemoteSceneAction(
            SceneMemberResourceRequestor::Ptr requestor, const std::string &sceneName,
            const std::string &key, const RCSResourceAttributes::Value &value)
                : m_sceneName{ sceneName }, m_requestor{ requestor }
        {
            assert(requestor);
            m_attributes[key] = value;
        }

        void RemoteSceneAction::setExecutionParameter(const std::string &key,
                                       const RCSResourceAttributes::Value &value,
                                       UpdateCallback clientCB)
        {
            if (key.empty())
            {
                throw RCSInvalidParameterException("Scene action key value is empty");
            }

            RCSResourceAttributes attr;
            attr[key] = RCSResourceAttributes::Value(value);

            setExecutionParameter(attr, std::move(clientCB));
        }

        void RemoteSceneAction::setExecutionParameter(const RCSResourceAttributes &attr,
            UpdateCallback clientCB)
        {
            if (attr.empty())
            {
                throw RCSInvalidParameterException("RCSResourceAttributes is empty");
            }

            SceneMemberResourceRequestor::InternalAddSceneActionCallback internalCB
                = std::bind(&RemoteSceneAction::onUpdated, this,
                std::placeholders::_1, attr, std::move(clientCB));

            m_requestor->requestSceneActionCreation(
                m_sceneName, attr, internalCB);
        }

        RCSResourceAttributes RemoteSceneAction::getExecutionParameter() const
        {
            return m_attributes;
        }

        RCSRemoteResourceObject::Ptr RemoteSceneAction::getRemoteResourceObject() const
        {
            return m_requestor->getRemoteResourceObject();
        }

        void RemoteSceneAction::onUpdated(int eCode, const RCSResourceAttributes &attr,
                                          const UpdateCallback &clientCB)
        {
            int result = SCENE_CLIENT_BADREQUEST;
            if (eCode == SCENE_RESPONSE_SUCCESS)
            {
                std::lock_guard< std::mutex > lock(m_attributeLock);
                m_attributes = attr;
                result = SCENE_RESPONSE_SUCCESS;
            }

            clientCB(result);
        }

    }
}