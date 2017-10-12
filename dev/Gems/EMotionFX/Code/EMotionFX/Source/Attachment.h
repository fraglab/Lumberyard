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

// include MCore related files
#include "EMotionFXConfig.h"
#include "BaseObject.h"


namespace EMotionFX
{
    // forward declarations
    class ActorInstance;


    /**
     * The attachment base class.
     * An attachment can be a simple weapon attached to a hand node, but also a mesh or set of meshes and bones that deform with the main skeleton.
     * This last example is useful for clothing items or character customization.
     */
    class EMFX_API Attachment
        : public BaseObject
    {
        MCORE_MEMORYOBJECTCATEGORY(Attachment, EMFX_DEFAULT_ALIGNMENT, EMFX_MEMCATEGORY_ATTACHMENTS);

    public:
        /**
         * Get the attachment type ID.
         * Every class inherited from this base class should have some TYPE ID.
         * @return The type ID of this attachment class.
         */
        virtual uint32 GetType() const = 0;

        /**
         * Get the attachment type string.
         * Every class inherited from this base class should have some type ID string, which should be equal to the class name really.
         * @return The type string of this attachment class, which should be the class name.
         */
        virtual const char* GetTypeString() const = 0;

        /**
         * Check if this attachment is being influenced by multiple nodes or not.
         * This is the case for attachments such as clothing items which get influenced by multiple nodes/bones inside the actor instance they are attached to.
         * @result Returns true if it is influenced by multiple nodes, otherwise false is returned.
         */
        virtual bool GetIsInfluencedByMultipleNodes() const = 0;

        /**
         * Update the attachment.
         * This can internally update node matrices for example, or other things.
         * This depends on the attachment type.
         */
        virtual void Update() = 0;

        /**
         * Get the actor instance object of the attachment.
         * This would for example return the actor instance that represents the gun when you attached a gun to a cowboy.
         * @result The actor instance representing the object you attach to something.
         */
        ActorInstance* GetAttachmentActorInstance() const;

        /**
         * Get the actor instance where we attach this attachment to.
         * This would for example return the cowboy, if we attach a gun to a cowboy.
         * @result The actor instance we are attaching something to.
         */
        ActorInstance* GetAttachToActorInstance() const;

        /**
         * Enable or disable fast update mode.
         * Fast updates mean that certain calculations will be skipped when updating the attachment.
         * @param allowFastUpdates Set to false to disable, and true to enable fast updates of deformable attachments.
         */
        void SetAllowFastUpdates(bool allowFastUpdates);

        /**
         * Check if this actor instance allows fast updates or not.
         * Fast updates mean that certain calculations will be skipped when updating the attachment.
         * @result Returns true when fast update mode is enabled, otherwise false is returned.
         */
        bool GetAllowFastUpdates() const;

    protected:
        ActorInstance*  mAttachment;        /**< The actor instance that represents the attachment. */
        ActorInstance*  mActorInstance;     /**< The actor instance where this attachment is added to. */
        bool            mFastUpdateMode;    /**< Enable to allow fast update mode [default=false]. */

        /**
         * The constructor.
         * @param attachToActorInstance The actor instance to attach to (for example a cowboy).
         * @param attachment The actor instance that you want to attach to this node (for example a gun).
         */
        Attachment(ActorInstance* attachToActorInstance, ActorInstance* attachment);

        /**
         * The destructor.
         * This does NOT delete the actor instance used by the attachment.
         */
        virtual ~Attachment();
    };
}   // namespace EMotionFX
