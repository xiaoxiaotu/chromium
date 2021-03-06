// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/logging.h"
#include "remoting/protocol/clipboard_filter.h"

namespace remoting {
namespace protocol {

ClipboardFilter::ClipboardFilter() : clipboard_stub_(NULL) {
}

ClipboardFilter::~ClipboardFilter() {
}

void ClipboardFilter::set_clipboard_stub(ClipboardStub* clipboard_stub) {
  clipboard_stub_ = clipboard_stub;
}

void ClipboardFilter::InjectClipboardEvent(const ClipboardEvent& event) {
  if (clipboard_stub_ != NULL)
    clipboard_stub_->InjectClipboardEvent(event);
}

}  // namespace protocol
}  // namespace remoting
