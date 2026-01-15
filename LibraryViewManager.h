#ifndef LIBRARY_VIEW_MANAGER_H
#define LIBRARY_VIEW_MANAGER_H

#include "ContentColumnView.h"
#include "MediaItem.h"
#include "SimpleColumnView.h"
#include <Messenger.h>
#include <String.h>
#include <SupportDefs.h>
#include <set>
#include <vector>

/**
 * @class LibraryViewManager
 * @brief Manages the "Column Browser" interface (Genre -> Artist -> Album ->
 * Tracks).
 *
 * This class handles the filtering logic, updating the cascading column views
 * based on selection, and maintaining the state of the "Active Playlist"
 * filtering (fActivePaths).
 *
 * It coordinates:
 * - `SimpleColumnView`s for Genre, Artist, Album.
 * - `ContentColumnView` for the main track list.
 */
class LibraryViewManager {
public:
  /**
   * @brief Constructs the manager.
   * @param target The BMessenger to receive selection change messages
   * (typically MainWindow).
   */
  LibraryViewManager(BMessenger target);
  ~LibraryViewManager();

  /** @name View Accessors */
  ///@{
  SimpleColumnView *GenreView() const;
  SimpleColumnView *ArtistView() const;
  SimpleColumnView *AlbumView() const;
  ContentColumnView *ContentView() const;

  /**
   * @brief Updates the filtered views based on the full database and current
   * selection.
   *
   * This is the heavy-lifting function that filters `allItems` down to the
   * lists displayed in each column using the current genre/artist/album
   * selection and search text.
   *
   * @param allItems Complete list of all media items in the cache.
   * @param isLibraryMode If true, shows everything. If false, filters by
   * `fActivePaths`.
   * @param currentPlaylist Name of the current playlist (used for display
   * context if needed).
   * @param filterText Search filter string (default empty).
   */
  void UpdateFilteredViews(const std::vector<MediaItem> &allItems,
                           bool isLibraryMode, const BString &currentPlaylist,
                           const BString &filterText = "");

  /**
   * @brief Incrementally adds a media item (used during live scanning).
   */
  void AddMediaItem(const MediaItem &item);

  /**
   * @brief Clears all filters and views.
   */
  void ResetFilters();

  ///@}

  /** @name Static Helper Methods */
  ///@{
  static BString SelectedText(SimpleColumnView *v);
  static BString SelectedData(SimpleColumnView *v);

  ///@}

  /** @name Playlist / Active Scope Management */
  ///@{
  const std::vector<BString> &ActivePaths() const;
  void SetActivePaths(const std::vector<BString> &paths);

  /**
   * @brief Checks if a specific file path is allowed in the current view mode.
   */
  bool IsPathAllowed(const BString &filePath, bool isLibraryMode) const;

private:
  /**
   * @brief Internal helper to check path allowance against active paths.
   */
  bool _PathAllowedByMode(const BString &filePath, bool isLibraryMode,
                          const std::vector<BString> &activePaths) const;

private:
  /** @name State */
  ///@{
  BMessenger fTarget;

  SimpleColumnView *fGenreView;
  SimpleColumnView *fArtistView;
  SimpleColumnView *fAlbumView;
  ContentColumnView *fContentView;

  std::vector<BString> fActivePaths;

  /// Cache last selection to avoid resetting downstream columns unnecessarily
  BString fLastSelectedGenre;
  BString fLastSelectedArtist;
  ///@}
};

#endif // LIBRARY_VIEW_MANAGER_H
