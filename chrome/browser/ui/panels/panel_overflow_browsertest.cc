// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <vector>
#include "chrome/browser/ui/browser.h"
#include "chrome/browser/ui/panels/base_panel_browser_test.h"
#include "chrome/browser/ui/panels/native_panel.h"
#include "chrome/browser/ui/panels/panel.h"
#include "chrome/browser/ui/panels/panel_manager.h"
#include "chrome/browser/ui/panels/panel_overflow_strip.h"
#include "chrome/browser/ui/panels/panel_settings_menu_model.h"
#include "chrome/browser/ui/panels/panel_strip.h"
#include "chrome/browser/ui/panels/test_panel_mouse_watcher.h"
#include "chrome/common/chrome_notification_types.h"
#include "chrome/test/base/ui_test_utils.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace {

//  We override the default value for testing purpose.
const int kMaxVisibleOverflowForTesting = 3;

// Encapsulates all the info we need to verify if a panel behaves as expected
// when we do the overflow testing.
struct PanelData {
  Panel* panel;
  Panel::ExpansionState expansion_state;
  bool visible;
  bool active;

  explicit PanelData(Panel* panel)
      : panel(panel),
        expansion_state(panel->expansion_state()),
        visible(!panel->GetBounds().IsEmpty()),
        active(panel->IsActive()) {
  }

  PanelData(Panel* panel, Panel::ExpansionState expansion_state,
            bool visible, bool active)
      : panel(panel),
        expansion_state(expansion_state),
        visible(visible),
        active(active) {
  }

  bool operator==(const PanelData& another) const {
    return panel == another.panel &&
           expansion_state == another.expansion_state &&
           visible == another.visible &&
           active == another.active;
  }

  bool operator!=(const PanelData& another) const {
    return !(*this == another);
  }
};

// For gtest printing.
::std::ostream& operator<<(::std::ostream& os, const PanelData& data);
::std::ostream& operator<<(::std::ostream& os, const PanelData& data) {
  return os << "(" << data.panel->browser()->app_name() << ", "
            << data.expansion_state << ", " << data.visible << ", "
            << data.active << ")";
}


class PanelDataList : public std::vector<PanelData> {
 public:
  void Add(Panel* panel, Panel::ExpansionState expansion_state,
           bool visible, bool active) {
    push_back(PanelData(panel, expansion_state, visible, active));
  }
};

}  // namespace

class PanelOverflowBrowserTest : public BasePanelBrowserTest {
 public:
  PanelOverflowBrowserTest() : BasePanelBrowserTest() {
  }

  virtual ~PanelOverflowBrowserTest() {
  }

  virtual void SetUpOnMainThread() OVERRIDE {
    BasePanelBrowserTest::SetUpOnMainThread();

    PanelManager::GetInstance()->panel_overflow_strip()->
        set_max_visible_panels(kMaxVisibleOverflowForTesting);

    // All the overflow tests assume 800x600 work area. Do the check now.
    DCHECK(PanelManager::GetInstance()->work_area().width() == 800);
  }

 protected:
  static PanelDataList GetAllNormalPanelData() {
    PanelDataList panel_data_list;
    PanelStrip::Panels panels =
        PanelManager::GetInstance()->panel_strip()->panels();
    for (PanelStrip::Panels::const_iterator iter = panels.begin();
         iter != panels.end(); ++iter) {
      Panel* panel = *iter;
      panel_data_list.push_back(PanelData(panel));
    }
    return panel_data_list;
  }

  static PanelDataList GetAllOverflowPanelData() {
    PanelDataList panel_data_list;
    PanelOverflowStrip::Panels panels =
        PanelManager::GetInstance()->panel_overflow_strip()->panels();
    for (PanelOverflowStrip::Panels::const_iterator iter = panels.begin();
         iter != panels.end(); ++iter) {
      Panel* panel = *iter;
      panel_data_list.push_back(PanelData(panel));
    }
    return panel_data_list;
  }

  static void MoveMouseAndWaitForOverflowAnimationEnded(
      const gfx::Point& position) {
    ui_test_utils::WindowedNotificationObserver signal(
        chrome::NOTIFICATION_PANEL_BOUNDS_ANIMATIONS_FINISHED,
        content::Source<PanelOverflowStrip>(
            PanelManager::GetInstance()->panel_overflow_strip()));
    MoveMouse(position);
    signal.Wait();
  }

  static bool IsPanelVisible(Panel* panel) {
    return !panel->GetBounds().IsEmpty();
  }

  std::vector<Panel*> CreateOverflowPanels(int num_normal_panels,
                                           int num_overflow_panels,
                                           const int* panel_widths) {
    const int kTestPanelHeight = 200;
    std::vector<Panel*> panels;

    // First, create normal panels to fill the panel strip.
    int i = 0;
    for (; i < num_normal_panels; ++i) {
      CreatePanelParams params(
          MakePanelName(i),
          gfx::Rect(0, 0, panel_widths[i], kTestPanelHeight),
          SHOW_AS_INACTIVE);
      Panel* panel = CreatePanelWithParams(params);
      panels.push_back(panel);
    }

    // Then, create panels that would be placed in the overflow strip.
    int num_panels = num_normal_panels + num_overflow_panels;
    for (; i < num_panels; ++i) {
      CreatePanelParams params(
          MakePanelName(i),
          gfx::Rect(0, 0, panel_widths[i], kTestPanelHeight),
          SHOW_AS_INACTIVE);
      Panel* panel = CreatePanelWithParams(params);
      WaitForExpansionStateChanged(panel, Panel::IN_OVERFLOW);
      panels.push_back(panel);
    }

    return panels;
  }
};

// TODO(jianli): remove the guard when overflow support is enabled on other
// platforms. http://crbug.com/105073
#if defined(OS_WIN)
#define MAYBE_CheckPanelProperties CheckPanelProperties
#define MAYBE_UpdateDraggableStatus UpdateDraggableStatus
#define MAYBE_CreateOverflowPanels CreateOverflowPanels
#define MAYBE_CreateMoreOverflowPanels CreateMoreOverflowPanels
// http://crbug.com/107230
#define MAYBE_CreatePanelOnDelayedOverflow CreatePanelOnDelayedOverflow
#define MAYBE_CloseOverflowPanels CloseOverflowPanels
#define MAYBE_CloseNormalPanels CloseNormalPanels
#define MAYBE_CloseWithDelayedOverflow CloseWithDelayedOverflow
#define MAYBE_ActivateOverflowPanels ActivateOverflowPanels
#define MAYBE_HoverOverOverflowArea HoverOverOverflowArea
#define MAYBE_ResizePanel ResizePanel
#else
#define MAYBE_CheckPanelProperties DISABLED_CheckPanelProperties
#define MAYBE_UpdateDraggableStatus DISABLED_UpdateDraggableStatus
#define MAYBE_CreateOverflowPanels DISABLED_CreateOverflowPanels
#define MAYBE_CreateMoreOverflowPanels DISABLED_CreateMoreOverflowPanels
#define MAYBE_CreatePanelOnDelayedOverflow DISABLED_CreatePanelOnDelayedOverflow
#define MAYBE_CloseOverflowPanels DISABLED_CloseOverflowPanels
#define MAYBE_CloseNormalPanels DISABLED_CloseNormalPanels
#define MAYBE_CloseWithDelayedOverflow DISABLED_CloseWithDelayedOverflow
#define MAYBE_ActivateOverflowPanels DISABLED_ActivateOverflowPanels
#define MAYBE_HoverOverOverflowArea DISABLED_HoverOverOverflowArea
#define MAYBE_ResizePanel DISABLED_ResizePanel
#endif

IN_PROC_BROWSER_TEST_F(PanelOverflowBrowserTest, MAYBE_CheckPanelProperties) {
  // Create 3 panels that fit.
  Panel* panel1 = CreatePanelWithBounds("1", gfx::Rect(0, 0, 250, 200));
  Panel* panel2 = CreatePanelWithBounds("2", gfx::Rect(0, 0, 300, 200));
  Panel* panel3 = CreatePanelWithBounds("3", gfx::Rect(0, 0, 200, 200));

  // Create an overflow panel without waiting for it to be moved to overflow.
  // Check its properties before and after it is moved to overflow.
  CreatePanelParams params(
      "4", gfx::Rect(0, 0, 255, 200), SHOW_AS_INACTIVE);
  params.wait_for_fully_created = false;
  Panel* panel4 = CreatePanelWithParams(params);

  EXPECT_EQ(Panel::EXPANDED, panel1->expansion_state());
  EXPECT_EQ(Panel::EXPANDED, panel2->expansion_state());
  EXPECT_EQ(Panel::EXPANDED, panel3->expansion_state());
  EXPECT_EQ(Panel::EXPANDED, panel4->expansion_state());

  EXPECT_FALSE(panel1->has_temporary_layout());
  EXPECT_FALSE(panel2->has_temporary_layout());
  EXPECT_FALSE(panel3->has_temporary_layout());
  EXPECT_TRUE(panel4->has_temporary_layout());

  EXPECT_TRUE(panel1->draggable());
  EXPECT_TRUE(panel2->draggable());
  EXPECT_TRUE(panel3->draggable());
  EXPECT_FALSE(panel4->draggable());

  // Make sure last panel really did overflow.
  WaitForExpansionStateChanged(panel4, Panel::IN_OVERFLOW);
  EXPECT_FALSE(panel4->has_temporary_layout());
  EXPECT_FALSE(panel4->draggable());

  PanelManager::GetInstance()->RemoveAll();
}

IN_PROC_BROWSER_TEST_F(PanelOverflowBrowserTest, MAYBE_UpdateDraggableStatus) {
  Panel* panel = CreatePanel("panel");
  EXPECT_TRUE(panel->draggable());
  panel->SetExpansionState(Panel::IN_OVERFLOW);
  EXPECT_FALSE(panel->draggable());
  panel->SetExpansionState(Panel::EXPANDED);
  EXPECT_TRUE(panel->draggable());
  panel->Close();
}

IN_PROC_BROWSER_TEST_F(PanelOverflowBrowserTest, MAYBE_CreateOverflowPanels) {
  PanelManager* panel_manager = PanelManager::GetInstance();
  PanelStrip* panel_strip = panel_manager->panel_strip();
  PanelOverflowStrip* panel_overflow_strip =
      panel_manager->panel_overflow_strip();

  const int panel_widths[] = {
      250, 260, 200,  // normal
      255, 220        // overflow
  };
  CreateOverflowPanels(3, 2, panel_widths);

  std::vector<Panel*> panels = panel_manager->panels();
  ASSERT_EQ(5u, panels.size());
  EXPECT_EQ(3, panel_strip->num_panels());
  EXPECT_EQ(2, panel_overflow_strip->num_panels());
  EXPECT_EQ(Panel::IN_OVERFLOW, panels[3]->expansion_state());
  EXPECT_TRUE(IsPanelVisible(panels[3]));
  EXPECT_EQ(Panel::IN_OVERFLOW, panels[4]->expansion_state());
  EXPECT_TRUE(IsPanelVisible(panels[4]));

  PanelManager::GetInstance()->RemoveAll();
}

IN_PROC_BROWSER_TEST_F(PanelOverflowBrowserTest,
                       MAYBE_CreateMoreOverflowPanels) {
  PanelManager* panel_manager = PanelManager::GetInstance();
  PanelStrip* panel_strip = panel_manager->panel_strip();
  PanelOverflowStrip* panel_overflow_strip =
      panel_manager->panel_overflow_strip();

  const int panel_widths[] = {
      250, 260, 200,  // normal
      255, 220, 210,  // overflow
      220, 230        // overflow-on-overflow
  };
  CreateOverflowPanels(3, 5, panel_widths);

  std::vector<Panel*> panels = panel_manager->panels();
  ASSERT_EQ(8u, panels.size());
  EXPECT_EQ(3, panel_strip->num_panels());
  EXPECT_EQ(5, panel_overflow_strip->num_panels());
  EXPECT_EQ(Panel::IN_OVERFLOW, panels[3]->expansion_state());
  EXPECT_TRUE(IsPanelVisible(panels[3]));
  EXPECT_EQ(Panel::IN_OVERFLOW, panels[4]->expansion_state());
  EXPECT_TRUE(IsPanelVisible(panels[4]));
  EXPECT_EQ(Panel::IN_OVERFLOW, panels[5]->expansion_state());
  EXPECT_TRUE(IsPanelVisible(panels[5]));
  EXPECT_EQ(Panel::IN_OVERFLOW, panels[6]->expansion_state());
  EXPECT_FALSE(IsPanelVisible(panels[6]));
  EXPECT_EQ(Panel::IN_OVERFLOW, panels[7]->expansion_state());
  EXPECT_FALSE(IsPanelVisible(panels[7]));

  PanelManager::GetInstance()->RemoveAll();
}

IN_PROC_BROWSER_TEST_F(PanelOverflowBrowserTest,
                       MAYBE_CreatePanelOnDelayedOverflow) {
  // Create 2 big panels.
  CreatePanelWithBounds("Panel0", gfx::Rect(0, 0, 260, 200));
  CreatePanelWithBounds("Panel1", gfx::Rect(0, 0, 260, 200));

  // Create an overflow panel without waiting for it to be moved to overflow.
  CreatePanelParams params(
      "Panel2", gfx::Rect(0, 0, 255, 200), SHOW_AS_INACTIVE);
  params.wait_for_fully_created = false;
  Panel* overflow_panel = CreatePanelWithParams(params);
  EXPECT_EQ(Panel::EXPANDED, overflow_panel->expansion_state());
  EXPECT_TRUE(overflow_panel->has_temporary_layout());

  // Create a small panel that could fit within the available space in the
  // panel strip.
  CreatePanelParams params2(
      "Panel3", gfx::Rect(0, 0, 110, 200), SHOW_AS_INACTIVE);
  Panel* panel3 = CreatePanelWithParams(params2);
  EXPECT_EQ(Panel::EXPANDED, panel3->expansion_state());
  EXPECT_FALSE(panel3->has_temporary_layout());

  WaitForExpansionStateChanged(overflow_panel, Panel::IN_OVERFLOW);
  EXPECT_FALSE(overflow_panel->has_temporary_layout());
  PanelManager::GetInstance()->RemoveAll();
}

IN_PROC_BROWSER_TEST_F(PanelOverflowBrowserTest, MAYBE_CloseOverflowPanels) {
  PanelManager* panel_manager = PanelManager::GetInstance();
  PanelStrip* panel_strip = panel_manager->panel_strip();
  PanelOverflowStrip* panel_overflow_strip =
      panel_manager->panel_overflow_strip();

  // Create normal and overflow panels.
  //   normal:               P0, P1, P2
  //   overflow:             P3, P4, P5
  //   overflow-on-overflow: P6, P7
  int num_normal_panels = 3;
  int num_overflow_panels = 5;
  const int panel_widths[] = {
      260, 250, 200,  // normal
      255, 260, 120,  // overflow
      240, 210        // overflow-on-overflow
  };
  std::vector<Panel*> panels = CreateOverflowPanels(
      num_normal_panels, num_overflow_panels, panel_widths);

  PanelDataList expected_normal_list;
  expected_normal_list.Add(panels[0], Panel::EXPANDED, true, false);
  expected_normal_list.Add(panels[1], Panel::EXPANDED, true, false);
  expected_normal_list.Add(panels[2], Panel::EXPANDED, true, false);
  EXPECT_EQ(expected_normal_list, GetAllNormalPanelData());

  PanelDataList expected_overflow_list;
  expected_overflow_list.Add(panels[3], Panel::IN_OVERFLOW, true, false);
  expected_overflow_list.Add(panels[4], Panel::IN_OVERFLOW, true, false);
  expected_overflow_list.Add(panels[5], Panel::IN_OVERFLOW, true, false);
  expected_overflow_list.Add(panels[6], Panel::IN_OVERFLOW, false, false);
  expected_overflow_list.Add(panels[7], Panel::IN_OVERFLOW, false, false);
  EXPECT_EQ(expected_overflow_list, GetAllOverflowPanelData());

  // Close an overflow-on-overflow panel. Expect only that panel is closed.
  //   normal:               P0, P1, P2
  //   overflow:             P3, P4, P5,
  //   overflow-on-overflow: P7
  CloseWindowAndWait(panels[6]->browser());
  num_overflow_panels--;
  ASSERT_EQ(num_normal_panels + num_overflow_panels,
            panel_manager->num_panels());
  EXPECT_EQ(num_normal_panels, panel_strip->num_panels());
  EXPECT_EQ(num_overflow_panels, panel_overflow_strip->num_panels());

  EXPECT_EQ(expected_normal_list, GetAllNormalPanelData());

  expected_overflow_list.clear();
  expected_overflow_list.Add(panels[3], Panel::IN_OVERFLOW, true, false);
  expected_overflow_list.Add(panels[4], Panel::IN_OVERFLOW, true, false);
  expected_overflow_list.Add(panels[5], Panel::IN_OVERFLOW, true, false);
  expected_overflow_list.Add(panels[7], Panel::IN_OVERFLOW, false, false);
  EXPECT_EQ(expected_overflow_list, GetAllOverflowPanelData());

  // Close an overflow panel. Expect an overflow-on-overflow panel to become
  // visible in the overflow strip.
  //   normal:               P0, P1, P2
  //   overflow:             P3, P5, P7
  CloseWindowAndWait(panels[4]->browser());
  num_overflow_panels--;
  ASSERT_EQ(num_normal_panels + num_overflow_panels,
            panel_manager->num_panels());
  EXPECT_EQ(num_normal_panels, panel_strip->num_panels());
  EXPECT_EQ(num_overflow_panels, panel_overflow_strip->num_panels());

  EXPECT_EQ(expected_normal_list, GetAllNormalPanelData());

  expected_overflow_list.clear();
  expected_overflow_list.Add(panels[3], Panel::IN_OVERFLOW, true, false);
  expected_overflow_list.Add(panels[5], Panel::IN_OVERFLOW, true, false);
  expected_overflow_list.Add(panels[7], Panel::IN_OVERFLOW, true, false);
  EXPECT_EQ(expected_overflow_list, GetAllOverflowPanelData());

  // Close an overflow panel. Expect only that panel is closed.
  //   normal:               P0, P1, P2
  //   overflow:             P5, P7
  CloseWindowAndWait(panels[3]->browser());
  num_overflow_panels--;
  ASSERT_EQ(num_normal_panels + num_overflow_panels,
            panel_manager->num_panels());
  EXPECT_EQ(num_normal_panels, panel_strip->num_panels());
  EXPECT_EQ(num_overflow_panels, panel_overflow_strip->num_panels());

  EXPECT_EQ(expected_normal_list, GetAllNormalPanelData());

  expected_overflow_list.clear();
  expected_overflow_list.Add(panels[5], Panel::IN_OVERFLOW, true, false);
  expected_overflow_list.Add(panels[7], Panel::IN_OVERFLOW, true, false);
  EXPECT_EQ(expected_overflow_list, GetAllOverflowPanelData());

  panel_manager->RemoveAll();
}

IN_PROC_BROWSER_TEST_F(PanelOverflowBrowserTest, MAYBE_CloseNormalPanels) {
  PanelManager* panel_manager = PanelManager::GetInstance();
  PanelStrip* panel_strip = panel_manager->panel_strip();
  PanelOverflowStrip* panel_overflow_strip =
      panel_manager->panel_overflow_strip();

  // Create normal and overflow panels.
  //   normal:               P0, P1, P2
  //   overflow:             P3, P4, P5
  //   overflow-on-overflow: P6, P7, P8
  int num_normal_panels = 3;
  int num_overflow_panels = 6;
  const int panel_widths[] = {
      260, 250, 200,  // normal
      255, 260, 120,  // overflow
      240, 210, 258   // overflow-on-overflow
  };
  std::vector<Panel*> panels = CreateOverflowPanels(
      num_normal_panels, num_overflow_panels, panel_widths);

  PanelDataList expected_normal_list;
  expected_normal_list.Add(panels[0], Panel::EXPANDED, true, false);
  expected_normal_list.Add(panels[1], Panel::EXPANDED, true, false);
  expected_normal_list.Add(panels[2], Panel::EXPANDED, true, false);
  EXPECT_EQ(expected_normal_list, GetAllNormalPanelData());

  PanelDataList expected_overflow_list;
  expected_overflow_list.Add(panels[3], Panel::IN_OVERFLOW, true, false);
  expected_overflow_list.Add(panels[4], Panel::IN_OVERFLOW, true, false);
  expected_overflow_list.Add(panels[5], Panel::IN_OVERFLOW, true, false);
  expected_overflow_list.Add(panels[6], Panel::IN_OVERFLOW, false, false);
  expected_overflow_list.Add(panels[7], Panel::IN_OVERFLOW, false, false);
  expected_overflow_list.Add(panels[8], Panel::IN_OVERFLOW, false, false);
  EXPECT_EQ(expected_overflow_list, GetAllOverflowPanelData());

  // Close a normal panel. Expect an overflow panel to move over and an
  // overflow-on-overflow panel to become visible.
  //   normal:               P0, P2, P3
  //   overflow:             P4, P5, P6
  //   overflow-on-overflow: P7, P8
  CloseWindowAndWait(panels[1]->browser());
  num_overflow_panels--;
  ASSERT_EQ(num_normal_panels + num_overflow_panels,
            panel_manager->num_panels());
  EXPECT_EQ(num_normal_panels, panel_strip->num_panels());
  EXPECT_EQ(num_overflow_panels, panel_overflow_strip->num_panels());

  expected_normal_list.clear();
  expected_normal_list.Add(panels[0], Panel::EXPANDED, true, false);
  expected_normal_list.Add(panels[2], Panel::EXPANDED, true, false);
  expected_normal_list.Add(panels[3], Panel::EXPANDED, true, false);
  EXPECT_EQ(expected_normal_list, GetAllNormalPanelData());

  expected_overflow_list.clear();
  expected_overflow_list.Add(panels[4], Panel::IN_OVERFLOW, true, false);
  expected_overflow_list.Add(panels[5], Panel::IN_OVERFLOW, true, false);
  expected_overflow_list.Add(panels[6], Panel::IN_OVERFLOW, true, false);
  expected_overflow_list.Add(panels[7], Panel::IN_OVERFLOW, false, false);
  expected_overflow_list.Add(panels[8], Panel::IN_OVERFLOW, false, false);
  EXPECT_EQ(expected_overflow_list, GetAllOverflowPanelData());

  // Close another normal panel. Remaining overflow panels cannot move over
  // due to not enough room.
  //   normal:               P0, P3
  //   overflow:             P4, P5, P6
  //   overflow-on-overflow: P7, P8
  CloseWindowAndWait(panels[2]->browser());
  num_normal_panels--;
  ASSERT_EQ(num_normal_panels + num_overflow_panels,
            panel_manager->num_panels());
  EXPECT_EQ(num_normal_panels, panel_strip->num_panels());
  EXPECT_EQ(num_overflow_panels, panel_overflow_strip->num_panels());

  expected_normal_list.clear();
  expected_normal_list.Add(panels[0], Panel::EXPANDED, true, false);
  expected_normal_list.Add(panels[3], Panel::EXPANDED, true, false);
  EXPECT_EQ(expected_normal_list, GetAllNormalPanelData());

  EXPECT_EQ(expected_overflow_list, GetAllOverflowPanelData());

  // Close one more normal panel. Expect two overflow panels to move over and
  // two overflow-on-overflow panels to become visible.
  //   normal:               P0, P4, P5
  //   overflow:             P6, P7, P8
  CloseWindowAndWait(panels[3]->browser());
  num_normal_panels++;
  num_overflow_panels -= 2;
  ASSERT_EQ(num_normal_panels + num_overflow_panels,
            panel_manager->num_panels());
  EXPECT_EQ(num_normal_panels, panel_strip->num_panels());
  EXPECT_EQ(num_overflow_panels, panel_overflow_strip->num_panels());

  expected_normal_list.clear();
  expected_normal_list.Add(panels[0], Panel::EXPANDED, true, false);
  expected_normal_list.Add(panels[4], Panel::EXPANDED, true, false);
  expected_normal_list.Add(panels[5], Panel::EXPANDED, true, false);
  EXPECT_EQ(expected_normal_list, GetAllNormalPanelData());

  expected_overflow_list.clear();
  expected_overflow_list.Add(panels[6], Panel::IN_OVERFLOW, true, false);
  expected_overflow_list.Add(panels[7], Panel::IN_OVERFLOW, true, false);
  expected_overflow_list.Add(panels[8], Panel::IN_OVERFLOW, true, false);
  EXPECT_EQ(expected_overflow_list, GetAllOverflowPanelData());

  // Close another normal panel. Expect one overflow panel to move over.
  //   normal:               P4, P5, P6
  //   overflow:             P7, P8
  CloseWindowAndWait(panels[0]->browser());
  num_overflow_panels--;
  ASSERT_EQ(num_normal_panels + num_overflow_panels,
            panel_manager->num_panels());
  EXPECT_EQ(num_normal_panels, panel_strip->num_panels());
  EXPECT_EQ(num_overflow_panels, panel_overflow_strip->num_panels());

  expected_normal_list.clear();
  expected_normal_list.Add(panels[4], Panel::EXPANDED, true, false);
  expected_normal_list.Add(panels[5], Panel::EXPANDED, true, false);
  expected_normal_list.Add(panels[6], Panel::EXPANDED, true, false);
  EXPECT_EQ(expected_normal_list, GetAllNormalPanelData());

  expected_overflow_list.clear();
  expected_overflow_list.Add(panels[7], Panel::IN_OVERFLOW, true, false);
  expected_overflow_list.Add(panels[8], Panel::IN_OVERFLOW, true, false);
  EXPECT_EQ(expected_overflow_list, GetAllOverflowPanelData());

  panel_manager->RemoveAll();
}

IN_PROC_BROWSER_TEST_F(PanelOverflowBrowserTest,
                       MAYBE_CloseWithDelayedOverflow) {
  PanelManager* panel_manager = PanelManager::GetInstance();
  PanelStrip* panel_strip = panel_manager->panel_strip();

  // Create 2 big panels.
  Panel* panel0 = CreatePanelWithBounds("Panel0", gfx::Rect(0, 0, 260, 200));
  Panel* panel1 = CreatePanelWithBounds("Panel1", gfx::Rect(0, 0, 260, 200));

  // Create an overflow panel without waiting for it to be moved to overflow.
  CreatePanelParams params(
      "Panel2", gfx::Rect(0, 0, 255, 200), SHOW_AS_INACTIVE);
  params.wait_for_fully_created = false;
  Panel* overflow_panel = CreatePanelWithParams(params);
  EXPECT_EQ(2, panel_strip->num_panels());
  EXPECT_EQ(1, panel_strip->num_temporary_layout_panels());
  EXPECT_TRUE(overflow_panel->has_temporary_layout());

  // Close the overflow panel while it is still in temporary layout.
  CloseWindowAndWait(overflow_panel->browser());
  EXPECT_EQ(2, panel_strip->num_panels());
  EXPECT_EQ(0, panel_strip->num_temporary_layout_panels());

  // Create another overflow panel without waiting for it to move to overflow.
  params.name = "Panel3";
  overflow_panel = CreatePanelWithParams(params);
  EXPECT_EQ(2, panel_strip->num_panels());
  EXPECT_EQ(1, panel_strip->num_temporary_layout_panels());
  EXPECT_TRUE(overflow_panel->has_temporary_layout());

  // Close one of the non-overflow panels. Expect the delayed overflow
  // panel will not be affected by the close.
  // Hack. Pretend to close panel by removing it directly. Cannot use
  // CloseWindowAndWait() here because it will allow the delayed overflow
  // to complete.
  panel_strip->Remove(panel1);
  EXPECT_EQ(1, panel_strip->num_panels());
  EXPECT_EQ(1, panel_strip->num_temporary_layout_panels());
  EXPECT_TRUE(overflow_panel->has_temporary_layout());

  // Make sure the overflow panel actually moves to overflow.
  WaitForExpansionStateChanged(overflow_panel, Panel::IN_OVERFLOW);
  EXPECT_EQ(0, panel_strip->num_temporary_layout_panels());

  // Hack. Put the "falsely closed" panel back into the panel strip
  // so we can properly close it to wrap up this test.
  panel_strip->AddPanel(panel1);

  panel0->Close();
  panel1->Close();
  overflow_panel->Close();
}

IN_PROC_BROWSER_TEST_F(PanelOverflowBrowserTest, MAYBE_ActivateOverflowPanels) {
  // Create normal and overflow panels.
  //   normal:               P0, P1, P2
  //   overflow:             P3, P4, P5
  //   overflow-on-overflow: P6, P7
  const int panel_widths[] = {
      250, 260, 200,  // normal
      210, 260, 230,  // overflow
      255, 210        // overflow-on-overflow
  };
  std::vector<Panel*> panels = CreateOverflowPanels(3, 5, panel_widths);

  PanelDataList expected_normal_list;
  expected_normal_list.Add(panels[0], Panel::EXPANDED, true, false);
  expected_normal_list.Add(panels[1], Panel::EXPANDED, true, false);
  expected_normal_list.Add(panels[2], Panel::EXPANDED, true, false);
  EXPECT_EQ(expected_normal_list, GetAllNormalPanelData());

  PanelDataList expected_overflow_list;
  expected_overflow_list.Add(panels[3], Panel::IN_OVERFLOW, true, false);
  expected_overflow_list.Add(panels[4], Panel::IN_OVERFLOW, true, false);
  expected_overflow_list.Add(panels[5], Panel::IN_OVERFLOW, true, false);
  expected_overflow_list.Add(panels[6], Panel::IN_OVERFLOW, false, false);
  expected_overflow_list.Add(panels[7], Panel::IN_OVERFLOW, false, false);
  EXPECT_EQ(expected_overflow_list, GetAllOverflowPanelData());

  // Activate an overflow panel. Expect one normal panel is swapped into the
  // overflow strip.
  //   normal:               P0, P1, P3
  //   overflow:             P2, P4, P5
  //   overflow-on-overflow: P6, P7
  panels[3]->Activate();
  WaitForPanelActiveState(panels[3], SHOW_AS_ACTIVE);
  WaitForExpansionStateChanged(panels[3], Panel::EXPANDED);

  expected_normal_list.clear();
  expected_normal_list.Add(panels[0], Panel::EXPANDED, true, false);
  expected_normal_list.Add(panels[1], Panel::EXPANDED, true, false);
  expected_normal_list.Add(panels[3], Panel::EXPANDED, true, true);
  EXPECT_EQ(expected_normal_list, GetAllNormalPanelData());

  expected_overflow_list.clear();
  expected_overflow_list.Add(panels[2], Panel::IN_OVERFLOW, true, false);
  expected_overflow_list.Add(panels[4], Panel::IN_OVERFLOW, true, false);
  expected_overflow_list.Add(panels[5], Panel::IN_OVERFLOW, true, false);
  expected_overflow_list.Add(panels[6], Panel::IN_OVERFLOW, false, false);
  expected_overflow_list.Add(panels[7], Panel::IN_OVERFLOW, false, false);
  EXPECT_EQ(expected_overflow_list, GetAllOverflowPanelData());

  // Activate an overflow panel. Expect two normal panels are swapped into the
  // overflow strip and one overflow panel to become hidden.
  //   normal:               P0, P4
  //   overflow:             P1, P3, P2
  //   overflow-on-overflow: P5, P6, P7
  panels[4]->Activate();
  WaitForPanelActiveState(panels[4], SHOW_AS_ACTIVE);
  WaitForExpansionStateChanged(panels[4], Panel::EXPANDED);
  WaitForPanelActiveState(panels[3], SHOW_AS_INACTIVE);

  expected_normal_list.clear();
  expected_normal_list.Add(panels[0], Panel::EXPANDED, true, false);
  expected_normal_list.Add(panels[4], Panel::EXPANDED, true, true);
  EXPECT_EQ(expected_normal_list, GetAllNormalPanelData());

  expected_overflow_list.clear();
  expected_overflow_list.Add(panels[1], Panel::IN_OVERFLOW, true, false);
  expected_overflow_list.Add(panels[3], Panel::IN_OVERFLOW, true, false);
  expected_overflow_list.Add(panels[2], Panel::IN_OVERFLOW, true, false);
  expected_overflow_list.Add(panels[5], Panel::IN_OVERFLOW, false, false);
  expected_overflow_list.Add(panels[6], Panel::IN_OVERFLOW, false, false);
  expected_overflow_list.Add(panels[7], Panel::IN_OVERFLOW, false, false);
  EXPECT_EQ(expected_overflow_list, GetAllOverflowPanelData());

  // Activate an overflow-on-overflow panel. Expect one normal panel is swapped
  // into the overflow strip.
  //   normal:               P0, P6
  //   overflow:             P4, P1, P3,
  //   overflow-on-overflow: P2, P5, P7
  panels[6]->Activate();
  WaitForPanelActiveState(panels[6], SHOW_AS_ACTIVE);
  WaitForExpansionStateChanged(panels[6], Panel::EXPANDED);
  WaitForPanelActiveState(panels[4], SHOW_AS_INACTIVE);

  expected_normal_list.clear();
  expected_normal_list.Add(panels[0], Panel::EXPANDED, true, false);
  expected_normal_list.Add(panels[6], Panel::EXPANDED, true, true);
  EXPECT_EQ(expected_normal_list, GetAllNormalPanelData());

  expected_overflow_list.clear();
  expected_overflow_list.Add(panels[4], Panel::IN_OVERFLOW, true, false);
  expected_overflow_list.Add(panels[1], Panel::IN_OVERFLOW, true, false);
  expected_overflow_list.Add(panels[3], Panel::IN_OVERFLOW, true, false);
  expected_overflow_list.Add(panels[2], Panel::IN_OVERFLOW, false, false);
  expected_overflow_list.Add(panels[5], Panel::IN_OVERFLOW, false, false);
  expected_overflow_list.Add(panels[7], Panel::IN_OVERFLOW, false, false);
  EXPECT_EQ(expected_overflow_list, GetAllOverflowPanelData());

  // Activate an overflow-on-overflow panel. No normal panel is swapped
  // since there has already been enough space in the panel strip.
  //   normal:               P0, P6, P7
  //   overflow:             P4, P1, P3,
  //   overflow-on-overflow: P2, P5
  panels[7]->Activate();
  WaitForPanelActiveState(panels[7], SHOW_AS_ACTIVE);
  WaitForExpansionStateChanged(panels[7], Panel::EXPANDED);
  WaitForPanelActiveState(panels[6], SHOW_AS_INACTIVE);

  expected_normal_list.clear();
  expected_normal_list.Add(panels[0], Panel::EXPANDED, true, false);
  expected_normal_list.Add(panels[6], Panel::EXPANDED, true, false);
  expected_normal_list.Add(panels[7], Panel::EXPANDED, true, true);
  EXPECT_EQ(expected_normal_list, GetAllNormalPanelData());

  expected_overflow_list.clear();
  expected_overflow_list.Add(panels[4], Panel::IN_OVERFLOW, true, false);
  expected_overflow_list.Add(panels[1], Panel::IN_OVERFLOW, true, false);
  expected_overflow_list.Add(panels[3], Panel::IN_OVERFLOW, true, false);
  expected_overflow_list.Add(panels[2], Panel::IN_OVERFLOW, false, false);
  expected_overflow_list.Add(panels[5], Panel::IN_OVERFLOW, false, false);
  EXPECT_EQ(expected_overflow_list, GetAllOverflowPanelData());

  PanelManager::GetInstance()->RemoveAll();
}

IN_PROC_BROWSER_TEST_F(PanelOverflowBrowserTest, MAYBE_HoverOverOverflowArea) {
  PanelManager* panel_manager = PanelManager::GetInstance();
  PanelMouseWatcher* mouse_watcher = new TestPanelMouseWatcher();
  panel_manager->SetMouseWatcherForTesting(mouse_watcher);
  PanelOverflowStrip* panel_overflow_strip =
      panel_manager->panel_overflow_strip();
  int iconified_width = panel_overflow_strip->current_display_width();

  // Create normal and overflow panels.
  //   normal:               P0, P1, P2
  //   overflow:             P3, P4, P5
  //   overflow-on-overflow: P6, P7
  const int panel_widths[] = {
      250, 260, 200,  // normal
      255, 220, 260,  // overflow
      140, 210        // overflow-on-overflow
  };
  std::vector<Panel*> panels = CreateOverflowPanels(3, 5, panel_widths);

  // Move mouse beyond the right edge of the top overflow panel.
  // Expect the overflow area remains shrunk.
  MoveMouse(gfx::Point(panels[5]->GetBounds().right() + 1,
                       panels[5]->GetBounds().y()));
  EXPECT_EQ(iconified_width, panel_overflow_strip->current_display_width());

  // Move mouse above the top overflow panel. Expect the overflow area
  // remains shrunk.
  MoveMouse(gfx::Point(panels[5]->GetBounds().x(),
                       panels[5]->GetBounds().y() - 1));
  EXPECT_EQ(iconified_width, panel_overflow_strip->current_display_width());

  // Move mouse below the bottom overflow panel. Expect the overflow area
  // remains shrunk.
  MoveMouse(gfx::Point(panels[3]->GetBounds().right(),
                       panels[3]->GetBounds().bottom() + 1));
  EXPECT_EQ(iconified_width, panel_overflow_strip->current_display_width());

  // Move mouse to the origin of an overflow panel. Expect the overflow area
  // gets expanded.
  MoveMouseAndWaitForOverflowAnimationEnded(
      panels[4]->GetBounds().origin());
  int hover_width = panel_overflow_strip->current_display_width();
  EXPECT_GT(hover_width, iconified_width);
  EXPECT_EQ(hover_width, panels[3]->GetBounds().width());
  EXPECT_EQ(hover_width, panels[4]->GetBounds().width());
  EXPECT_EQ(hover_width, panels[5]->GetBounds().width());
  EXPECT_EQ(hover_width, panels[6]->GetBounds().width());
  EXPECT_EQ(hover_width, panels[7]->GetBounds().width());

  // Move mouse to the origin of another overflow panel. Expect the overflow
  // area remains expanded.
  MoveMouse(panels[5]->GetBounds().origin());
  EXPECT_EQ(hover_width, panel_overflow_strip->current_display_width());

  // Move mouse beyond the left edge of an overflow panel. Expect the overflow
  // area remains expanded.
  MoveMouse(gfx::Point(panels[5]->GetBounds().x() - 5,
                       panels[5]->GetBounds().y()));
  EXPECT_EQ(hover_width, panel_overflow_strip->current_display_width());

  // Move mouse to the origin of the top overflow-on-overflow panel. Expect the
  // overflow area remains expanded.
  MoveMouse(panels[7]->GetBounds().origin());
  EXPECT_EQ(hover_width, panel_overflow_strip->current_display_width());

  // Move mouse to the bottom-right corner of the bottom overflow panel.
  // Expect the overflow area remains expanded.
  MoveMouse(gfx::Point(panels[3]->GetBounds().right(),
                       panels[3]->GetBounds().bottom()));
  EXPECT_EQ(hover_width, panel_overflow_strip->current_display_width());

  // Move mouse beyond the right edge of the hover panel. Expect the overflow
  // area gets shrunk.
  MoveMouseAndWaitForOverflowAnimationEnded(gfx::Point(
      panels[4]->GetBounds().right() + 5, panels[4]->GetBounds().y()));
  EXPECT_EQ(iconified_width, panel_overflow_strip->current_display_width());
  EXPECT_EQ(iconified_width, panels[3]->GetBounds().width());
  EXPECT_EQ(iconified_width, panels[4]->GetBounds().width());
  EXPECT_EQ(iconified_width, panels[5]->GetBounds().width());
  EXPECT_EQ(0, panels[6]->GetBounds().width());
  EXPECT_EQ(0, panels[7]->GetBounds().width());

  // Move mouse beyond the left edge of an overflow panel. Expect the overflow
  // area gets expanded.
  MoveMouseAndWaitForOverflowAnimationEnded(gfx::Point(
      panels[4]->GetBounds().x() - 5, panels[4]->GetBounds().y()));
  EXPECT_EQ(hover_width, panel_overflow_strip->current_display_width());
  EXPECT_EQ(hover_width, panels[3]->GetBounds().width());
  EXPECT_EQ(hover_width, panels[4]->GetBounds().width());
  EXPECT_EQ(hover_width, panels[5]->GetBounds().width());
  EXPECT_EQ(hover_width, panels[6]->GetBounds().width());
  EXPECT_EQ(hover_width, panels[7]->GetBounds().width());

  // Move mouse above the top overflow-on-overflow panel. Expect the overflow
  // area gets shrunk.
  MoveMouseAndWaitForOverflowAnimationEnded(gfx::Point(
      panels[7]->GetBounds().x(), panels[7]->GetBounds().y() - 1));
  EXPECT_EQ(iconified_width, panel_overflow_strip->current_display_width());
  EXPECT_EQ(iconified_width, panels[3]->GetBounds().width());
  EXPECT_EQ(iconified_width, panels[4]->GetBounds().width());
  EXPECT_EQ(iconified_width, panels[5]->GetBounds().width());
  EXPECT_EQ(0, panels[6]->GetBounds().width());
  EXPECT_EQ(0, panels[7]->GetBounds().width());

  panel_manager->RemoveAll();
}

IN_PROC_BROWSER_TEST_F(PanelOverflowBrowserTest, MAYBE_ResizePanel) {
  PanelManager* panel_manager = PanelManager::GetInstance();
  panel_manager->enable_auto_sizing(true);
  PanelStrip* panel_strip = panel_manager->panel_strip();
  PanelOverflowStrip* overflow_strip = panel_manager->panel_overflow_strip();

  // Create 4 panels that fit.
  // normal: P1 (250), P2 (200), P3 (100), P4 (100)
  // overflow: empty
  Panel* panel1 = CreatePanelWithBounds("1", gfx::Rect(0, 0, 250, 200));
  Panel* panel2 = CreatePanelWithBounds("2", gfx::Rect(0, 0, 200, 200));
  Panel* panel3 = CreatePanelWithBounds("3", gfx::Rect(0, 0, 100, 200));
  Panel* panel4 = CreatePanelWithBounds("4", gfx::Rect(0, 0, 100, 200));
  EXPECT_EQ(4, panel_strip->num_panels());
  EXPECT_EQ(0, overflow_strip->num_panels());

  // Resize last panel so that it is too big to fit and overflows.
  // normal: P1 (250), P2 (200), P3 (100)
  // overflow: P4 (250)*
  panel_manager->OnPreferredWindowSizeChanged(panel4, gfx::Size(250, 200));
  EXPECT_EQ(3, panel_strip->num_panels());
  EXPECT_EQ(1, overflow_strip->num_panels());
  EXPECT_EQ(Panel::IN_OVERFLOW, panel4->expansion_state());

  // Open another panel that will fit.
  // normal: P1 (250), P2 (200), P3 (100), P5 (100)*
  // overflow: P4 (250)
  Panel* panel5 = CreatePanelWithBounds("5", gfx::Rect(0, 0, 100, 200));
  EXPECT_EQ(4, panel_strip->num_panels());
  EXPECT_EQ(1, overflow_strip->num_panels());
  EXPECT_EQ(Panel::EXPANDED, panel5->expansion_state());
  EXPECT_EQ(Panel::IN_OVERFLOW, panel4->expansion_state());  // no change

  // Resize a panel from the middle of the strip so that it causes a
  // panel to overflow.
  // normal: P1 (250), P2 (200), P3 (250)*
  // overflow: P5 (100), P4 (250)
  panel_manager->OnPreferredWindowSizeChanged(panel3, gfx::Size(250, 200));
  EXPECT_EQ(3, panel_strip->num_panels());
  EXPECT_EQ(2, overflow_strip->num_panels());
  EXPECT_EQ(Panel::IN_OVERFLOW, panel4->expansion_state());
  EXPECT_EQ(Panel::IN_OVERFLOW, panel5->expansion_state());
  const PanelOverflowStrip::Panels& overflow = overflow_strip->panels();
  EXPECT_EQ(panel5, overflow[0]);  // new overflow panel is first
  EXPECT_EQ(panel4, overflow[1]);

  // Resize panel smaller so that panel from overflow can fit.
  // normal: P1 (250), P2 (200), P3 (100)*, P5 (100)
  // overflow: P4 (250)
  panel_manager->OnPreferredWindowSizeChanged(panel3, gfx::Size(100, 200));
  EXPECT_EQ(4, panel_strip->num_panels());
  EXPECT_EQ(1, overflow_strip->num_panels());
  EXPECT_EQ(Panel::EXPANDED, panel5->expansion_state());
  EXPECT_EQ(Panel::IN_OVERFLOW, panel4->expansion_state());

  // Resize smaller again but not small enough to fit overflow panel.
  // normal: P1 (250), P2 (100)*, P3 (100), P5 (100)
  // overflow: P4 (250)
  panel_manager->OnPreferredWindowSizeChanged(panel2, gfx::Size(100, 200));
  EXPECT_EQ(4, panel_strip->num_panels());
  EXPECT_EQ(1, overflow_strip->num_panels());
  EXPECT_EQ(Panel::IN_OVERFLOW, panel4->expansion_state());  // no change

  // Resize overflow panel to make it fit.
  // normal: P1 (250), P2 (100), P3 (100), P5 (100), P4 (100)*
  // overflow: empty
  panel_manager->OnPreferredWindowSizeChanged(panel4, gfx::Size(100, 200));
  EXPECT_EQ(5, panel_strip->num_panels());
  EXPECT_EQ(0, overflow_strip->num_panels());
  EXPECT_EQ(Panel::EXPANDED, panel4->expansion_state());

  // Resize a panel bigger, but not enough to cause overflow.
  // normal: P1 (250), P2 (100), P3 (150)*, P5 (100), P4 (100)
  // overflow: empty
  panel_manager->OnPreferredWindowSizeChanged(panel3, gfx::Size(150, 200));
  EXPECT_EQ(5, panel_strip->num_panels());
  EXPECT_EQ(0, overflow_strip->num_panels());

  // Resize a panel to bump more than one panel to overflow.
  // normal: P1 (250), P2 (250)*, P3 (150)
  // overflow: P5 (100), P4 (100)
  panel_manager->OnPreferredWindowSizeChanged(panel2, gfx::Size(250, 200));
  EXPECT_EQ(3, panel_strip->num_panels());
  EXPECT_EQ(2, overflow_strip->num_panels());
  EXPECT_EQ(Panel::IN_OVERFLOW, panel4->expansion_state());
  EXPECT_EQ(Panel::IN_OVERFLOW, panel5->expansion_state());
  const PanelOverflowStrip::Panels& overflow2 = overflow_strip->panels();
  EXPECT_EQ(panel5, overflow2[0]);  // strip order is preserved
  EXPECT_EQ(panel4, overflow2[1]);

  panel1->Close();
  panel2->Close();
  panel3->Close();
  panel4->Close();
  panel5->Close();
}
