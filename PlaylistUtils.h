#ifndef PLAYLIST_UTILS_H
#define PLAYLIST_UTILS_H

#include <String.h>
#include <vector>

/**
 * @brief Loads the contents of a playlist file.
 * @param name The name of the playlist (without .m3u extension).
 * @return A vector of file paths contained in the playlist.
 */
std::vector<BString> LoadPlaylist(const BString &name);

/**
 * @brief Saves a list of paths to a playlist file.
 * @param name The name of the playlist (without .m3u extension).
 * @param paths The list of file paths to save.
 */
void SavePlaylist(const BString &name, const std::vector<BString> &paths);

/**
 * @brief Appends a single item to an existing playlist.
 *
 * If the item is already present, it will not be added again.
 *
 * @param path The file path of the item to add.
 * @param playlist The name of the target playlist.
 */
void AddItemToPlaylist(const BString &path, const BString &playlist);

/**
 * @brief Creates a new, empty playlist file.
 * @param name The name of the new playlist.
 */
void CreatePlaylist(const BString &name);

/**
 * @brief Deletes an existing playlist file.
 * @param name The name of the playlist to delete.
 */
void DeletePlaylist(const BString &name);

#endif // PLAYLIST_UTILS_H
