#ifndef IPC_SESSION_H
#define IPC_SESSION_H

#include "common.h"

/*
 * Integrated IPC Session — the core message-transfer system.
 *
 * This module implements a real end-to-end workflow:
 *   1. Create IPC resources (anonymous pipe or named FIFO)
 *   2. Fork sender and receiver processes
 *   3. Transfer actual user-entered messages byte-by-byte
 *   4. Track statistics with mutex protection
 *   5. Handle signals for notification and shutdown
 *   6. Clean up all resources properly
 *
 * It demonstrates CO-1 through CO-6 in one connected workflow.
 */

/* Run an integrated IPC session using an anonymous pipe */
void ipc_session_pipe(void);

/* Run an integrated IPC session using a named FIFO */
void ipc_session_fifo(void);

/* Run the complete end-to-end demonstration (pipe + FIFO + signals) */
void ipc_session_complete(void);

#endif /* IPC_SESSION_H */
