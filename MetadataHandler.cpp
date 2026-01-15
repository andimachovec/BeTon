#include "MetadataHandler.h"
#include "CacheManager.h"
#include "Debug.h"
#include "Messages.h"
#include "TagSync.h"

#include <Alert.h>
#include <Directory.h>
#include <Entry.h>
#include <Path.h>

MetadataHandler::MetadataHandler(BMessenger target) : fTarget(target) {}

MetadataHandler::~MetadataHandler() {}

/**
 * @brief Applies the provided cover art data to all audio files in the same
 * directory as the given file.
 *
 * @param filePath Path to a file in the target directory (usually the one being
 * edited).
 * @param data Pointer to the raw image data.
 * @param size Size of the image data in bytes.
 */
void MetadataHandler::ApplyAlbumCover(const BString &filePath, const void *data,
                                      ssize_t size) {
  _ProcessDirectoryForCover(filePath, false, data, size);
}

/**
 * @brief Removes embedded cover art from all audio files in the same directory
 * as the given file.
 *
 * @param filePath Path to a file in the target directory.
 */
void MetadataHandler::ClearAlbumCover(const BString &filePath) {
  _ProcessDirectoryForCover(filePath, true);
}

/**
 * @brief Applies cover art to all files specified in the BMessage.
 *
 * Takes a BMessage containing a "bytes" buffer and "mime" string, and a list of
 * "file" strings. Applies the cover to each file individually.
 *
 * @param msg The message containing cover data and file list.
 */
void MetadataHandler::ApplyCoverToAll(const BMessage *msg) {
  const void *data = nullptr;
  ssize_t size = 0;
  if (msg->FindData("bytes", B_RAW_TYPE, &data, &size) == B_OK && data &&
      size > 0) {
    const char *mime = nullptr;
    msg->FindString("mime", &mime);
    BString path;
    int32 i = 0;
    while (msg->FindString("file", i++, &path) == B_OK) {
      TagSync::WriteEmbeddedCover(BPath(path.String()), (const uint8 *)data,
                                  (size_t)size, mime);
    }
  }
}

/**
 * @brief Saves metadata tags to one or more files based on the BMessage.
 *
 * Iterates through "file" entries in the message and updates tags based on
 * available fields (title, artist, album, etc.).
 * Also updates BFS attributes if available and notifies the UI/CacheManager.
 *
 * @param msg The message containing tag data and file paths.
 */
void MetadataHandler::SaveTags(const BMessage *msg) {
  BString file;
  int32 i = 0;

  while (msg->FindString("file", i++, &file) == B_OK) {
    if (file.IsEmpty())
      continue;

    BPath path(file.String());

    TagData td;
    TagSync::ReadTags(path, td);

    BString s;
    if (msg->FindString("title", &s) == B_OK)
      td.title = s;
    if (msg->FindString("artist", &s) == B_OK)
      td.artist = s;
    if (msg->FindString("album", &s) == B_OK)
      td.album = s;
    if (msg->FindString("albumArtist", &s) == B_OK)
      td.albumArtist = s;
    if (msg->FindString("composer", &s) == B_OK)
      td.composer = s;
    if (msg->FindString("genre", &s) == B_OK)
      td.genre = s;
    if (msg->FindString("comment", &s) == B_OK)
      td.comment = s;

    auto _toUInt = [](const char *str) -> unsigned int {
      return (unsigned int)atoi(str);
    };

    if (msg->FindString("year", &s) == B_OK)
      td.year = _toUInt(s.String());
    if (msg->FindString("track", &s) == B_OK)
      td.track = _toUInt(s.String());
    if (msg->FindString("trackTotal", &s) == B_OK)
      td.trackTotal = _toUInt(s.String());
    if (msg->FindString("disc", &s) == B_OK)
      td.disc = _toUInt(s.String());
    if (msg->FindString("discTotal", &s) == B_OK)
      td.discTotal = _toUInt(s.String());

    if (msg->FindString("mbAlbumID", &s) == B_OK)
      td.mbAlbumID = s;
    if (msg->FindString("mbArtistID", &s) == B_OK)
      td.mbArtistID = s;
    if (msg->FindString("mbTrackID", &s) == B_OK)
      td.mbTrackID = s;

    DEBUG_PRINT("[MetadataHandler] SaveTags: Calling WriteTagsToFile. "
                "mbAlbumID='%s', mbTrackID='%s'\\n",
                td.mbAlbumID.String(), td.mbTrackID.String());
    bool ok = TagSync::WriteTagsToFile(path, td, nullptr);
    if (ok) {
      TagData tdSaved;
      TagSync::ReadTags(path, tdSaved);

      if (TagSync::IsBeFsVolume(path)) {
        TagSync::WriteBfsAttributes(path, tdSaved, nullptr, 512 * 1024);
      }

      BMessage update(MSG_MEDIA_ITEM_FOUND);
      update.AddString("path", path.Path());
      update.AddString("title", td.title);
      update.AddString("artist", td.artist);
      update.AddString("album", td.album);
      update.AddString("genre", td.genre);
      update.AddString("comment", td.comment);
      update.AddInt32("year", td.year);
      update.AddInt32("track", td.track);
      update.AddInt32("trackTotal", td.trackTotal);
      update.AddInt32("disc", td.disc);
      update.AddInt32("discTotal", td.discTotal);
      update.AddInt32("duration", td.lengthSec);
      update.AddInt32("bitrate", td.bitrate);

      update.AddString("mbAlbumID", td.mbAlbumID);
      update.AddString("mbArtistID", td.mbArtistID);
      update.AddString("mbTrackID", td.mbTrackID);

      fTarget.SendMessage(&update);

    } else {
      (new BAlert("savefail", "Konnte Tags nicht speichern.", "OK"))->Go();
    }
  }
}

/**
 * @brief Helper to iterate over the text file's directory and apply/clear cover
 * art for all supported audio files.
 *
 * @param filePath Path to a file in the target directory (acts as anchor).
 * @param clear If true, removes cover art. If false, applies `data`.
 * @param data Raw image data (ignored if clear is true).
 * @param size Size of image data (ignored if clear is true).
 */
void MetadataHandler::_ProcessDirectoryForCover(const BString &filePath,
                                                bool clear, const void *data,
                                                size_t size) {
  BPath p(filePath.String());
  BPath parent;
  if (p.GetParent(&parent) == B_OK) {
    BDirectory dir(parent.Path());
    BEntry entry;
    while (dir.GetNextEntry(&entry) == B_OK) {
      BPath ep;
      if (entry.GetPath(&ep) == B_OK && !entry.IsDirectory()) {
        BString pathStr = ep.Path();

        if (pathStr.EndsWith(".mp3") || pathStr.EndsWith(".flac") ||
            pathStr.EndsWith(".m4a") || pathStr.EndsWith(".ogg") ||
            pathStr.EndsWith(".wav")) {

          bool res;
          if (clear) {
            res = TagSync::WriteEmbeddedCover(ep, nullptr, 0, nullptr);
          } else {
            res = TagSync::WriteEmbeddedCover(ep, (const uint8 *)data, size,
                                              nullptr);
          }
          DEBUG_PRINT("  -> %s cover for '%s': %s\\n",
                      clear ? "clearing" : "applying", pathStr.String(),
                      res ? "OK" : "FAIL");
        }
      }
    }
  }
}
