#ifndef TAG_SYNC_H
#define TAG_SYNC_H

#include <Path.h>
#include <String.h>
#include <SupportDefs.h>

#include <cstdint>
#include <vector>

/**
 * @struct TagData
 * @brief Holds metadata read from or written to audio files.
 */
struct TagData {
  BString title, artist, album, genre, comment;

  uint32 year = 0;
  uint32 track = 0;
  BString albumArtist;
  BString composer;
  uint32 trackTotal = 0;
  uint32 disc = 0;
  uint32 discTotal = 0;

  uint32 lengthSec = 0;
  uint32 bitrate = 0;
  uint32 sampleRate = 0;
  uint32 channels = 0;

  BString mbAlbumID, mbArtistID, mbTrackID;
  BString acoustId, acoustIdFp;
};

/**
 * @struct CoverBlob
 * @brief Simple container for binary cover art data.
 */
struct CoverBlob {
  std::vector<uint8_t> bytes;

  void clear() { bytes.clear(); }
  void assign(const void *p, size_t n) {
    const auto *b = static_cast<const uint8_t *>(p);
    bytes.assign(b, b + n);
  }
  const void *data() const { return bytes.empty() ? nullptr : bytes.data(); }
  size_t size() const { return bytes.size(); }
};

/**
 * @namespace TagSync
 * @brief Utilities for reading and writing audio file metadata.
 */
namespace TagSync {

/**
 * @brief Reads metadata from the specified file.
 * @param path The path to the audio file.
 * @param out Output struct to populate with metadata.
 * @return True on success, false otherwise.
 */
bool ReadTags(const BPath &path, TagData &out);

/**
 * @brief Extracts embedded cover art from the file.
 * @param path The path to the audio file.
 * @param out Output struct to populate with cover data.
 * @return True if cover art was found and extracted.
 */
bool ExtractEmbeddedCover(const BPath &path, CoverBlob &out);

/**
 * @brief Writes embedded cover art to the file.
 * @param file The path to the audio file.
 * @param data Pointer to the image data.
 * @param size Size of the image data in bytes.
 * @param mimeOpt Optional MIME type string (e.g., "image/jpeg").
 * @return True on success.
 */
bool WriteEmbeddedCover(const BPath &file, const uint8_t *data, size_t size,
                        const char *mimeOpt = nullptr);

/**
 * @brief Overload that takes a CoverBlob.
 */
bool WriteEmbeddedCover(const BPath &file, const CoverBlob &blob,
                        const char *mimeOpt = nullptr);

/**
 * @brief Writes metadata and optionally cover art to the file.
 * @param path The path to the audio file.
 * @param td The new metadata to write.
 * @param coverOpt Optional pointer to cover art data to write.
 * @return True on success.
 */
bool WriteTagsToFile(const BPath &path, const TagData &td,
                     const CoverBlob *coverOpt);

/**
 * @brief Writes only metadata tags to the file.
 * @param path The path to the audio file.
 * @param in The new metadata to write.
 * @return True on success.
 */
bool WriteTags(const BPath &path, const TagData &in);

/**
 * @brief Mirrors metadata and cover art to BFS attributes.
 * @param path The path to the file.
 * @param in The metadata to write as attributes.
 * @param cover Optional cover data to write as a thumbnail attribute.
 * @param coverMaxBytes Maximum allowed size for the cover attribute.
 * @return True on success.
 */
bool WriteBfsAttributes(const BPath &path, const TagData &in,
                        const CoverBlob *cover,
                        size_t coverMaxBytes = 512 * 1024);

/**
 * @brief Checks if the file resides on a BFS volume.
 * @param path The path to check. 
 * @return True if on a BFS volume, false otherwise.
 */
bool IsBeFsVolume(const BPath &path);

} // namespace TagSync

#endif // TAG_SYNC_H
