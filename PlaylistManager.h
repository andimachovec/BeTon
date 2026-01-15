#ifndef PLAYLIST_MANAGER_H
#define PLAYLIST_MANAGER_H

#include <Message.h>
#include <Messenger.h>
#include <String.h>
#include <vector>

class PlaylistListView;

class PlaylistManager {
public:
  PlaylistManager(BMessenger target);
  virtual ~PlaylistManager();

  PlaylistListView *View() const;

  void LoadAvailablePlaylists();
  std::vector<BString> LoadPlaylist(const BString &name);
  void SavePlaylist(const BString &name, const std::vector<BString> &paths);

  void AddPlaylistEntry(const BString &name, const BString &fullPath);
  void CreateNewPlaylist(const BString &name);
  void RenamePlaylist(const BString &oldName, const BString &newName);
  void SetPlaylistFolderPath(const BString &path);
  void GetPlaylistNames(BMessage &out, bool onlyWritable = true) const;
  bool IsPlaylistWritable(const BString &name) const;

  void ReorderPlaylistItem(const BString &name, int32 fromIndex, int32 toIndex);

  void Select(int32 index);
  int32 CountItems() const;

private:
  /** @name Data */
  ///@{
  PlaylistListView *fPlaylistView;
  BMessenger fTarget;
  BString fPlaylistBasePath;
  ///@}
};

#endif
