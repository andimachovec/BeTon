#include "SimpleColumnView.h"
#include "Debug.h"

#include <Font.h>
#include <ScrollBar.h>
#include <Window.h>

#include <algorithm>
#include <cmath>
#include <cstdio>

/**
 * @brief Constructor.
 * @param name The name of the view.
 */
SimpleColumnView::SimpleColumnView(const char *name)
    : BView(BRect(0, 0, 1, 1), name, B_FOLLOW_ALL,
            B_WILL_DRAW | B_FRAME_EVENTS),
      fCurrentSelection(-1), fUseCustomColor(false) {
  font_height fh;
  be_plain_font->GetHeight(&fh);
  float fontHeight = fh.ascent + fh.descent + fh.leading;
  fItemHeight = ceilf(fontHeight * 1.4f);

  SetViewColor(B_TRANSPARENT_COLOR);
  fSelectionColor = ui_color(B_LIST_SELECTED_BACKGROUND_COLOR);
}

/**
 * @brief Adds an item with the given text.
 * @param text The display text for the item.
 */
void SimpleColumnView::AddItem(const BString &text) {
  AddItem(text, "");
  UpdateScrollbars();
  Invalidate();
}

/**
 * @brief Adds an item with text and an associated path/value.
 * @param text The display text.
 * @param path The hidden value/path associated with the item.
 */
void SimpleColumnView::AddItem(const BString &text, const BString &path) {
  fItems.push_back({text, path, false});
  UpdateScrollbars();
  Invalidate();
}

/**
 * @brief Removes all items from the list.
 */
void SimpleColumnView::Clear() {
  fItems.clear();
  fCurrentSelection = -1;
  UpdateScrollbars();
  Invalidate();
}

/**
 * @return The number of items in the list.
 */
int32 SimpleColumnView::CountItems() const { return (int32)fItems.size(); }

/**
 * @return The display text of the item at the specified index.
 */
const BString &SimpleColumnView::ItemAt(int32 index) const {
  static BString empty("");
  if (index < 0 || index >= (int32)fItems.size())
    return empty;
  return fItems[index].text;
}

/**
 * @return The hidden path/value of the item at the specified index.
 */
const BString &SimpleColumnView::PathAt(int32 index) const {
  static BString empty("");
  if (index < 0 || index >= (int32)fItems.size())
    return empty;
  return fItems[index].path;
}

/**
 * @return The index of the currently selected item, or -1 if none.
 */
int32 SimpleColumnView::CurrentSelection() const { return fCurrentSelection; }

/**
 * @brief Removes the item at the specified index.
 */
void SimpleColumnView::RemoveItemAt(int32 index) {
  if (index >= 0 && index < (int32)fItems.size()) {
    fItems.erase(fItems.begin() + index);
    Invalidate();
  }
}

/**
 * @brief Selects the item at the specified index.
 * @param index The index to select.
 */
void SimpleColumnView::Select(int32 index) {
  if (index < 0 || index >= (int32)fItems.size())
    return;
  if (fCurrentSelection >= 0 && fCurrentSelection < (int32)fItems.size())
    fItems[fCurrentSelection].selected = false;

  fCurrentSelection = index;
  fItems[index].selected = true;
  Invalidate();
}

/**
 * @brief Scrolls the view to ensure the selected item is visible.
 */
void SimpleColumnView::ScrollToSelection() {
  if (fCurrentSelection < 0 || fCurrentSelection >= (int32)fItems.size())
    return;

  float y = fCurrentSelection * fItemHeight;
  BRect bounds = Bounds();

  float viewHeight = bounds.Height();

  float targetY = y - (viewHeight - fItemHeight) / 2.0f;

  if (targetY < 0)
    targetY = 0;

  ScrollTo(0, targetY);
}

/**
 * @brief Updates scrollbar range and proportion based on item count.
 */
void SimpleColumnView::UpdateScrollbars() {
  const float lh = fItemHeight;
  const float contentHeight = std::max(1.0f, (float)fItems.size() * lh);

  float viewHeight = Bounds().Height();
  if (Parent())
    viewHeight = Parent()->Bounds().Height();

  if (BScrollBar *sb = ScrollBar(B_VERTICAL)) {
    float max = contentHeight - viewHeight;
    if (max < 0)
      max = 0;

    sb->SetRange(0.0f, max);

    float val = sb->Value();
    if (val > max)
      sb->SetValue(max);
    if (val < 0)
      sb->SetValue(0);

    float proportion = (contentHeight > 0.0f)
                           ? std::min(1.0f, viewHeight / contentHeight)
                           : 1.0f;
    sb->SetProportion(proportion);

    sb->SetSteps(lh, std::max(lh, viewHeight - lh));
  }

  Invalidate();
}

/**
 * @return The calculated line height for items.
 */
float SimpleColumnView::LineHeight() const {
  BFont font;
  const_cast<SimpleColumnView *>(this)->GetFont(&font);
  font_height fh;
  font.GetHeight(&fh);
  float fontHeight = fh.ascent + fh.descent + fh.leading;
  return ceilf(fontHeight * 1.4f);
}

void SimpleColumnView::Draw(BRect updateRect) {
  BRect bounds = Bounds();
  int32 first = (int32)(updateRect.top / fItemHeight);
  int32 last = (int32)(updateRect.bottom / fItemHeight);
  int32 maxRow = std::max(last, (int32)fItems.size() - 1);

  rgb_color base = ui_color(B_LIST_BACKGROUND_COLOR);

  int brightness = base.red + base.green + base.blue;
  bool isDark = (brightness < 384);

  for (int32 i = first; i <= maxRow; ++i) {
    float top = i * fItemHeight;
    BRect rowRect(bounds.left, top, bounds.right, top + fItemHeight - 1);

    if ((i & 1) == 0) {
      SetHighColor(base);
    } else {
      if (isDark)
        SetHighColor(tint_color(base, 0.90));
      else
        SetHighColor(tint_color(base, 1.05));
    }
    FillRect(rowRect);

    if (i < (int32)fItems.size()) {

      if (fItems[i].selected) {
        if (fUseCustomColor) {
          SetHighColor(fSelectionColor);
          FillRect(rowRect);
          // Gray border to match BColumnListView appearance
          SetHighColor((rgb_color){152, 152, 152, 255});
          StrokeRect(rowRect);
          SetHighColor(fSelectionTextColor);
        } else {
          rgb_color c = ui_color(B_LIST_SELECTED_BACKGROUND_COLOR);
          SetHighColor(c);
          FillRect(rowRect);
          // Gray border to match BColumnListView appearance
          SetHighColor((rgb_color){152, 152, 152, 255});
          StrokeRect(rowRect);
          SetHighColor(ui_color(B_LIST_SELECTED_ITEM_TEXT_COLOR));
        }
      } else {
        SetHighColor(ui_color(B_LIST_ITEM_TEXT_COLOR));
      }

      font_height fh;
      GetFontHeight(&fh);
      float textHeight = ceilf(fh.ascent + fh.descent + fh.leading);
      // Center vertically in the row rect
      float baseline = rowRect.top +
                       floorf((rowRect.Height() - textHeight) / 2.0f) +
                       fh.ascent;

      DrawString(fItems[i].text.String(), BPoint(rowRect.left + 5, baseline));
    }
  }
}

void SimpleColumnView::FrameResized(float width, float height) {
  UpdateScrollbars();
}

void SimpleColumnView::MouseDown(BPoint where) {
  int32 index = (int32)(where.y / fItemHeight);

  if (index >= 0 && index < (int32)fItems.size()) {
    Select(index);
    SelectionChanged(index);
  }
}

void SimpleColumnView::MessageReceived(BMessage *msg) {
  if (msg->what == B_SIMPLE_DATA) {
    int32 index = CurrentSelection();
    if (index >= 0) {
      DEBUG_PRINT("[SimpleColumnView] Drop received on index %d (%s)\\n", index,
                  ItemAt(index).String());
    }
  } else {
    BView::MessageReceived(msg);
  }
}

/**
 * @brief Sets the message command to send when selection changes.
 */
void SimpleColumnView::SetSelectionMessage(uint32 what) {
  DEBUG_PRINT(
      "[SimpleColumnView] SimpleColumnView::SetSelectionMessage this=%p "
      "what=%c\\n",
      this, (char)what);
  fSelectionWhat = what;
}

/**
 * @brief Sets the target messenger for selection change notifications.
 */
void SimpleColumnView::SetTarget(BMessenger target) { fTarget = target; }

/**
 * @brief Notifies the target that the selection has changed.
 */
void SimpleColumnView::SelectionChanged(int32 index) {
  if (fSelectionWhat != 0 && fTarget.IsValid() && index >= 0) {
    BMessage msg(fSelectionWhat);
    msg.AddInt32("index", index);
    msg.AddString("name", ItemAt(index));
    fTarget.SendMessage(&msg);
  }
}

void SimpleColumnView::SetSelectionColor(rgb_color color) {
  fSelectionColor = color;

  // Calculate relative luminance to determine text color
  float luminance =
      (0.299f * color.red + 0.587f * color.green + 0.114f * color.blue) /
      255.0f;

  // Use dark text on light backgrounds, light text on dark backgrounds
  if (luminance > 0.5f) {
    fSelectionTextColor = (rgb_color){0, 0, 0, 255};
  } else {
    fSelectionTextColor = (rgb_color){255, 255, 255, 255};
  }

  fUseCustomColor = true;
  Invalidate();
}
