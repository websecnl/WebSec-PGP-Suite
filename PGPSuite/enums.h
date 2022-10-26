#pragma once

namespace suite
{
    enum
    {
        ID_Hello = wxID_HIGHEST + 1,

        /* id's to link button to input field */
        ID_OPEN_PUBKEY,
        ID_OPEN_SECKEY,
        ID_OPEN_ENC_FILE,
        ID_OPEN_FILE,
        ID_EDIT_TEXT,

        /* actions */
        ID_GENERATE_KEY,
        ID_ENCRYPT_FILE,
        ID_DECRYPT_FILE,
        ID_ENC_TYPE_RADIO_CHANGED,
        ID_SHOW_GENERATE_SETTINGS,
        ID_CHECK_VERSION,
        ID_STARTUP_CHECKBOX_SETTING,

        /* menu's */
        ID_REGISTER_EXTENSION,

        /* unused */
        ID_SAVE_FILE,
    };
}