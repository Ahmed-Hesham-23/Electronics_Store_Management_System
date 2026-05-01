#include "ProductFactory.h"

product* ProductFactory::createProduct(
    string type,
    string id, string name, string brand, double price, int stock,
    string model,
    double screenSize,
    int ram,
    string processor,
    int batteryLife,
    bool hasStylus
) {
    if (type == "Phone") {
        return new Phone(id, name, brand, price, stock, model, screenSize);
    }
    else if (type == "Laptop") {
        return new Laptop(id, name, brand, price, stock, model, processor, ram);
    }
    else if (type == "Tablet") {
        return new Tablet(id, name, brand, price, stock, model, screenSize, batteryLife, hasStylus);
    }

    return nullptr;
}