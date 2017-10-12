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


// check if the box is partially above the plane
MCORE_INLINE bool PlaneEq::PartiallyAbove(const AABB& box) const
{
    const Vector3 minVec = box.GetMin();
    const Vector3 maxVec = box.GetMax();
    const Vector3 testPoint(IsNegative(mNormal.x) ? minVec.x : maxVec.x,
        IsNegative(mNormal.y) ? minVec.y : maxVec.y,
        IsNegative(mNormal.z) ? minVec.z : maxVec.z);

    return IsPositive(mNormal.Dot(testPoint) + mDist);
}


// check if the box is completely above the plane
MCORE_INLINE bool PlaneEq::CompletelyAbove(const AABB& box) const
{
    const Vector3 minVec = box.GetMin();
    const Vector3 maxVec = box.GetMax();
    const Vector3 testPoint(IsPositive(mNormal.x) ? minVec.x : maxVec.x,
        IsPositive(mNormal.y) ? minVec.y : maxVec.y,
        IsPositive(mNormal.z) ? minVec.z : maxVec.z);

    return IsPositive(mNormal.Dot(testPoint) + mDist);
}
