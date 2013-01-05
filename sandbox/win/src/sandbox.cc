// Copyright (c) 2006-2008 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stdio.h>
#include <windows.h>
#include "sandbox/win/src/sandbox.h"
#include "sandbox/win/src/sandbox_factory.h"
#include "sandbox/win/src/broker_services.h"
#include "sandbox/win/src/target_services.h"

#if defined(_WIN64) && !defined(NACL_WIN64)
// We allow building this code for Win64 as part of NaCl to enable development
#pragma message("Sandbox code was not fully tested on 64-bit Windows.\
 crbug.com/168414 ")
#endif


namespace sandbox {
// The section for IPC and policy.
SANDBOX_INTERCEPT HANDLE  g_shared_section = NULL;

static bool               s_is_broker =  false;

// GetBrokerServices: the current implementation relies on a shared section
// that is created by the broker and opened by the target.
BrokerServices* SandboxFactory::GetBrokerServices() {
  // Can't be the broker if the shared section is open.
  if (NULL != g_shared_section) {
    return NULL;
  }
  // If the shared section does not exist we are the broker, then create
  // the broker object.
  s_is_broker = true;
  return BrokerServicesBase::GetInstance();
}

// GetTargetServices implementation must follow the same technique as the
// GetBrokerServices, but in this case the logic is the opposite.
TargetServices* SandboxFactory::GetTargetServices() {
  // Can't be the target if the section handle is not valid.
  if (NULL == g_shared_section) {
    return NULL;
  }
  // We are the target
  s_is_broker = false;
  // Creates and returns the target services implementation.
  return TargetServicesBase::GetInstance();
}

}  // namespace sandbox
