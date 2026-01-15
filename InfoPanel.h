#ifndef INFO_PANEL_H
#define INFO_PANEL_H

#include <Box.h>
#include <String.h>
#include <View.h>

class BCardLayout;
class BStringView;
class BBitmap;
class CoverView;

/**
 * @class InfoPanel
 * @brief A view that toggles between displaying file metadata and cover art.
 *
 * Uses a BCardLayout to switch between the two states.
 * - In `Info` mode, it shows a BStringView with track details.
 * - In `Cover` mode, it shows a CoverView.
 */
class InfoPanel : public BView {
public:
  InfoPanel();
  ~InfoPanel() override;

  /**
   * @brief Display modes for the InfoPanel.
   */
  enum Mode {
    Info = 0, ///< Shows text metadata (artist, album, etc.)
    Cover = 1 ///< Shows album cover art
  };

  /**
   * @brief Manually switch the display mode.
   * @param mode The mode to switch to.
   */
  void Switch(Mode mode);

  /**
   * @brief Returns the current display mode.
   */
  Mode GetMode() const;

  /**
   * @brief Updates the metadata text.
   * Does NOT automatically switch to Info mode.
   */
  void SetFileInfo(const BString &text);

  /**
   * @brief Sets the cover image.
   * Automatically switches to Cover mode.
   */
  void SetCover(BBitmap *bmp);

  /**
   * @brief Clears the current cover image.
   */
  void ClearCover();

  void MessageReceived(BMessage *msg) override;

private:
  /** @name UI Components */
  ///@{
  BCardLayout *fCards = nullptr;

  BBox *fInfoBox = nullptr;
  BStringView *fInfoText = nullptr;

  BView *fCoverPane = nullptr;
  CoverView *fCoverView = nullptr;
  ///@}

  Mode fMode = Info;
};

#endif // INFO_PANEL_V2_H
