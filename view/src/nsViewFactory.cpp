/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 *
 * The contents of this file are subject to the Netscape Public
 * License Version 1.1 (the "License"); you may not use this file
 * except in compliance with the License. You may obtain a copy of
 * the License at http://www.mozilla.org/NPL/
 *
 * Software distributed under the License is distributed on an "AS
 * IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
 * implied. See the License for the specific language governing
 * rights and limitations under the License.
 *
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is Netscape
 * Communications Corporation.  Portions created by Netscape are
 * Copyright (C) 1998 Netscape Communications Corporation. All
 * Rights Reserved.
 *
 * Contributor(s): 
 *   Pierre Phaneuf <pp@ludusdesign.com>
 */

#include "nscore.h"
#include "nsIGenericFactory.h"
#include "nsISupports.h"
#include "nsCOMPtr.h"
#include "nsIServiceManager.h"
#include "nsIGenericFactory.h"

#include "nsViewsCID.h"
#include "nsView.h"
#include "nsScrollingView.h"
#include "nsScrollPortView.h"

#include "nsIModule.h"
#include "nsIPref.h"

static const char kUseNewViewManagerPref[] = "nglayout.debug.enable_scary_view_manager";

#include "nsViewManager.h"
#include "nsViewManager2.h"

NS_GENERIC_FACTORY_CONSTRUCTOR(nsViewManager)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsViewManager2)

static NS_IMETHODIMP ViewManagerConstructor(nsISupports *aOuter, REFNSIID aIID, void **aResult)
{
  PRBool useNewViewManager = PR_TRUE;

  nsCOMPtr<nsIPref> prefs( do_GetService(NS_PREF_CONTRACTID) );
  if (prefs)
    prefs->GetBoolPref(kUseNewViewManagerPref, &useNewViewManager);

  return useNewViewManager ? nsViewManagerConstructor(aOuter, aIID, aResult)
                           : nsViewManager2Constructor(aOuter, aIID, aResult);
}

/* man, I'm going to hell for this, but they're not refcounted */
#undef NS_ADDREF
#define NS_ADDREF(x)  (void)0;
#undef NS_RELEASE
#define NS_RELEASE(x) (void)0;
NS_GENERIC_FACTORY_CONSTRUCTOR(nsView)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsScrollingView)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsScrollPortView)

static nsModuleComponentInfo components[] = {
  { "View Manager", NS_VIEW_MANAGER_CID, "@mozilla.org/view-manager;1",
    ViewManagerConstructor },
  { "View", NS_VIEW_CID, "@mozilla.org/view;1", nsViewConstructor },
  { "Scrolling View", NS_SCROLLING_VIEW_CID, "@mozilla.org/scrolling-view;1",
    nsScrollingViewConstructor },
  { "Scroll Port View", NS_SCROLL_PORT_VIEW_CID,
    "@mozilla.org/scroll-port-view;1", nsScrollPortViewConstructor }
};

NS_IMPL_NSGETMODULE(nsViewModule, components)
