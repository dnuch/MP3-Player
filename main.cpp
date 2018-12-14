//#define NDEBUG
#include <cassert>
#include <utilities.h>

#include "MP3Tasks.h"

int main(void) {
    /// This "stack" memory is enough for each task to run properly (512 * 32-bit (4 bytes)) = 2Kbytes stack
    const uint32_t STACK_SIZE_WORDS = 0x00000200;

    assert(sd->Init() && audio->Init() && Init_MP3_GPIO_Interrupts() && oled->Init());

    /// command line queue handles
    sdFileCmdTaskHandle = xQueueCreate(1, sizeof(uint8_t *));
    mp3CmdTaskHandle = xQueueCreate(2, sizeof(uint8_t *));
    txtCmdTaskHandle = xQueueCreate(2, sizeof(uint8_t *));

    /// isr queue handles
    mp3QueueHandle = xQueueCreate(2, sizeof(uint8_t *));

    scheduler_add_task(new terminalTask(PRIORITY_HIGH));

    /// command line handled tasks
    xTaskCreate(vPlayMp3FilesFromCmd, "cmp3cmd", STACK_SIZE_WORDS, nullptr, PRIORITY_HIGH, nullptr);
    xTaskCreate(vPlayTxtFilesFromCmd, "ctxtcmd", STACK_SIZE_WORDS, nullptr, PRIORITY_HIGH, nullptr);
    xTaskCreate(vSendFilesFromCmd,    "pmp3cmd", STACK_SIZE_WORDS, nullptr, PRIORITY_LOW,  nullptr);

    /// isr handled tasks
    xTaskCreate(vPlayMp3Files,        "cmp3",    STACK_SIZE_WORDS, nullptr, PRIORITY_HIGH, nullptr);
    xTaskCreate(vSendMp3Files,        "pmp3",    STACK_SIZE_WORDS, nullptr, PRIORITY_LOW,  nullptr);

    xTaskCreate(vIncrVolumeOrList, "vinc", STACK_SIZE_WORDS, nullptr, PRIORITY_HIGH, nullptr);
    xTaskCreate(vDecrVolumeOrList, "vdec", STACK_SIZE_WORDS, nullptr, PRIORITY_HIGH, nullptr);

    xTaskCreate(vFastForwardOrSelect, "vffs",    STACK_SIZE_WORDS, nullptr, PRIORITY_HIGH, nullptr);

    /// init oled display of song lists & Volume
    // assumes 3 songs in sd file
    oled->initDisplay(sd->getCurrentFileName(), sd->getNextFileNameFromIndex(1), sd->getNextFileNameFromIndex(2));

    scheduler_start();

    for (;;); /* endless loop - retain code execution */
}
