#include "PlaylistUtils.h"
#include "Debug.h"
#include "MainWindow.h"

#include <Directory.h>
#include <Entry.h>
#include <File.h>
#include <FindDirectory.h>
#include <Path.h>
#include <StringList.h>

#include <cstdio>

#define PLAYLIST_FOLDER "Playlists"

// Global pointer to MainWindow is needed to notify UI updates.
extern MainWindow *gMainWindow;

/**
 * @brief Constructs the path to the playlist directory in user settings.
 * @return BPath pointing to the 'BeTon/Playlists' folder.
 */
static BPath GetPlaylistDirectory() {
  BPath path;
  if (find_directory(B_USER_SETTINGS_DIRECTORY, &path) == B_OK) {
    path.Append("BeTon");
    path.Append(PLAYLIST_FOLDER);
  }
  return path;
}

/**
 * @brief Adds a track to a specific playlist.
 *
 * If the track is already in the playlist, it is not added again.
 * The playlist file (.m3u) is updated, and if the UI is available,
 * the track is added to the UI display as well.
 *
 * @param path File path of the track.
 * @param playlist Name of the playlist (without extension).
 */
void AddItemToPlaylist(const BString &path, const BString &playlist) {
  DEBUG_PRINT("[PlaylistUtils] AddItemToPlaylist aufgerufen mit: %s -> %s\n",
              path.String(), playlist.String());

  std::vector<BString> items = LoadPlaylist(playlist);
  for (const auto &item : items) {
    if (item.Compare(path) == 0) {
      DEBUG_PRINT("[PlaylistUtils] Pfad bereits enthalten, wird nicht erneut "
                  "hinzugefuegt\n");
      return;
    }
  }

  items.push_back(path);
  SavePlaylist(playlist, items);
  DEBUG_PRINT("[PlaylistUtils] Pfad hinzugefuegt und gespeichert\n");

  if (gMainWindow) {
    BString label;

    BPath p(path.String());
    label = p.Leaf();

    gMainWindow->AddPlaylistEntry(playlist, label, path);
  }
}

/**
 * @brief Retrieves the file path for a content item by its index.
 * @param index Index in the content list.
 * @return File path string.
 */
BString GetPathForContentItem(int index) {
  if (gMainWindow)
    return gMainWindow->GetPathForContentItem(index);
  return "";
}

/**
 * @brief Creates a new, empty playlist file.
 * @param name Name of the playlist.
 */
void CreatePlaylist(const BString &name) {
  std::vector<BString> empty;
  SavePlaylist(name, empty);
  DEBUG_PRINT("[PlaylistUtils] Neue Playlist '%s' angelegt\n", name.String());
}

/**
 * @brief Deletes a playlist file.
 * @param name Name of the playlist to delete.
 */
void DeletePlaylist(const BString &name) {
  BPath dirPath(GetPlaylistDirectory());

  BString fileName = name;
  fileName += ".m3u";
  dirPath.Append(fileName.String());

  BEntry entry(dirPath.Path());
  if (entry.Exists()) {
    if (entry.Remove() == B_OK) {
      DEBUG_PRINT("[PlaylistUtils] Playlist '%s' geloescht (%s)\n",
                  name.String(), dirPath.Path());
    } else {
      DEBUG_PRINT(
          "[PlaylistUtils] Playlist '%s' konnte nicht geloescht werden\n",
          name.String());
    }
  }
}

/**
 * @brief Loads all track paths from a playlist file.
 *
 * Each line in the .m3u file is treated as a file path.
 *
 * @param name Name of the playlist.
 * @return Vector of file path strings.
 */
std::vector<BString> LoadPlaylist(const BString &name) {
  std::vector<BString> items;

  BPath dirPath = GetPlaylistDirectory();
  BString fileName = name;
  fileName += ".m3u";
  dirPath.Append(fileName.String());

  BFile file(dirPath.Path(), B_READ_ONLY);
  if (file.InitCheck() != B_OK) {
    DEBUG_PRINT("[PlaylistUtils] Datei konnte nicht geoeffnet werden: %s\n",
                dirPath.Path());
    return items;
  }

  off_t fileSize = 0;
  if (file.GetSize(&fileSize) != B_OK || fileSize == 0) {
    DEBUG_PRINT("[PlaylistUtils] Datei leer oder ungueltig: %s\n",
                dirPath.Path());
    return items;
  }

  char *buffer = new char[fileSize + 1];
  if (file.Read(buffer, fileSize) != fileSize) {
    DEBUG_PRINT("[PlaylistUtils] Fehler beim Lesen der Datei: %s\n",
                dirPath.Path());
    delete[] buffer;
    return items;
  }

  buffer[fileSize] = '\0';
  BString content(buffer);
  delete[] buffer;

  BStringList lines;
  content.Split("\n", true, lines);

  for (int32 i = 0; i < lines.CountStrings(); ++i) {
    BString line = lines.StringAt(i);
    line.RemoveAll("\r");
    if (!line.IsEmpty()) {
      items.push_back(line);
      DEBUG_PRINT("[PlaylistUtils] Eingelesen: %s\n", line.String());
    }
  }

  DEBUG_PRINT("[PlaylistUtils] %zu Eintraege geladen aus Playlist '%s'\n",
              items.size(), name.String());
  return items;
}

/**
 * @brief Saves a list of paths to a playlist file.
 *
 * Overwrites the existing file. Creates the directory if it doesn't exist.
 *
 * @param name Name of the playlist.
 * @param paths Vector of path strings to save.
 */
void SavePlaylist(const BString &name, const std::vector<BString> &paths) {
  BPath dirPath = GetPlaylistDirectory();

  BDirectory dir(dirPath.Path());
  if (dir.InitCheck() != B_OK) {
    create_directory(dirPath.Path(), 0777);
  }

  BString fileName = name;
  fileName += ".m3u";
  dirPath.Append(fileName.String());

  BFile file(dirPath.Path(), B_WRITE_ONLY | B_CREATE_FILE | B_ERASE_FILE);
  if (file.InitCheck() != B_OK) {
    DEBUG_PRINT(
        "[PlaylistUtils-ERROR] Konnte Playlist-Datei nicht oeffnen: %s\n",
        dirPath.Path());
    return;
  }

  for (const auto &path : paths) {
    file.Write(path.String(), path.Length());
    file.Write("\n", 1);
    DEBUG_PRINT("-> geschrieben: %s\n", path.String());
  }

  DEBUG_PRINT("Playlist '%s' gespeichert (%zu Eintraege)\n", name.String(),
              paths.size());
}
