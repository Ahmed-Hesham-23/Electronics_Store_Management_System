/*******************************************************************************
 * FILE: ProductFactory.cpp
 *
 * ROLE:
 * This file implements the "Factory Method" Design Pattern. It acts as a
 * centralized manufacturing hub for all Product-derived objects.
 *
 * KEY RESPONSIBILITIES:
 * 1. Abstraction of Instantiation:
 *    Provides static methods (createSmartPhone, createLaptop, createAccessory)
 *    so the rest of the application doesn't need to know the specific
 *    constructor details of each subclass.
 *
 * 2. Pre-Creation Validation:
 *    Enforces business logic guards before an object is even born. It ensures
 *    that no product enters the system with invalid states (e.g., negative
 *    prices, zero RAM, or impossible battery capacities).
 *
 * 3. SKU Generation:
 *    Automatically handles the formatting of Stock Keeping Units (SKUs) based
 *    on the product type (e.g., prefixing "SP-" for phones or "LP-" for laptops),
 *    ensuring naming consistency across the database.
 *
 * 4. Memory Management:
 *    Handles the 'new' keyword allocation for heap-based objects, returning
 *    pointers that can be managed by the Store's inventory vectors.
 *******************************************************************************/
#include "Product.cpp"
#include <iostream>
#include <string>
#include <stdexcept>
using namespace std;
// =============================================================================
// ProductFactory
// =============================================================================
class ProductFactory {
    // Input user validation
    static void validateBase(double price, int stock, const string &context) {
        if (price <= 0)
            throw invalid_argument(context + ": price must be positive.");
        if (stock < 0)
            throw invalid_argument(context + ": stock cannot be negative.");
    }
public:
    static SmartPhone *createSmartPhone(
        const string &id,
        const string &name,
        const string &brand,
        const string &model,
        double price,
        int stock,
        int ram,
        int storage,
        int battery,
        double display,
        int camera,
        const string &os,
        bool has5G = false,
        bool hasNFC = false,
        int refreshRate = 60,
        const string &chip = "Unknown") {
        // Validation for base and special attributes
        validateBase(price, stock, "SmartPhone '" + name + "'");
        if (ram <= 0)
            throw invalid_argument("SmartPhone '" + name + "': RAM must be positive.");
        if (storage <= 0)
            throw invalid_argument("SmartPhone '" + name + "': storage must be positive.");
        if (battery <= 0)
            throw invalid_argument("SmartPhone '" + name + "': battery must be positive.");
        if (display <= 0)
            throw invalid_argument("SmartPhone '" + name + "': display size must be positive.");
        if (camera <= 0)
            throw invalid_argument("SmartPhone '" + name + "': camera MP must be positive.");
        if (refreshRate <= 0)
            throw invalid_argument("SmartPhone '" + name + "': refresh rate must be positive.");
        string sku = "SP-" + id;
        SmartPhone *sp = new SmartPhone(id, sku, name, brand, model,
                                        price, stock,
                                        ram, storage, battery,
                                        display, camera, os);
        sp->setHas5G(has5G);
        sp->setHasNFC(hasNFC);
        sp->setRefreshRate(refreshRate);
        sp->setChipset(chip);
        return sp;
    }
    static Laptop *createLaptop(
        const string &id,
        const string &name,
        const string &brand,
        const string &model,
        double price,
        int stock,
        int ram,
        int storage,
        bool ssd,
        double display,
        const string &os,
        const string &cpu,
        const string &gpu,
        int batteryWh,
        LaptopFormFactor form,
        double weightKg = 0.0,
        bool touchScreen = false,
        int usbPorts = 0,
        bool hasThunderbolt = false) {
        validateBase(price, stock, "Laptop '" + name + "'");
        if (ram <= 0)
            throw invalid_argument("Laptop '" + name + "': RAM must be positive.");
        if (storage <= 0)
            throw invalid_argument("Laptop '" + name + "': storage must be positive.");
        if (display <= 0)
            throw invalid_argument("Laptop '" + name + "': display size must be positive.");
        if (batteryWh <= 0)
            throw invalid_argument("Laptop '" + name + "': battery capacity must be positive.");
        if (weightKg < 0)
            throw invalid_argument("Laptop '" + name + "': weight cannot be negative.");
        if (usbPorts < 0)
            throw invalid_argument("Laptop '" + name + "': USB port count cannot be negative.");
        string sku = "LP-" + id;
        Laptop *lp = new Laptop(id, sku, name, brand, model,
                                price, stock,
                                ram, storage, ssd,
                                display, os, cpu, gpu,
                                batteryWh, form, weightKg);
        lp->setTouchscreen(touchScreen);
        lp->setUsbPorts(usbPorts);
        lp->setThunderbolt(hasThunderbolt);
        return lp;
    }
    static Accessory *createAccessory(
        const string &id,
        const string &name,
        const string &brand,
        const string &model,
        double price,
        int stock,
        AccessoryCategory cat,
        const string &color = "",
        const string &material = "",
        const string &connectivity = "",
        bool isWireless = false
        const vector<string> &compatibleDevices = {}) {
        validateBase(price, stock, "Accessory '" + name + "'");
        string sku = "AC-" + id;
        Accessory *ac = new Accessory(id, sku, name, brand, model,
                                      price, stock, cat, color, material);
        ac->setConnectivity(connectivity);
        ac->setWireless(isWireless);
        for (const string &device : compatibleDevices) {
            ac->addCompatibility(device);
        }
        return ac;
    }
};
