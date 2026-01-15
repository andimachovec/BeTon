#include "NamePrompt.h"
#include "Messages.h"

#include <Catalog.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT                                                  \
  "NamePrompt" // Or "Playlist" if that's more semantic, but usually Classname
               // is safe.

#include <Button.h>
#include <LayoutBuilder.h>
#include <TextControl.h>

/**
 * @brief Constructs the NamePrompt window.
 * @param target The messenger to send the result (OK/Cancel) to.
 */
NamePrompt::NamePrompt(BMessenger target)
    : BWindow(BRect(100, 100, 400, 180), B_TRANSLATE("Playlist"),
              B_TITLED_WINDOW_LOOK, B_MODAL_APP_WINDOW_FEEL,
              B_NOT_RESIZABLE | B_AUTO_UPDATE_SIZE_LIMITS),
      fTarget(target), fMessageWhat(MSG_PLAYLIST_CREATED) {
  fText = new BTextControl("name", B_TRANSLATE("Name:"), "", nullptr);

  BButton *okButton =
      new BButton("ok", B_TRANSLATE("OK"), new BMessage(MSG_NAME_PROMPT_OK));
  okButton->MakeDefault(true);
  BButton *cancelButton = new BButton("cancel", B_TRANSLATE("Cancel"),
                                      new BMessage(MSG_NAME_PROMPT_CANCEL));

  BLayoutBuilder::Group<>(this, B_VERTICAL, B_USE_DEFAULT_SPACING)
      .SetInsets(B_USE_DEFAULT_SPACING)
      .Add(fText)
      .AddGroup(B_HORIZONTAL)
      .AddGlue()
      .Add(cancelButton)
      .Add(okButton)
      .End();

  CenterOnScreen();
}

/**
 * @brief Sets the text initially displayed in the input field.
 * @param name The initial name string.
 */
void NamePrompt::SetInitialName(const BString &name) {
  if (fText)
    fText->SetText(name);
}

/**
 * @brief Sets the command constant for the message sent upon confirmation.
 * @param what The message 'what' constant (e.g., MSG_PLAYLIST_CREATED).
 */
void NamePrompt::SetMessageWhat(uint32 what) { fMessageWhat = what; }

void NamePrompt::MessageReceived(BMessage *msg) {
  switch (msg->what) {
  case MSG_NAME_PROMPT_OK: {
    BMessage reply(fMessageWhat);
    reply.AddString("name", fText->Text());

    if (fMessageWhat == MSG_NAME_PROMPT_RENAME) {
      // The old playlist name is stored in the window title by the caller
      reply.AddString("old", Title());
    }
    fTarget.SendMessage(&reply);
    Quit();
    break;
  }
  case MSG_NAME_PROMPT_CANCEL:
    Quit();
    break;
  default:
    BWindow::MessageReceived(msg);
  }
}
