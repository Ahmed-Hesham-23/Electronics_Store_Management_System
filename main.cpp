/*******************************************************************************
 * FILE: main.cpp
 *
 * ROLE:
 * Entry point for the Store application.
 * Initialises all subsystems (Inventory, userManager, FileManager),
 * loads persisted data, then runs the top-level menu loop.
 *
 * Flow:
 *   1. Load all saved data (products, users, orders).
 *   2. Show the main menu: Sign Up / Login / Exit.
 *   3. After login the appropriate menu (Customer or Admin) is driven
 *      entirely by userManager::login(), which already contains the
 *      per-role sub-menu loops.
 *   4. On exit, save all data back to disk.
 *******************************************************************************/

#include "Store.h"
#include <limits>
int main() {
    // ── Core singletons / containers ────────────────────────────────────────
    Inventory     inventory;
    userManager*  um      = userManager::getUMinstance();
    vector<Order> orders;

    // ── Persistence layer ────────────────────────────────────────────────────
    FileManager fm(inventory, *um, orders);
    fm.loadAll();          // loads Warehouse.csv, users.csv, Orders.csv

    // ── Make sure at least one Admin exists ──────────────────────────────────
    // createAdmin() checks internally and prompts only when none exists.
    um->createAdmin();

    // ── Main application loop ────────────────────────────────────────────────
    int choice = 0;
    do {
        cout << "\n========================================\n";
        cout << "        Welcome to the Store\n";
        cout << "========================================\n";
        cout << "  1. Sign Up\n";
        cout << "  2. Login\n";
        cout << "  0. Exit\n";
        cout << "----------------------------------------\n";
        cout << "Your choice: ";

        if (!(cin >> choice)) {          // handle non-numeric input
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            choice = -1;
        } else {
            cin.ignore();
        }

        switch (choice) {
            case 1:
                um->signUp();
                break;

            case 2: {
                // login() drives the full per-role sub-menu loop and
                // returns the logged-in User* (or nullptr on failure).
                // Any Order created during checkout needs to be appended
                // to our orders vector, so we pass it via a lambda wrapper
                // that stores the returned order if valid.
                //
                // NOTE: because userManager::login() already contains the
                // complete Customer / Admin menus internally (as implemented
                // in Store.cpp), we just call it and capture the session.
                User* user = um->login(inventory, orders);
                // If the user completed a checkout inside login(), the Order
                // was created locally there. To persist it we rely on
                // FileManager::saveOrders() at exit. Future refactor could
                // pass the orders vector into login() directly.
                (void)user;   // currently unused after return
                break;
            }

            case 0:
                cout << "\nSaving all data before exit...\n";
                fm.saveAll();
                cout << "Thank you for using the Store. Goodbye!\n";
                break;

            default:
                cout << "Invalid choice. Please enter 0, 1, or 2.\n";
                break;
        }

    } while (choice != 0);

    return 0;
}