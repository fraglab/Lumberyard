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

#include "InAppPurchases_precompiled.h"

#include <InAppPurchases/InAppPurchasesInterface.h>

namespace InAppPurchases
{
    InAppPurchasesInterface* InAppPurchasesInterface::iapInstance = nullptr;

    InAppPurchasesInterface* InAppPurchasesInterface::GetInstance()
    {
        if (iapInstance == nullptr)
        {
            iapInstance = InAppPurchasesInterface::CreateInstance();
            AZ_Warning("InAppPurchases", iapInstance, "Inapp purchases not supported on this platform!");
        }

        return iapInstance;
    }

    void InAppPurchasesInterface::DestroyInstance()
    {
        if (iapInstance != nullptr)
        {
            iapInstance->m_cache.ClearCachedProductDetails();
            iapInstance->m_cache.ClearCachedPurchasedProductDetails();
            delete iapInstance;
            iapInstance = nullptr;
        }
    }
    
    void InAppPurchasesCache::ClearCachedProductDetails()
    {
        m_cachedProductDetails.set_capacity(0);
    }
    
    void InAppPurchasesCache::ClearCachedPurchasedProductDetails()
    {
        m_cachedPurchasedProducts.set_capacity(0);
    }
    
    void InAppPurchasesCache::AddProductDetailsToCache(const ProductDetails* productDetails)
    {
        m_cachedProductDetails.push_back(AZStd::unique_ptr<ProductDetails const>(productDetails));
    }
    
    void InAppPurchasesCache::AddPurchasedProductDetailsToCache(const PurchasedProductDetails* purchasedProductDetails)
    {
        m_cachedPurchasedProducts.push_back(AZStd::unique_ptr<PurchasedProductDetails const>(purchasedProductDetails));
    }
    
    const AZStd::vector<AZStd::unique_ptr<ProductDetails const> >& InAppPurchasesCache::GetCachedProductDetails() const
    {
        return m_cachedProductDetails;
    }
    
    const AZStd::vector<AZStd::unique_ptr<PurchasedProductDetails const> >& InAppPurchasesCache::GetCachedPurchasedProductDetails() const
    {
        return m_cachedPurchasedProducts;
    }
}
