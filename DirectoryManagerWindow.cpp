#include "DirectoryManagerWindow.h"
#include "Messages.h"

#include <Catalog.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "DirectoryManagerWindow"

#include <Alert.h>
#include <Directory.h>
#include <Entry.h>
#include <File.h>
#include <FindDirectory.h>
#include <LayoutBuilder.h>
#include <NodeInfo.h>
#include <Path.h>
#include <StorageDefs.h>

/**
 * @brief Constructs the Directory Manager window.
 *
 * Sets up the UI layout:
 * - List view for directories.
 * - Add/Remove/OK buttons.
 * - File panel for selecting new folders.
 *
 * Loads existing directory settings from disk.
 *
 * @param cacheManager Messenger to the CacheManager for triggering rescans.
 */
DirectoryManagerWindow::DirectoryManagerWindow(BMessenger cacheManager)
    : BWindow(BRect(100, 100, 500, 400), B_TRANSLATE("Manage Music Folders"),
              B_TITLED_WINDOW, B_ASYNCHRONOUS_CONTROLS),
      fCacheManager(cacheManager) {

  // UI Components
  fDirectoryList = new BListView("directoryList");
  BScrollView *scroll =
      new BScrollView("scroll", fDirectoryList, 0, false, true);

  fBtnAdd = new BButton("Add", B_TRANSLATE("Add"), new BMessage(MSG_DIR_ADD));
  fBtnRemove = new BButton("Remove", B_TRANSLATE("Remove"),
                           new BMessage(MSG_DIR_REMOVE));
  fBtnOK = new BButton("OK", B_TRANSLATE("OK"), new BMessage(MSG_DIR_OK));

  // File Panel for folder selection
  fAddPanel =
      new BFilePanel(B_OPEN_PANEL, new BMessenger(this), nullptr,
                     B_DIRECTORY_NODE, false, nullptr, nullptr, true, true);

  // Layout Setup
  BBox *buttonBox = new BBox(B_FANCY_BORDER);
  BLayoutBuilder::Group<>(buttonBox, B_HORIZONTAL, 10)
      .SetInsets(10, 10, 10, 10)
      .Add(fBtnAdd)
      .Add(fBtnRemove)
      .AddGlue()
      .Add(fBtnOK);

  BLayoutBuilder::Group<>(this, B_VERTICAL, 10)
      .SetInsets(10, 10, 10, 10)
      .Add(scroll)
      .Add(buttonBox);

  // Calculate font-relative size
  font_height fh;
  be_plain_font->GetHeight(&fh);
  float fontHeight = fh.ascent + fh.descent + fh.leading;

  ResizeTo(fontHeight * 27, fontHeight * 20);
  CenterOnScreen();

  // Load saved settings
  BPath settingsPath;
  if (find_directory(B_USER_SETTINGS_DIRECTORY, &settingsPath) == B_OK) {
    settingsPath.Append("BeTon/directories.txt");
    BFile file(settingsPath.Path(), B_READ_ONLY);
    if (file.InitCheck() == B_OK) {
      BString line;
      char ch;
      while (file.Read(&ch, 1) == 1) {
        if (ch == '\n') {
          if (!line.IsEmpty()) {
            fDirectoryList->AddItem(new BStringItem(line.String()));
            fDirectories.push_back(BPath(line.String()));
            line.Truncate(0);
          }
        } else {
          line += ch;
        }
      }
    }
  }
}

void DirectoryManagerWindow::MessageReceived(BMessage *msg) {
  switch (msg->what) {
  case MSG_DIR_ADD:
    fAddPanel->Show();
    break;

  case B_REFS_RECEIVED: {
    entry_ref ref;
    if (msg->FindRef("refs", &ref) == B_OK) {
      AddDirectory(ref);
    }
    break;
  }

  case MSG_DIR_REMOVE:
    RemoveSelectedDirectory();
    break;

  case MSG_DIR_OK:
    SaveSettings();
    // Notify CacheManager to rescan with new list
    if (fCacheManager.IsValid()) {
      fCacheManager.SendMessage(MSG_RESCAN);
    }
    Quit();
    break;

  default:
    BWindow::MessageReceived(msg);
    break;
  }
}

/**
 * @brief Adds a directory to the list.
 * Prevents adding duplicate or invalid directories.
 */
void DirectoryManagerWindow::AddDirectory(const entry_ref &ref) {
  BPath path(&ref);
  if (path.InitCheck() != B_OK)
    return;

  // Check for duplicates!!
  for (int32 i = 0; i < fDirectoryList->CountItems(); ++i) {
    BStringItem *item = static_cast<BStringItem *>(fDirectoryList->ItemAt(i));
    if (item && item->Text() == path.Path())
      return;
  }

  fDirectoryList->AddItem(new BStringItem(path.Path()));
  fDirectories.push_back(path);
}

void DirectoryManagerWindow::RemoveSelectedDirectory() {
  int32 index = fDirectoryList->CurrentSelection();
  if (index < 0)
    return;

  delete fDirectoryList->RemoveItem(index);
  fDirectories.erase(fDirectories.begin() + index);
}

/**
 * @brief Saves the list of configured directories to disk.
 * Path: ~/config/settings/BeTon/directories.txt
 */
void DirectoryManagerWindow::SaveSettings() {
  BPath settingsPath;
  if (find_directory(B_USER_SETTINGS_DIRECTORY, &settingsPath) != B_OK)
    return;

  settingsPath.Append("BeTon");
  create_directory(settingsPath.Path(), 0755);
  settingsPath.Append("directories.txt");

  BFile file(settingsPath.Path(), B_WRITE_ONLY | B_CREATE_FILE | B_ERASE_FILE);
  if (file.InitCheck() != B_OK)
    return;

  for (const auto &path : fDirectories) {
    file.Write(path.Path(), strlen(path.Path()));
    file.Write("\n", 1);
  }
}
