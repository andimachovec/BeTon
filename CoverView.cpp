#include "CoverView.h"
#include "Debug.h"
#include <Bitmap.h>
#include <cstdio>

CoverView::CoverView(const char *name)
    : BView(name, B_WILL_DRAW | B_FULL_UPDATE_ON_RESIZE) {
  SetViewColor(B_TRANSPARENT_COLOR);
}

CoverView::~CoverView() {
  delete fBitmap;
  fBitmap = nullptr;
}

/**
 * @brief Updates the displayed cover image.
 * Makes a defensive copy of the provided bitmap.
 *
 * @param bmp The new bitmap to display (can be nullptr to clear).
 */
void CoverView::SetBitmap(BBitmap *bmp) {
  BBitmap *clone = nullptr;
  if (bmp && bmp->IsValid()) {
    clone = new BBitmap(bmp);
    if (!clone->IsValid()) {
      delete clone;
      clone = nullptr;
    }
  }
  delete fBitmap;
  fBitmap = clone;

  DEBUG_PRINT("[CoverView] SetBitmap: %p %s\n", fBitmap,
              (fBitmap && fBitmap->IsValid()) ? "valid" : "null");
  Invalidate();
}

/**
 * @brief Draws the cover scaled to fit the view bounds.
 */
void CoverView::Draw(BRect) {
  // Fill background
  SetHighColor(ui_color(B_PANEL_BACKGROUND_COLOR));
  FillRect(Bounds());

  if (!fBitmap || !fBitmap->IsValid()) {
    // No valid bitmap -> just background
    return;
  }

  // Draw scaled bitmap
  DrawBitmapAsync(fBitmap, fBitmap->Bounds(), Bounds());
}

void CoverView::GetPreferredSize(float *w, float *h) {
  if (w)
    *w = 200;
  if (h)
    *h = 200;
}
