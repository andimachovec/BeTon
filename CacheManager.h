#ifndef CACHE_MANAGER_H
#define CACHE_MANAGER_H

#include "MediaItem.h"
#include "Messages.h"
#include <Looper.h>
#include <Messenger.h>
#include <String.h>
#include <map>
#include <vector>

/**
 * @class CacheManager
 * @brief Manages the central media library cache.
 *
 * The CacheManager is responsible for:
 * - Loading and saving the 'media.cache' file.
 * - Coordinating the scanning process (via MediaScanner).
 * - Maintaining the in-memory state of all known media files (fEntries).
 * - Notifying the UI about progress and updates.
 *
 * It runs as a BLooper to handle asynchronous messages.
 */
class CacheManager : public BLooper {
public:
  /**
   * @brief Construct a new Cache Manager object
   *
   * @param target The target messenger (usually MainWindow) to receive
   * notifications.
   */
  CacheManager(const BMessenger &target);

  /**
   * @brief Loads the cache from disk.
   */
  void LoadCache();

  /**
   * @brief Saves the current cache to disk.
   */
  void SaveCache();

  /**
   * @brief Starts the scanning process for all configured directories.
   */
  void StartScan();

  void MessageReceived(BMessage *msg) override;

  const std::map<BString, MediaItem> &Entries() const { return fEntries; }

  /**
   * @brief Returns a flattened vector of all media items.
   * Useful for UI population.
   */
  std::vector<MediaItem> AllEntries() const;

private:
  void AddOrUpdateEntry(const MediaItem &entry);
  void LoadDirectories(std::vector<BString> &outDirs);
  void MarkBaseOffline(const BString &basePath);

  /** @name Data */
  ///@{
  std::map<BString, MediaItem> fEntries;
  BMessenger fTarget;
  BString fCachePath;
  int32 fActiveScanners{0};
  ///@}
};

#endif // CACHE_MANAGER_H
