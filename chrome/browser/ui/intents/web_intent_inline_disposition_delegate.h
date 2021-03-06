// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_INTENTS_WEB_INTENT_INLINE_DISPOSITION_DELEGATE_H_
#define CHROME_BROWSER_UI_INTENTS_WEB_INTENT_INLINE_DISPOSITION_DELEGATE_H_
#pragma once

#include "base/compiler_specific.h"
#include "content/public/browser/web_contents_delegate.h"

class WebIntentPicker;

// This class is the policy delegate for the rendered page in the intents
// inline disposition bubble.
class WebIntentInlineDispositionDelegate : public content::WebContentsDelegate {
 public:
  // |picker| is notified when the web contents loading state changes. Must not
  // be NULL.
  explicit WebIntentInlineDispositionDelegate(WebIntentPicker* picker);
  virtual ~WebIntentInlineDispositionDelegate();

  // WebContentsDelegate implementation.
  virtual bool IsPopupOrPanel(
      const content::WebContents* source) const OVERRIDE;
  virtual bool ShouldAddNavigationToHistory(
    const history::HistoryAddPageArgs& add_page_args,
    content::NavigationType navigation_type) OVERRIDE;
  virtual content::WebContents* OpenURLFromTab(
      content::WebContents* source,
      const content::OpenURLParams& params) OVERRIDE;
  virtual void LoadingStateChanged(content::WebContents* source) OVERRIDE;

 private:
  // Picker to notify when loading state changes. Weak pointer.
  WebIntentPicker* picker_;
};

#endif  // CHROME_BROWSER_UI_INTENTS_WEB_INTENT_INLINE_DISPOSITION_DELEGATE_H_
