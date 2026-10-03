#include "common.h"
#include "ipc_session.h"
#include "co1_system_calls.h"
#include "co2_process_control.h"
#include "co3_ipc.h"
#include "co4_memory.h"
#include "co5_files.h"
#include "co6_threads.h"

/*
 * Linux IPC Monitoring and Message Transfer System
 *
 * An integrated application that demonstrates all six Course Outcomes
 * in one connected message-transfer workflow:
 *
 *   CO-1: OS as a service layer — real system calls with return values
 *   CO-2: Process creation and control — fork(), waitpid(), PIDs
 *   CO-3: IPC — anonymous pipe, named FIFO, signals, synchronization
 *   CO-4: Memory management — malloc(), Copy-on-Write, free()
 *   CO-5: File descriptors and filesystems — pipe/FIFO descriptors, stat()
 *   CO-6: Concurrency and synchronization — mutex-protected statistics
 */

static void print_menu(void)
{
    printf("\n");
    printf("===========================================\n");
    printf("  LINUX IPC MONITORING & MESSAGE TRANSFER\n");
    printf("===========================================\n");
    printf("\n");
    printf("  1. Start integrated IPC session — anonymous pipe\n");
    printf("  2. Start integrated IPC session — named FIFO\n");
    printf("  3. Run complete end-to-end demonstration\n");
    printf("  4. Inspect IPC resources and file descriptors\n");
    printf("  5. Run memory/Copy-on-Write diagnostic\n");
    printf("  6. Run concurrency and mutex diagnostic\n");
    printf("  7. Run system-call verification instructions\n");
    printf("  8. Exit safely\n");
    printf("\n");
    printf("Choose an option: ");
    fflush(stdout);
}

int main(void)
{
    int choice;

    while (1) {
        print_menu();

        if (scanf("%d", &choice) != 1) {
            int c;
            while ((c = getchar()) != '\n' && c != EOF)
                ;
            if (feof(stdin)) {
                printf("\nEnd of input. Exiting safely.\n");
                return 0;
            }
            printf("\nInvalid input. Please enter a number.\n");
            continue;
        }

        switch (choice) {
        case 1:
            ipc_session_pipe();
            break;
        case 2:
            ipc_session_fifo();
            break;
        case 3:
            ipc_session_complete();
            break;
        case 4:
            co5_demo();
            break;
        case 5:
            co4_demo();
            break;
        case 6:
            co6_demo();
            break;
        case 7:
            co1_demo();
            break;
        case 8:
            printf("\nExiting safely. All resources cleaned up.\n");
            return 0;
        default:
            printf("\nInvalid option. Please choose 1-8.\n");
            break;
        }
    }

    return 0;
}
