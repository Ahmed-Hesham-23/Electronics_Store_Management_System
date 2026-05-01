#include "Inventory.h"

void Inventory::addProduct(product *p) {
    products.push_back(p);
}

void Inventory::removeProduct(string id) {
    for (auto it = products.begin(); it != products.end(); ++it) {
        if ((*it)->getId() == id) {
            delete *it; // Clean up memory
            products.erase(it); //remove the pointer from the vector
            break;
        }
    }
}

product* Inventory::searchProduct(string id) {
    for (product *p: products) {
        if (p->getId() == id) return p;
    }
    return nullptr;
}

void Inventory::updateStock(string id, int qty) {
    product *p = searchProduct(id);
    if (p) {
        p->updateStock(qty);
    }
}

vector<product *> Inventory::getAllProducts() {
    return products;
}

};
