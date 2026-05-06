#include "ProductFactory.cpp"
#include "Inventory.cpp"
#include <iostream>
#include <cassert>

using namespace std;

// Helper to print section headers
void section(const string &title)
{
    cout << "\n";
    cout << "╔══════════════════════════════════════════╗\n";
    cout << "║  " << title << "\n";
    cout << "╚══════════════════════════════════════════╝\n";
}

int main()
{
    Inventory inv;

    // =========================================================================
    // 1. LOAD FROM FILE (if Warehouse.csv exists from a previous run)
    // =========================================================================
    section("1. LOAD FROM FILE");
    inv.loadFromFile();

    // =========================================================================
    // 2. CREATE PRODUCTS VIA FACTORY
    // =========================================================================
    section("2. CREATING PRODUCTS VIA ProductFactory");

    // --- SmartPhones ---
    SmartPhone *iphone = ProductFactory::createSmartPhone(
        "001", "iPhone 15 Pro", "Apple", "15 Pro",
        1299.99, 50,
        8, 256, 3274, 6.1, 48,
        "iOS 17",
        true, true, 120, "Apple A17 Pro");
    iphone->setDescription("Apple's flagship with titanium frame.");

    SmartPhone *samsung = ProductFactory::createSmartPhone(
        "002", "Galaxy S24 Ultra", "Samsung", "S24 Ultra",
        1199.99, 30,
        12, 512, 5000, 6.8, 200,
        "Android 14",
        true, true, 120, "Snapdragon 8 Gen 3");
    samsung->setDescription("Samsung's best with S-Pen support.");

    // --- Laptops ---
    Laptop *macbook = ProductFactory::createLaptop(
        "003", "MacBook Pro 14", "Apple", "MBP14",
        1999.99, 20,
        16, 512, true, 14.2,
        "macOS Sonoma", "Apple M3 Pro", "Integrated",
        70, LaptopFormFactor::ULTRABOOK,
        1.6, false, 3, true);
    macbook->setDescription("M3 Pro chip, stunning Liquid Retina display.");

    Laptop *dell = ProductFactory::createLaptop(
        "004", "Dell XPS 15", "Dell", "XPS15-9530",
        1749.99, 15,
        32, 1024, true, 15.6,
        "Windows 11", "Intel Core i9-13900H", "NVIDIA RTX 4060",
        86, LaptopFormFactor::WORKSTATION,
        1.86, true, 4, true);
    dell->setDescription("Powerful Windows laptop for creatives.");

    // --- Accessories ---
    Accessory *airpods = ProductFactory::createAccessory(
        "005", "AirPods Pro 2", "Apple", "MQTP3",
        249.99, 100,
        AccessoryCategory::HEADPHONES,
        "White", "Plastic",
        "Bluetooth 5.3", true);
    airpods->addCompatibility("iPhone 15 Pro");
    airpods->addCompatibility("MacBook Pro 14");
    airpods->setDescription("Active noise cancellation, spatial audio.");

    Accessory *usbhub = ProductFactory::createAccessory(
        "006", "Anker USB-C Hub", "Anker", "A83460A1",
        49.99, 200,
        AccessoryCategory::CABLE,
        "Black", "Aluminum",
        "USB-C", false);
    usbhub->addCompatibility("MacBook Pro 14");
    usbhub->addCompatibility("Dell XPS 15");

    cout << "Products created successfully.\n";

    // =========================================================================
    // 3. ADD PRODUCTS TO INVENTORY
    // =========================================================================
    section("3. ADDING PRODUCTS TO INVENTORY");

    inv += iphone;
    inv += samsung;
    inv += macbook;
    inv += dell;
    inv += airpods;
    inv += usbhub;

    // Test duplicate ID rejection
    cout << "\n-- Attempting to add duplicate ID (001):\n";
    SmartPhone *duplicate = ProductFactory::createSmartPhone(
        "001", "Fake Phone", "Unknown", "X",
        99.99, 1, 4, 64, 3000, 6.0, 12, "Android");
    bool added = inv.addProduct(duplicate);
    if (!added) delete duplicate; // clean up since inventory rejected it

    cout << "\nInventory summary: " << inv << "\n";

    // =========================================================================
    // 4. DISPLAY ALL PRODUCTS
    // =========================================================================
    section("4. DISPLAY ALL PRODUCTS");
    inv.printAll();

    // =========================================================================
    // 5. SEARCH FUNCTIONALITY
    // =========================================================================
    section("5. SEARCH TESTS");

    // By ID
    cout << "-- findById('003'):\n";
    Product *found = inv.findById("003");
    if (found) found->displayDetails();

    // By name keyword
    cout << "\n-- findByName('pro'):\n";
    auto byName = inv.findByName("pro");
    cout << "Found " << byName.size() << " result(s):\n";
    for (auto *p : byName)
        cout << "  -> " << p->getName() << "\n";

    // By brand
    cout << "\n-- findByBrand('apple'):\n";
    auto byBrand = inv.findByBrand("apple");
    cout << "Found " << byBrand.size() << " Apple product(s):\n";
    for (auto *p : byBrand)
        cout << "  -> " << p->getName() << "\n";

    // By price range
    cout << "\n-- findByPriceRange(200, 1300):\n";
    auto byPrice = inv.findByPriceRange(200, 1300);
    cout << "Found " << byPrice.size() << " product(s) in range $200-$1300:\n";
    for (auto *p : byPrice)
        cout << "  -> " << p->getName() << " ($" << p->getCurrentPrice() << ")\n";

    // =========================================================================
    // 6. SELL & RESTOCK
    // =========================================================================
    section("6. SELL & RESTOCK");

    cout << "-- Selling 5 iPhone 15 Pro (stock before: "
         << inv.findById("001")->getStockQuantity() << ")\n";
    inv.sell("001", 5);
    cout << "   Stock after: " << inv.findById("001")->getStockQuantity() << "\n";

    cout << "\n-- Restocking Galaxy S24 Ultra by 10 (stock before: "
         << inv.findById("002")->getStockQuantity() << ")\n";
    inv.restock("002", 10);
    cout << "   Stock after: " << inv.findById("002")->getStockQuantity() << "\n";

    cout << "\n-- Attempting to sell more than stock (sell 9999 of '006'):\n";
    inv.sell("006", 9999);

    cout << "\n-- Attempting to sell a nonexistent product:\n";
    inv.sell("999", 1);

    // =========================================================================
    // 7. DISCOUNTS
    // =========================================================================
    section("7. DISCOUNTS");

    cout << "-- Applying 10% discount to MacBook Pro 14:\n";
    cout << "   Before: $" << inv.findById("003")->getCurrentPrice() << "\n";
    inv.applyDiscount("003", 10, DiscountType::PERCENTAGE);
    cout << "   After : $" << inv.findById("003")->getCurrentPrice() << "\n";

    cout << "\n-- Applying $50 fixed discount to AirPods Pro 2:\n";
    cout << "   Before: $" << inv.findById("005")->getCurrentPrice() << "\n";
    inv.applyDiscount("005", 50, DiscountType::FIXED_AMOUNT);
    cout << "   After : $" << inv.findById("005")->getCurrentPrice() << "\n";

    cout << "\n-- Resetting discount on MacBook:\n";
    inv.findById("003")->resetDiscount();
    cout << "   Reset to: $" << inv.findById("003")->getCurrentPrice() << "\n";

    // =========================================================================
    // 8. STATUS CHANGES
    // =========================================================================
    section("8. STATUS CHANGES");

    cout << "-- Discontinuing Dell XPS 15:\n";
    inv.discontinue("004");
    cout << "   Status: " << inv.findById("004")->getStatusString() << "\n";

    cout << "\n-- Attempting to sell discontinued product:\n";
    try {
        inv.findById("004")->sell(1);
    } catch (const logic_error &e) {
        cout << "   Caught expected error: " << e.what() << "\n";
    }

    // =========================================================================
    // 9. TAX CALCULATION
    // =========================================================================
    section("9. TAX CALCULATION");

    Product *sp = inv.findById("001");
    cout << "iPhone 15 Pro price: $" << sp->getCurrentPrice() << "\n";
    cout << "Price with 14% tax: $" << fixed << setprecision(2)
         << sp->getFinalPrice(14) << "\n";

    // =========================================================================
    // 10. ACCESSORY COMPATIBILITY
    // =========================================================================
    section("10. ACCESSORY COMPATIBILITY");

    Accessory *ac = dynamic_cast<Accessory *>(inv.findById("005"));
    cout << "AirPods Pro 2 compatible with:\n";
    for (const auto &d : ac->getCompatibleDevices())
        cout << "  - " << d << "\n";

    cout << "\n-- Adding new compatibility: 'iPad Pro'\n";
    ac->addCompatibility("iPad Pro");
    cout << "-- Removing compatibility: 'MacBook Pro 14'\n";
    ac->removeCompatibility("MacBook Pro 14");
    cout << "Updated compatibility list:\n";
    for (const auto &d : ac->getCompatibleDevices())
        cout << "  - " << d << "\n";

    // =========================================================================
    // 11. INVENTORY STATISTICS
    // =========================================================================
    section("11. INVENTORY STATISTICS");

    cout << "Total products  : " << inv.count() << "\n";
    cout << "Available       : " << inv.countAvailable() << "\n";
    cout << "Out/Discontinued: " << inv.countOutOfStock() << "\n";
    cout << fixed << setprecision(2)
         << "Total value     : $" << inv.totalInventoryValue() << "\n";

    // =========================================================================
    // 12. OPERATOR OVERLOADING - REMOVE
    // =========================================================================
    section("12. REMOVE PRODUCT (operator-=)");

    cout << "-- Removing Anker USB-C Hub (ID: 006)\n";
    inv -= "006";
    cout << "   Products remaining: " << inv.count() << "\n";

    cout << "\n-- Removing nonexistent product (ID: 999):\n";
    inv -= "999";

    // =========================================================================
    // 13. VALIDATION / ERROR HANDLING
    // =========================================================================
    section("13. VALIDATION & ERROR HANDLING");

    cout << "-- Creating SmartPhone with negative price (should throw):\n";
    try {
        auto *bad = ProductFactory::createSmartPhone(
            "099", "BadPhone", "X", "X",
            -500, 10, 4, 64, 3000, 6.0, 12, "Android");
        delete bad;
    } catch (const invalid_argument &e) {
        cout << "   Caught: " << e.what() << "\n";
    }

    cout << "\n-- Creating Laptop with negative stock (should throw):\n";
    try {
        auto *bad = ProductFactory::createLaptop(
            "098", "BadLaptop", "X", "X",
            999, -5, 8, 256, true, 15.0,
            "Windows", "i5", "", 50, LaptopFormFactor::ULTRABOOK);
        delete bad;
    } catch (const invalid_argument &e) {
        cout << "   Caught: " << e.what() << "\n";
    }

    cout << "\n-- Invalid price range search:\n";
    try {
        inv.findByPriceRange(-10, 100);
    } catch (const invalid_argument &e) {
        cout << "   Caught: " << e.what() << "\n";
    }

    // =========================================================================
    // 14. SAVE TO FILE
    // =========================================================================
    section("14. SAVE TO FILE");
    // saveToFile is called automatically on every change,
    // but we print the final state clearly here.
    cout << "Final inventory saved to Warehouse.csv automatically.\n";
    cout << "Final summary: " << inv << "\n";

    cout << "\n✅ All tests completed.\n";
    return 0;
}