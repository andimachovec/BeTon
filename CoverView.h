#ifndef COVER_VIEW_H
#define COVER_VIEW_H

#include <View.h>
class BBitmap;

/**
 * @class CoverView
 * @brief A simple view to display album cover art.
 *
 * It handles:
 * - Scaling the image to fit the view.
 * - Managing the lifecycle of the BBitmap (takes ownership).
 */
class CoverView : public BView {
public:
  explicit CoverView(const char *name);
  ~CoverView() override;

  /**
   * @brief Sets the cover image.
   * @param bmp The bitmap to display. The view takes a copy/ownership logic
   * depending on implementation. (Note: Implementation actually duplicates the
   * bitmap, so this pointer ownership transfer is implicit via copy).
   */
  void SetBitmap(BBitmap *bmp);

  void Draw(BRect update) override;
  void GetPreferredSize(float *w, float *h) override;

private:
  /** @name Data */
  ///@{
  BBitmap *fBitmap = nullptr;
  ///@}
};

#endif // COVER_VIEW_H
