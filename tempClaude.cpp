// =============================================================================
//  e-Commerce Inventory Management System
//  Egypt University of Informatics – OOP (CSE122) Spring 2026
//  Demonstrates: Inheritance, Polymorphism, Encapsulation, Abstraction,
//                Singleton, Factory, Observer, Operator Overloading,
//                File Handling, STL containers, Error Handling
// =============================================================================

#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include <stdexcept>
#include <algorithm>
#include <map>
#include <set>
#include <fstream>
#include <sstream>
#include <functional>
#include <memory>
#include <ctime>
#include <limits>

using namespace std;

// =============================================================================
//  Forward declarations
// =============================================================================
class product;
class CartItem;
class Order;
class User;
class Customer;
class Admin;
class StoreManager;

// =============================================================================
//  Enumerations
// =============================================================================
enum class ProductStatus
{
    AVAILABLE,
    OUT_OF_STOCK,
    DISCONTINUED,
    COMING_SOON
};
enum class DiscountType
{
    PERCENTAGE,
    FIXED_AMOUNT
};
enum class LaptopFormFactor
{
    ULTRABOOK,
    GAMING,
    WORKSTATION,
    CONVERTIBLE,
    CHROMEBOOK
};
enum class AccessoryCategory
{
    CASE_COVER,
    CHARGER,
    CABLE,
    HEADPHONES,
    SCREEN_PROTECTOR,
    KEYBOARD,
    MOUSE
};
enum class OrderStatus
{
    PENDING,
    CONFIRMED,
    SHIPPED,
    DELIVERED,
    CANCELLED
};

// =============================================================================
//  OBSERVER PATTERN – IStockObserver interface + concrete observer
// =============================================================================
class IStockObserver
{
public:
    virtual void onStockRestored(const string &productName, int newQty) = 0;
    virtual ~IStockObserver() {}
};

// =============================================================================
//  ABSTRACT BASE CLASS – product
// =============================================================================
class product
{
private:
    string productId;
    string sku;
    string name;
    string brand;
    string model;
    string description;
    double mainPrice;
    double currentPrice;
    double discount;
    bool onSale;
    ProductStatus status;
    int stockQuantity;

    // Observer list (Observer pattern)
    vector<IStockObserver *> observers;

    void updateStatus()
    {
        if (status == ProductStatus::DISCONTINUED || status == ProductStatus::COMING_SOON)
            return;
        status = (stockQuantity > 0) ? ProductStatus::AVAILABLE : ProductStatus::OUT_OF_STOCK;
    }

    string statusToString() const
    {
        switch (status)
        {
        case ProductStatus::AVAILABLE:
            return "Available";
        case ProductStatus::OUT_OF_STOCK:
            return "Out of Stock";
        case ProductStatus::DISCONTINUED:
            return "Discontinued";
        case ProductStatus::COMING_SOON:
            return "Coming Soon";
        default:
            return "Unknown";
        }
    }

protected:
    void displayBase(ostream &out) const
    {
        out << "========================================\n";
        out << " Name : " << name << " (" << brand << " " << model << ")\n";
        out << " SKU  : " << sku << " | ID: " << productId << "\n";
        out << " Price: $" << fixed << setprecision(2) << currentPrice;
        if (onSale)
            out << " [ON SALE - was $" << fixed << setprecision(2) << mainPrice << "]";
        out << "\n";
        out << " Stock: " << stockQuantity
            << " | Status: " << statusToString() << "\n";
        if (!description.empty())
            out << " Desc : " << description << "\n";
    }

public:
    product()
        : mainPrice(0.0), currentPrice(0.0), discount(0.0),
          onSale(false), stockQuantity(0), status(ProductStatus::OUT_OF_STOCK) {}

    product(const string &id, const string &sku,
            const string &name, const string &brand,
            const string &model, double price, int stock)
        : productId(id), sku(sku), name(name), brand(brand),
          model(model), mainPrice(price), currentPrice(price),
          discount(0.0), onSale(false), stockQuantity(stock),
          status(stock > 0 ? ProductStatus::AVAILABLE : ProductStatus::OUT_OF_STOCK)
    {
        if (price < 0)
            throw invalid_argument("Price cannot be negative.");
        if (stock < 0)
            throw invalid_argument("Stock cannot be negative.");
    }

    // --- Pure virtual (Abstraction) ---
    virtual void displayDetails() const = 0;
    virtual string getType() const = 0; // returns "smartphone" / "laptop" / "accessory"

    virtual void print(ostream &out) const { displayBase(out); }

    virtual ~product() {}

    // --- Observer management ---
    void addObserver(IStockObserver *obs) { observers.push_back(obs); }
    void removeObserver(IStockObserver *obs)
    {
        observers.erase(remove(observers.begin(), observers.end(), obs), observers.end());
    }
    void notifyObservers()
    {
        for (auto *obs : observers)
            obs->onStockRestored(name, stockQuantity);
    }

    // --- Getters ---
    string getProductId() const { return productId; }
    string getSku() const { return sku; }
    string getName() const { return name; }
    string getBrand() const { return brand; }
    string getModel() const { return model; }
    string getDescription() const { return description; }
    double getMainPrice() const { return mainPrice; }
    double getCurrentPrice() const { return currentPrice; }
    double getDiscount() const { return discount; }
    bool getOnSale() const { return onSale; }
    int getStockQuantity() const { return stockQuantity; }
    ProductStatus getStatus() const { return status; }
    string getStatusString() const { return statusToString(); }

    // --- Setters & Mutators ---
    void setDescription(const string &d) { description = d; }

    void markComingSoon()
    {
        status = ProductStatus::COMING_SOON;
        stockQuantity = 0;
    }

    void setDiscountSale(double amount, DiscountType type)
    {
        if (type == DiscountType::PERCENTAGE)
        {
            if (amount < 0 || amount > 100)
                throw invalid_argument("Percentage must be between 0 and 100.");
            discount = mainPrice * (amount / 100.0);
        }
        else
        {
            if (amount < 0 || amount > mainPrice)
                throw invalid_argument("Fixed discount must be 0..mainPrice.");
            discount = amount;
        }
        currentPrice = mainPrice - discount;
        onSale = (discount > 0);
    }

    void resetDiscount()
    {
        discount = 0.0;
        currentPrice = mainPrice;
        onSale = false;
    }

    double getFinalPrice(double taxes = 0.0) const
    {
        if (taxes < 0)
            throw invalid_argument("Tax rate cannot be negative.");
        return currentPrice + currentPrice * (taxes / 100.0);
    }

    bool sell(int quantity)
    {
        if (quantity <= 0)
            throw invalid_argument("Quantity must be positive.");
        if (status == ProductStatus::DISCONTINUED)
            throw logic_error("Cannot sell a discontinued product.");
        if (status == ProductStatus::COMING_SOON)
            throw logic_error("Product is not yet available.");
        if (stockQuantity < quantity)
            return false;
        stockQuantity -= quantity;
        updateStatus();
        return true;
    }

    void restock(int quantity)
    {
        if (quantity <= 0)
            throw invalid_argument("Quantity must be positive.");
        if (status == ProductStatus::DISCONTINUED)
            throw logic_error("Cannot restock a discontinued product.");
        bool wasOutOfStock = (stockQuantity == 0);
        stockQuantity += quantity;
        updateStatus();
        if (wasOutOfStock && stockQuantity > 0)
            notifyObservers(); // Observer pattern notification
    }

    bool isAvailable() const { return status == ProductStatus::AVAILABLE && stockQuantity > 0; }

    void discontinue()
    {
        status = ProductStatus::DISCONTINUED;
        stockQuantity = 0;
    }

    // --- Operator Overloading ---
    bool operator==(const product &other) const { return productId == other.productId; }
    bool operator<(const product &other) const { return currentPrice < other.currentPrice; }

    friend ostream &operator<<(ostream &out, const product &p)
    {
        p.print(out);
        return out;
    }
};

// =============================================================================
//  smartPhone  (inherits product)
// =============================================================================
class smartPhone : public product
{
private:
    int ram, storage, batteryMah, cameraMP, refreshRateHz;
    double inches;
    string phoneOS, chipset;
    bool has5G, hasNFC;

public:
    smartPhone(const string &id, const string &sku,
               const string &name, const string &brand,
               const string &model, double price, int stock,
               int ram, int storage, int battery,
               double display, int camera, const string &os)
        : product(id, sku, name, brand, model, price, stock),
          ram(ram), storage(storage), batteryMah(battery),
          inches(display), cameraMP(camera), phoneOS(os),
          has5G(false), hasNFC(false), refreshRateHz(60) {}

    void setHas5G(bool v) { has5G = v; }
    void setHasNFC(bool v) { hasNFC = v; }
    void setRefreshRate(int hz)
    {
        if (hz <= 0)
            throw invalid_argument("Refresh rate must be positive.");
        refreshRateHz = hz;
    }
    void setChipset(const string &c) { chipset = c; }
    void setOS(const string &os) { phoneOS = os; }

    int getRam() const { return ram; }
    int getStorage() const { return storage; }
    int getBattery() const { return batteryMah; }
    double getDisplay() const { return inches; }
    int getCamera() const { return cameraMP; }
    string getOS() const { return phoneOS; }
    bool getHas5G() const { return has5G; }
    bool getHasNFC() const { return hasNFC; }
    int getRefreshRate() const { return refreshRateHz; }
    string getChipset() const { return chipset; }

    bool isFlagship() const { return ram >= 12 && cameraMP >= 100; }
    bool isGamingPhone() const { return ram >= 8 && batteryMah >= 4500; }
    bool isMidRange() const
    {
        double p = getCurrentPrice();
        return p >= 300.0 && p < 700.0;
    }

    string getType() const override { return "smartphone"; }
    void displayDetails() const override { cout << *this; }

    void print(ostream &out) const override
    {
        displayBase(out);
        out << " --- Smartphone Details ---\n";
        if (!chipset.empty())
            out << " Chipset    : " << chipset << "\n";
        out << " RAM        : " << ram << " GB\n";
        out << " Storage    : " << storage << " GB\n";
        out << " Battery    : " << batteryMah << " mAh\n";
        out << " Display    : " << inches << "\" @ " << refreshRateHz << " Hz\n";
        out << " Camera     : " << cameraMP << " MP\n";
        out << " OS         : " << phoneOS << "\n";
        out << " 5G         : " << (has5G ? "Yes" : "No") << "\n";
        out << " NFC        : " << (hasNFC ? "Yes" : "No") << "\n";
        out << " Flagship   : " << (isFlagship() ? "Yes" : "No") << "\n";
        out << " Gaming     : " << (isGamingPhone() ? "Yes" : "No") << "\n";
        out << "========================================\n";
    }
};

// =============================================================================
//  laptop  (inherits product)
// =============================================================================
class laptop : public product
{
private:
    int ramGB, storageGB, batteryWh, usbPorts;
    bool isSSD, hasTouchscreen, hasThunderbolt;
    double displayInches, weightKg;
    string operatingSystem, cpu, gpu;
    LaptopFormFactor formFactor;

    string formFactorToString() const
    {
        switch (formFactor)
        {
        case LaptopFormFactor::ULTRABOOK:
            return "Ultrabook";
        case LaptopFormFactor::GAMING:
            return "Gaming";
        case LaptopFormFactor::WORKSTATION:
            return "Workstation";
        case LaptopFormFactor::CONVERTIBLE:
            return "Convertible (2-in-1)";
        case LaptopFormFactor::CHROMEBOOK:
            return "Chromebook";
        default:
            return "Unknown";
        }
    }

public:
    laptop(const string &id, const string &sku,
           const string &name, const string &brand,
           const string &model, double price, int stock,
           int ram, int storage, bool ssd,
           double display, const string &os,
           const string &cpu, const string &gpu,
           int batteryWh, LaptopFormFactor form,
           double weight = 0.0)
        : product(id, sku, name, brand, model, price, stock),
          ramGB(ram), storageGB(storage), isSSD(ssd),
          displayInches(display), operatingSystem(os),
          cpu(cpu), gpu(gpu), batteryWh(batteryWh), formFactor(form),
          hasTouchscreen(false), usbPorts(0), hasThunderbolt(false),
          weightKg(weight) {}

    void setTouchscreen(bool v) { hasTouchscreen = v; }
    void setUsbPorts(int n)
    {
        if (n < 0)
            throw invalid_argument("USB port count cannot be negative.");
        usbPorts = n;
    }
    void setThunderbolt(bool v) { hasThunderbolt = v; }

    int getRam() const { return ramGB; }
    int getStorage() const { return storageGB; }
    bool getSSD() const { return isSSD; }
    double getDisplay() const { return displayInches; }
    string getOS() const { return operatingSystem; }
    string getCPU() const { return cpu; }
    string getGPU() const { return gpu; }
    int getBatteryWh() const { return batteryWh; }
    LaptopFormFactor getFormFactor() const { return formFactor; }

    bool isWorkhorseReady() const { return ramGB >= 16 && isSSD && storageGB >= 512; }
    bool isUltrathin() const { return weightKg > 0 && weightKg < 1.5; }

    string getType() const override { return "laptop"; }
    void displayDetails() const override { cout << *this; }

    void print(ostream &out) const override
    {
        displayBase(out);
        out << " --- Laptop Details ---\n";
        out << " Form Factor: " << formFactorToString() << "\n";
        out << " CPU        : " << cpu << "\n";
        out << " GPU        : " << (gpu.empty() ? "Integrated" : gpu) << "\n";
        out << " RAM        : " << ramGB << " GB\n";
        out << " Storage    : " << storageGB << " GB " << (isSSD ? "(SSD)" : "(HDD)") << "\n";
        out << " Display    : " << displayInches << "\"\n";
        out << " Battery    : " << batteryWh << " Wh\n";
        out << " OS         : " << operatingSystem << "\n";
        out << " Workhorse  : " << (isWorkhorseReady() ? "Yes" : "No") << "\n";
        out << "========================================\n";
    }
};

// =============================================================================
//  accessory  (inherits product)
// =============================================================================
class accessory : public product
{
private:
    AccessoryCategory category;
    vector<string> compatibleWith;
    string color, material, connectivity;
    bool isWireless;

    string categoryToString() const
    {
        switch (category)
        {
        case AccessoryCategory::CASE_COVER:
            return "Case / Cover";
        case AccessoryCategory::CHARGER:
            return "Charger";
        case AccessoryCategory::CABLE:
            return "Cable";
        case AccessoryCategory::HEADPHONES:
            return "Headphones";
        case AccessoryCategory::SCREEN_PROTECTOR:
            return "Screen Protector";
        case AccessoryCategory::KEYBOARD:
            return "Keyboard";
        case AccessoryCategory::MOUSE:
            return "Mouse";
        default:
            return "Other";
        }
    }

public:
    accessory(const string &id, const string &sku,
              const string &name, const string &brand,
              const string &model, double price, int stock,
              AccessoryCategory cat,
              const string &color = "", const string &material = "")
        : product(id, sku, name, brand, model, price, stock),
          category(cat), color(color), material(material), isWireless(false) {}

    void addCompatibility(const string &device)
    {
        if (!isCompatibleWith(device))
            compatibleWith.push_back(device);
    }
    void removeCompatibility(const string &device)
    {
        compatibleWith.erase(remove(compatibleWith.begin(), compatibleWith.end(), device),
                             compatibleWith.end());
    }
    bool isCompatibleWith(const string &device) const
    {
        return find(compatibleWith.begin(), compatibleWith.end(), device) != compatibleWith.end();
    }
    const vector<string> &getCompatibleDevices() const { return compatibleWith; }

    void setConnectivity(const string &c) { connectivity = c; }
    void setWireless(bool v) { isWireless = v; }
    void setColor(const string &c) { color = c; }
    void setMaterial(const string &m) { material = m; }

    AccessoryCategory getCategory() const { return category; }
    string getColor() const { return color; }
    string getMaterial() const { return material; }
    string getConnectivity() const { return connectivity; }
    bool getIsWireless() const { return isWireless; }

    string getType() const override { return "accessory"; }
    void displayDetails() const override { cout << *this; }

    void print(ostream &out) const override
    {
        displayBase(out);
        out << " --- Accessory Details ---\n";
        out << " Category    : " << categoryToString() << "\n";
        if (!color.empty())
            out << " Color       : " << color << "\n";
        if (!material.empty())
            out << " Material    : " << material << "\n";
        if (!connectivity.empty())
            out << " Connectivity: " << connectivity << "\n";
        out << " Wireless    : " << (isWireless ? "Yes" : "No") << "\n";
        if (!compatibleWith.empty())
        {
            out << " Compatible  : ";
            for (size_t i = 0; i < compatibleWith.size(); ++i)
            {
                if (i)
                    out << ", ";
                out << compatibleWith[i];
            }
            out << "\n";
        }
        out << "========================================\n";
    }
};

// =============================================================================
//  CartItem  – value type representing one line in a cart / order
// =============================================================================
struct CartItem
{
    product *item;
    int quantity;

    CartItem(product *p, int q) : item(p), quantity(q) {}

    double subtotal() const { return item->getCurrentPrice() * quantity; }

    // Operator overloading
    bool operator==(const CartItem &other) const { return item == other.item; }
    bool operator<(const CartItem &other) const { return subtotal() < other.subtotal(); }

    friend ostream &operator<<(ostream &out, const CartItem &ci)
    {
        out << " - " << left << setw(28) << ci.item->getName()
            << " x" << setw(3) << ci.quantity
            << "  @$" << fixed << setprecision(2) << setw(8) << ci.item->getCurrentPrice()
            << "  = $" << fixed << setprecision(2) << ci.subtotal();
        return out;
    }
};

// =============================================================================
//  Cart  – encapsulates all shopping-cart logic
//
//  Responsibilities:
//    • Add / remove / update items             (CRUD)
//    • Enforce stock constraints               (Business logic)
//    • Calculate totals with optional tax      (Processing)
//    • Pretty-print contents                   (Reporting)
//    • Apply / remove a coupon discount        (Promotion)
//    • Produce an immutable snapshot for Order (Checkout)
//    • Operator overloads: +=, -=, [], <<      (Readability)
// =============================================================================
class Cart
{
private:
    vector<CartItem> items;
    string ownerId;        // customer ID – for display / audit
    double couponDiscount; // flat $ off the total (0 = none)
    string couponCode;

    // Linear find by product pointer
    int indexOf(const product *p) const
    {
        for (int i = 0; i < (int)items.size(); ++i)
            if (items[i].item == p)
                return i;
        return -1;
    }

    // Linear find by product ID string
    int indexOfId(const string &id) const
    {
        for (int i = 0; i < (int)items.size(); ++i)
            if (items[i].item->getProductId() == id)
                return i;
        return -1;
    }

public:
    explicit Cart(const string &ownerId = "")
        : ownerId(ownerId), couponDiscount(0.0) {}

    // Non-copyable (owns logical state tied to a customer session)
    Cart(const Cart &) = default; // allow copy for Order snapshot
    Cart &operator=(const Cart &) = default;

    // -------------------------------------------------------------------------
    //  Core CRUD
    // -------------------------------------------------------------------------

    // Add qty units of product p.  Merges with existing line if present.
    void add(product *p, int qty)
    {
        if (!p)
            throw invalid_argument("Null product pointer.");
        if (qty <= 0)
            throw invalid_argument("Quantity must be positive.");
        if (!p->isAvailable())
            throw logic_error("'" + p->getName() + "' is not available.");

        int idx = indexOf(p);
        int newQty = (idx == -1 ? 0 : items[idx].quantity) + qty;

        if (newQty > p->getStockQuantity())
            throw logic_error("Only " + to_string(p->getStockQuantity()) +
                              " unit(s) of '" + p->getName() + "' in stock.");
        if (idx == -1)
            items.push_back(CartItem(p, qty));
        else
            items[idx].quantity = newQty;
    }

    // Remove entire line by product ID
    void remove(const string &productId)
    {
        int idx = indexOfId(productId);
        if (idx == -1)
            throw logic_error("Product '" + productId + "' is not in the cart.");
        items.erase(items.begin() + idx);
    }

    // Update quantity of an existing line (0 = remove)
    void updateQty(const string &productId, int newQty)
    {
        if (newQty < 0)
            throw invalid_argument("Quantity cannot be negative.");
        int idx = indexOfId(productId);
        if (idx == -1)
            throw logic_error("Product '" + productId + "' is not in the cart.");
        if (newQty == 0)
        {
            items.erase(items.begin() + idx);
            return;
        }
        if (newQty > items[idx].item->getStockQuantity())
            throw logic_error("Not enough stock for '" + items[idx].item->getName() + "'.");
        items[idx].quantity = newQty;
    }

    // Empty the cart
    void clear()
    {
        items.clear();
        couponDiscount = 0.0;
        couponCode.clear();
    }

    // -------------------------------------------------------------------------
    //  Coupon / promotion
    // -------------------------------------------------------------------------
    void applyCoupon(const string &code, double flatDiscount)
    {
        if (flatDiscount <= 0)
            throw invalid_argument("Discount must be positive.");
        couponCode = code;
        couponDiscount = flatDiscount;
        cout << "  Coupon '" << code << "' applied – $"
             << fixed << setprecision(2) << flatDiscount << " off.\n";
    }

    void removeCoupon()
    {
        couponCode.clear();
        couponDiscount = 0.0;
    }

    // -------------------------------------------------------------------------
    //  Totals
    // -------------------------------------------------------------------------
    double subtotal() const
    {
        double t = 0;
        for (auto &ci : items)
            t += ci.subtotal();
        return t;
    }

    // Total after coupon, before tax
    double discountedTotal() const
    {
        double t = subtotal() - couponDiscount;
        return (t < 0) ? 0.0 : t;
    }

    // Final amount including tax (tax in %)
    double grandTotal(double taxPct = 0.0) const
    {
        if (taxPct < 0)
            throw invalid_argument("Tax rate cannot be negative.");
        double dt = discountedTotal();
        return dt + dt * (taxPct / 100.0);
    }

    // -------------------------------------------------------------------------
    //  Queries / iteration
    // -------------------------------------------------------------------------
    bool isEmpty() const { return items.empty(); }
    int itemCount() const { return (int)items.size(); }
    int totalUnits() const
    {
        int n = 0;
        for (auto &ci : items)
            n += ci.quantity;
        return n;
    }

    bool contains(const string &productId) const { return indexOfId(productId) != -1; }

    // Read-only access to a specific line (throws if not found)
    const CartItem &getItem(const string &productId) const
    {
        int idx = indexOfId(productId);
        if (idx == -1)
            throw logic_error("Product '" + productId + "' not in cart.");
        return items[idx];
    }

    const vector<CartItem> &getItems() const { return items; }

    // Return a copy sorted by subtotal descending (for reporting)
    vector<CartItem> sortedBySubtotal() const
    {
        vector<CartItem> sorted = items;
        sort(sorted.begin(), sorted.end(),
             [](const CartItem &a, const CartItem &b)
             { return a.subtotal() > b.subtotal(); });
        return sorted;
    }

    // -------------------------------------------------------------------------
    //  Checkout snapshot  – validates stock then returns items for Order
    // -------------------------------------------------------------------------
    vector<CartItem> checkoutSnapshot() const
    {
        if (items.empty())
            throw logic_error("Cart is empty. Nothing to checkout.");
        for (auto &ci : items)
            if (ci.item->getStockQuantity() < ci.quantity)
                throw logic_error("Insufficient stock for '" + ci.item->getName() + "'.");
        return items; // caller commits stock deduction
    }

    // -------------------------------------------------------------------------
    //  Display
    // -------------------------------------------------------------------------
    void print(double taxPct = 0.0) const
    {
        if (items.empty())
        {
            cout << "  Cart is empty.\n";
            return;
        }

        cout << "\n  ╔══════════════════════════════════════════════════╗\n";
        cout << "    SHOPPING CART  –  " << ownerId << "\n";
        cout << "  ╠══════════════════════════════════════════════════╣\n";
        for (auto &ci : items)
            cout << "  " << ci << "\n";
        cout << "  ╠══════════════════════════════════════════════════╣\n";
        cout << "    Subtotal       : $" << fixed << setprecision(2) << subtotal() << "\n";
        if (couponDiscount > 0)
        {
            cout << "    Coupon (" << setw(8) << left << couponCode << ") : -$"
                 << fixed << setprecision(2) << couponDiscount << "\n";
            cout << "    After coupon   : $" << fixed << setprecision(2) << discountedTotal() << "\n";
        }
        if (taxPct > 0)
            cout << "    Tax (" << fixed << setprecision(1) << taxPct << "%)      : $"
                 << fixed << setprecision(2) << (discountedTotal() * taxPct / 100.0) << "\n";
        cout << "    GRAND TOTAL    : $" << fixed << setprecision(2) << grandTotal(taxPct) << "\n";
        cout << "    Items: " << itemCount() << " line(s), " << totalUnits() << " unit(s)\n";
        cout << "  ╚══════════════════════════════════════════════════╝\n";
    }

    // -------------------------------------------------------------------------
    //  Operator overloading
    // -------------------------------------------------------------------------

    // cart += product*   →  add 1 unit
    Cart &operator+=(product *p)
    {
        add(p, 1);
        return *this;
    }

    // cart -= productId  →  remove entire line
    Cart &operator-=(const string &productId)
    {
        remove(productId);
        return *this;
    }

    // cart["SP001"]      →  const reference to CartItem (throws if absent)
    const CartItem &operator[](const string &productId) const
    {
        return getItem(productId);
    }

    // Comparison: carts equal when they have the same lines
    bool operator==(const Cart &other) const { return items == other.items; }

    friend ostream &operator<<(ostream &out, const Cart &c)
    {
        if (c.isEmpty())
        {
            out << "  [Cart is empty]\n";
            return out;
        }
        out << "  Cart (" << c.ownerId << "): "
            << c.itemCount() << " line(s), "
            << c.totalUnits() << " unit(s), "
            << "subtotal $" << fixed << setprecision(2) << c.subtotal();
        if (c.couponDiscount > 0)
            out << " [coupon -$" << fixed << setprecision(2) << c.couponDiscount << "]";
        return out;
    }
};

class Order
{
private:
    static int nextOrderId;
    int orderId;
    string customerId;
    vector<CartItem> items;
    double totalAmount;
    OrderStatus status;
    string timestamp;

    string statusToString() const
    {
        switch (status)
        {
        case OrderStatus::PENDING:
            return "Pending";
        case OrderStatus::CONFIRMED:
            return "Confirmed";
        case OrderStatus::SHIPPED:
            return "Shipped";
        case OrderStatus::DELIVERED:
            return "Delivered";
        case OrderStatus::CANCELLED:
            return "Cancelled";
        default:
            return "Unknown";
        }
    }

    string currentTimestamp() const
    {
        time_t now = time(nullptr);
        char buf[20];
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&now));
        return string(buf);
    }

public:
    Order(const string &custId, const vector<CartItem> &cartItems)
        : orderId(++nextOrderId), customerId(custId),
          items(cartItems), status(OrderStatus::PENDING)
    {
        totalAmount = 0;
        for (auto &ci : items)
            totalAmount += ci.subtotal();
        timestamp = currentTimestamp();
    }

    int getOrderId() const { return orderId; }
    string getCustomerId() const { return customerId; }
    double getTotal() const { return totalAmount; }
    OrderStatus getStatus() const { return status; }
    string getTimestamp() const { return timestamp; }
    const vector<CartItem> &getItems() const { return items; }

    void setStatus(OrderStatus s) { status = s; }

    // Operator overloading: compare orders by total
    bool operator<(const Order &other) const { return totalAmount < other.totalAmount; }

    friend ostream &operator<<(ostream &out, const Order &o)
    {
        out << "╔══════════════════════════════════════╗\n";
        out << "  Order #" << o.orderId
            << "  [" << o.statusToString() << "]\n";
        out << "  Customer : " << o.customerId << "\n";
        out << "  Date     : " << o.timestamp << "\n";
        out << "  ──────────────────────────────────── \n";
        for (auto &ci : o.items)
            out << ci << "\n";
        out << "  ──────────────────────────────────── \n";
        out << "  TOTAL    : $" << fixed << setprecision(2) << o.totalAmount << "\n";
        out << "╚══════════════════════════════════════╝\n";
        return out;
    }

    // Serialize to CSV line for file storage
    string toCSV() const
    {
        ostringstream ss;
        ss << orderId << "|" << customerId << "|"
           << fixed << setprecision(2) << totalAmount << "|"
           << statusToString() << "|" << timestamp;
        return ss.str();
    }
};

int Order::nextOrderId = 0;

// =============================================================================
//  ABSTRACT USER BASE CLASS  (Abstraction + Inheritance)
// =============================================================================
class User : public IStockObserver
{
protected:
    string userId;
    string name;
    string email;
    string passwordHash; // simple hash for demo

    static string simpleHash(const string &s)
    {
        size_t h = hash<string>{}(s);
        ostringstream ss;
        ss << hex << h;
        return ss.str();
    }

public:
    User(const string &id, const string &name,
         const string &email, const string &password)
        : userId(id), name(name), email(email),
          passwordHash(simpleHash(password)) {}

    virtual ~User() {}

    // Pure virtual – Abstraction
    virtual string getRole() const = 0;
    virtual void showMenu() const = 0;

    bool checkPassword(const string &pwd) const
    {
        return passwordHash == simpleHash(pwd);
    }

    string getUserId() const { return userId; }
    string getName() const { return name; }
    string getEmail() const { return email; }

    void setName(const string &n) { name = n; }
    void setEmail(const string &e) { email = e; }

    // IStockObserver implementation (Observer pattern)
    void onStockRestored(const string &productName, int newQty) override
    {
        cout << "\n[NOTIFICATION for " << name << "] "
             << productName << " is back in stock! (" << newQty << " units)\n";
    }

    // Operator overloading: compare users by ID
    bool operator==(const User &other) const { return userId == other.userId; }

    friend ostream &operator<<(ostream &out, const User &u)
    {
        out << "[" << u.getRole() << "] " << u.name
            << " | ID: " << u.userId << " | " << u.email;
        return out;
    }
};

// =============================================================================
//  Customer  (inherits User)
// =============================================================================
class Customer : public User
{
private:
    Cart cart;
    vector<Order> orderHistory;
    string address;
    double totalSpent;

public:
    Customer(const string &id, const string &name,
             const string &email, const string &password,
             const string &address = "")
        : User(id, name, email, password),
          cart(id), // Cart knows who owns it
          address(address), totalSpent(0.0)
    {
    }

    string getRole() const override { return "Customer"; }
    string getAddress() const { return address; }
    double getTotalSpent() const { return totalSpent; }
    const vector<Order> &getOrderHistory() const { return orderHistory; }

    void setAddress(const string &a) { address = a; }

    // --- Cart delegation ---
    Cart &getCart() { return cart; }
    const Cart &getCart() const { return cart; }

    void addToCart(product *p, int qty) { cart.add(p, qty); }
    void removeFromCart(const string &id) { cart.remove(id); }
    void updateCartQty(const string &id, int qty) { cart.updateQty(id, qty); }
    void clearCart() { cart.clear(); }
    void applyCoupon(const string &code, double disc) { cart.applyCoupon(code, disc); }

    void viewCart(double taxPct = 0.0) const { cart.print(taxPct); }

    bool cartIsEmpty() const { return cart.isEmpty(); }
    double cartTotal() const { return cart.subtotal(); }

    // Checkout: validate → deduct stock → create Order → clear cart
    Order checkout(double taxPct = 0.0)
    {
        // checkoutSnapshot() throws if empty or stock insufficient
        vector<CartItem> snapshot = cart.checkoutSnapshot();

        for (auto &ci : snapshot)
            ci.item->sell(ci.quantity);

        Order o(userId, snapshot);
        orderHistory.push_back(o);
        totalSpent += cart.grandTotal(taxPct);
        cart.clear();
        return o;
    }

    void printOrderHistory() const
    {
        if (orderHistory.empty())
        {
            cout << "  No orders placed yet.\n";
            return;
        }
        for (auto &o : orderHistory)
            cout << o;
    }

    void showMenu() const override
    {
        cout << "\n  CUSTOMER MENU\n";
        cout << "  [1] Browse Products\n";
        cout << "  [2] Search Product\n";
        cout << "  [3] View Cart\n";
        cout << "  [4] Add to Cart\n";
        cout << "  [5] Remove from Cart\n";
        cout << "  [6] Update Item Quantity\n";
        cout << "  [7] Apply Coupon\n";
        cout << "  [8] Checkout\n";
        cout << "  [9] My Order History\n";
        cout << "  [10] My Profile\n";
        cout << "  [0] Logout\n";
    }
};

// =============================================================================
//  Admin  (inherits User)
// =============================================================================
class Admin : public User
{
private:
    string department;

public:
    Admin(const string &id, const string &name,
          const string &email, const string &password,
          const string &dept = "General")
        : User(id, name, email, password), department(dept) {}

    string getRole() const override { return "Admin"; }
    string getDepartment() const { return department; }

    void showMenu() const override
    {
        cout << "\n  ADMIN MENU\n";
        cout << "  [1]  Add Product\n";
        cout << "  [2]  Remove Product\n";
        cout << "  [3]  Restock Product\n";
        cout << "  [4]  Set Discount\n";
        cout << "  [5]  Discontinue Product\n";
        cout << "  [6]  View All Products\n";
        cout << "  [7]  View All Orders\n";
        cout << "  [8]  View All Customers\n";
        cout << "  [9]  Sales Report\n";
        cout << "  [10] Out-of-Stock Report\n";
        cout << "  [11] Inventory Summary\n";
        cout << "  [12] Products Sorted by Price\n";
        cout << "  [13] Save Data to Files\n";
        cout << "  [14] Load Products from File\n";
        cout << "  [0]  Logout\n";
    }
};

// =============================================================================
//  FACTORY PATTERN – ProductFactory
// =============================================================================
class ProductFactory
{
public:
    // Creates a smartphone with sensible defaults
    static smartPhone *createSmartPhone(const string &id,
                                        const string &name,
                                        const string &brand,
                                        double price, int stock,
                                        int ram, int storage,
                                        const string &os)
    {
        string sku = "SP-" + id;
        return new smartPhone(id, sku, name, brand, "std",
                              price, stock, ram, storage, 4000, 6.5, 48, os);
    }

    static laptop *createLaptop(const string &id,
                                const string &name,
                                const string &brand,
                                double price, int stock,
                                int ram, int storage,
                                const string &cpu)
    {
        string sku = "LP-" + id;
        return new laptop(id, sku, name, brand, "std", price, stock,
                          ram, storage, true, 15.6, "Windows 11",
                          cpu, "", 56, LaptopFormFactor::ULTRABOOK, 1.8);
    }

    static accessory *createAccessory(const string &id,
                                      const string &name,
                                      const string &brand,
                                      double price, int stock,
                                      AccessoryCategory cat)
    {
        string sku = "AC-" + id;
        return new accessory(id, sku, name, brand, "std", price, stock, cat);
    }
};

// =============================================================================
//  Inventory  – owns all products; central product management class
//  Encapsulates: storage, search, CRUD, file I/O, and reporting for products.
// =============================================================================
class Inventory
{
private:
    vector<product *> products; // owns the pointers
    const string FILE_PATH = "products.txt";

    // Internal linear search by ID
    int indexOf(const string &id) const
    {
        for (int i = 0; i < (int)products.size(); ++i)
            if (products[i]->getProductId() == id)
                return i;
        return -1;
    }

public:
    // --- No copy, owns raw pointers ---
    Inventory() = default;
    Inventory(const Inventory &) = delete;
    Inventory &operator=(const Inventory &) = delete;

    ~Inventory()
    {
        for (auto *p : products)
            delete p;
    }

    // -------------------------------------------------------------------------
    //  CRUD
    // -------------------------------------------------------------------------

    // Add a product (takes ownership)
    bool addProduct(product *p)
    {
        if (!p)
            throw invalid_argument("Null product pointer.");
        if (indexOf(p->getProductId()) != -1)
        {
            cout << "  [Inventory] Product ID '" << p->getProductId() << "' already exists.\n";
            return false;
        }
        products.push_back(p);
        return true;
    }

    // Remove by ID – returns true if found & removed
    bool removeProduct(const string &id)
    {
        int idx = indexOf(id);
        if (idx == -1)
            return false;
        delete products[idx];
        products.erase(products.begin() + idx);
        return true;
    }

    // -------------------------------------------------------------------------
    //  Lookup
    // -------------------------------------------------------------------------
    product *findById(const string &id) const
    {
        int idx = indexOf(id);
        return (idx == -1) ? nullptr : products[idx];
    }

    // Case-insensitive name search – returns all matches
    vector<product *> findByName(const string &keyword) const
    {
        vector<product *> results;
        string kw = keyword;
        transform(kw.begin(), kw.end(), kw.begin(), ::tolower);
        for (auto *p : products)
        {
            string n = p->getName();
            transform(n.begin(), n.end(), n.begin(), ::tolower);
            if (n.find(kw) != string::npos)
                results.push_back(p);
        }
        return results;
    }

    // Filter by type string ("smartphone" / "laptop" / "accessory")
    vector<product *> findByType(const string &type) const
    {
        vector<product *> results;
        for (auto *p : products)
            if (p->getType() == type)
                results.push_back(p);
        return results;
    }

    // Filter by brand (case-insensitive)
    vector<product *> findByBrand(const string &brand) const
    {
        vector<product *> results;
        string br = brand;
        transform(br.begin(), br.end(), br.begin(), ::tolower);
        for (auto *p : products)
        {
            string pb = p->getBrand();
            transform(pb.begin(), pb.end(), pb.begin(), ::tolower);
            if (pb == br)
                results.push_back(p);
        }
        return results;
    }

    // Filter by price range
    vector<product *> findByPriceRange(double minPrice, double maxPrice) const
    {
        if (minPrice < 0 || maxPrice < minPrice)
            throw invalid_argument("Invalid price range.");
        vector<product *> results;
        for (auto *p : products)
            if (p->getCurrentPrice() >= minPrice && p->getCurrentPrice() <= maxPrice)
                results.push_back(p);
        return results;
    }

    // -------------------------------------------------------------------------
    //  Stock operations (delegate to product, keep logic centralised)
    // -------------------------------------------------------------------------
    bool restock(const string &id, int qty)
    {
        product *p = findById(id);
        if (!p)
            return false;
        p->restock(qty); // product notifies observers internally
        return true;
    }

    bool sell(const string &id, int qty)
    {
        product *p = findById(id);
        if (!p)
            return false;
        return p->sell(qty);
    }

    bool applyDiscount(const string &id, double amount, DiscountType type)
    {
        product *p = findById(id);
        if (!p)
            return false;
        p->setDiscountSale(amount, type);
        return true;
    }

    bool discontinue(const string &id)
    {
        product *p = findById(id);
        if (!p)
            return false;
        p->discontinue();
        return true;
    }

    // -------------------------------------------------------------------------
    //  Accessors / statistics
    // -------------------------------------------------------------------------
    int count() const { return (int)products.size(); }
    bool isEmpty() const { return products.empty(); }
    const vector<product *> &all() const { return products; }

    int countAvailable() const
    {
        int n = 0;
        for (auto *p : products)
            if (p->isAvailable())
                ++n;
        return n;
    }

    int countOutOfStock() const
    {
        int n = 0;
        for (auto *p : products)
            if (!p->isAvailable())
                ++n;
        return n;
    }

    double totalInventoryValue() const
    {
        double v = 0;
        for (auto *p : products)
            v += p->getCurrentPrice() * p->getStockQuantity();
        return v;
    }

    // -------------------------------------------------------------------------
    //  Reporting (print to stdout)
    // -------------------------------------------------------------------------
    void printAll() const
    {
        if (products.empty())
        {
            cout << "  No products in inventory.\n";
            return;
        }
        cout << "\n  ===== PRODUCT CATALOG (" << count() << " items) =====\n";
        for (auto *p : products)
            p->displayDetails(); // Polymorphism
    }

    void printByType(const string &type) const
    {
        auto res = findByType(type);
        if (res.empty())
        {
            cout << "  No products of type '" << type << "'.\n";
            return;
        }
        for (auto *p : res)
            p->displayDetails();
    }

    void printSearchResults(const string &keyword) const
    {
        auto res = findByName(keyword);
        if (res.empty())
        {
            cout << "  No products matching '" << keyword << "'.\n";
            return;
        }
        for (auto *p : res)
            p->displayDetails();
    }

    void printOutOfStock() const
    {
        cout << "\n  ===== OUT-OF-STOCK / UNAVAILABLE =====\n";
        bool any = false;
        for (auto *p : products)
            if (!p->isAvailable())
            {
                cout << "  [" << p->getProductId() << "] "
                     << p->getName() << " – " << p->getStatusString() << "\n";
                any = true;
            }
        if (!any)
            cout << "  All products are in stock!\n";
    }

    void printInventorySummary() const
    {
        cout << "\n  ===== INVENTORY SUMMARY =====\n";
        cout << "  Total Products   : " << count() << "\n";
        cout << "  Available        : " << countAvailable() << "\n";
        cout << "  Out of Stock     : " << countOutOfStock() << "\n";
        cout << "  Inventory Value  : $" << fixed << setprecision(2)
             << totalInventoryValue() << "\n";

        // Count by type
        map<string, int> byType;
        for (auto *p : products)
            byType[p->getType()]++;
        cout << "  By Type:\n";
        for (auto &[type, cnt] : byType)
            cout << "    " << type << ": " << cnt << "\n";
    }

    // Sorted view by price (ascending)
    void printSortedByPrice() const
    {
        vector<product *> sorted = products;
        sort(sorted.begin(), sorted.end(),
             [](product *a, product *b)
             { return *a < *b; });
        cout << "\n  ===== PRODUCTS SORTED BY PRICE =====\n";
        for (auto *p : sorted)
            cout << "  $" << fixed << setprecision(2) << p->getCurrentPrice()
                 << "  [" << p->getProductId() << "] " << p->getName() << "\n";
    }

    // -------------------------------------------------------------------------
    //  File I/O
    // -------------------------------------------------------------------------
    void saveToFile() const
    {
        ofstream f(FILE_PATH);
        if (!f)
        {
            cerr << "  [Inventory] Cannot open " << FILE_PATH << "\n";
            return;
        }
        for (auto *p : products)
        {
            f << p->getProductId() << "|"
              << p->getType() << "|"
              << p->getName() << "|"
              << p->getBrand() << "|"
              << p->getSku() << "|"
              << fixed << setprecision(2) << p->getCurrentPrice() << "|"
              << p->getStockQuantity() << "|"
              << p->getStatusString() << "\n";
        }
        cout << "  [Inventory] Products saved to " << FILE_PATH << "\n";
    }

    void loadFromFile()
    {
        ifstream f(FILE_PATH);
        if (!f)
        {
            cout << "  [Inventory] No products file found.\n";
            return;
        }
        string line;
        int loaded = 0;
        while (getline(f, line))
        {
            istringstream ss(line);
            string id, type, name, brand, sku, priceStr, stockStr, statusStr;
            getline(ss, id, '|');
            getline(ss, type, '|');
            getline(ss, name, '|');
            getline(ss, brand, '|');
            getline(ss, sku, '|');
            getline(ss, priceStr, '|');
            getline(ss, stockStr, '|');
            getline(ss, statusStr, '|');

            if (indexOf(id) != -1)
                continue; // skip duplicates

            try
            {
                double price = stod(priceStr);
                int stock = stoi(stockStr);
                product *p = nullptr;
                if (type == "smartphone")
                    p = new smartPhone(id, sku, name, brand, "loaded",
                                       price, stock, 8, 128, 4000, 6.5, 48, "Android");
                else if (type == "laptop")
                    p = new laptop(id, sku, name, brand, "loaded",
                                   price, stock, 8, 256, true, 15.6,
                                   "Windows", "Unknown CPU", "", 50,
                                   LaptopFormFactor::ULTRABOOK);
                else
                    p = new accessory(id, sku, name, brand, "loaded",
                                      price, stock, AccessoryCategory::CASE_COVER);
                if (p)
                {
                    products.push_back(p);
                    ++loaded;
                }
            }
            catch (...)
            { /* skip malformed lines */
            }
        }
        cout << "  [Inventory] Loaded " << loaded << " product(s) from file.\n";
    }

    // -------------------------------------------------------------------------
    //  Operator overloading
    // -------------------------------------------------------------------------

    // += : add a product to inventory
    Inventory &operator+=(product *p)
    {
        addProduct(p);
        return *this;
    }

    // -= : remove a product by ID string
    Inventory &operator-=(const string &id)
    {
        if (!removeProduct(id))
            cout << "  [Inventory] Product '" << id << "' not found for removal.\n";
        return *this;
    }

    // [] : lookup by ID (returns nullptr if not found)
    product *operator[](const string &id) const { return findById(id); }

    // << : print summary
    friend ostream &operator<<(ostream &out, const Inventory &inv)
    {
        out << "Inventory: " << inv.count() << " product(s) | "
            << inv.countAvailable() << " available | "
            << "Value: $" << fixed << setprecision(2) << inv.totalInventoryValue();
        return out;
    }
};

// =============================================================================
//  SINGLETON PATTERN – StoreManager
// =============================================================================
class StoreManager
{
private:
    // --- Singleton boilerplate ---
    static StoreManager *instance;
    StoreManager() : taxRate(14.0) {}
    StoreManager(const StoreManager &) = delete;
    StoreManager &operator=(const StoreManager &) = delete;

    // --- Data ---
    Inventory inventory; // <-- dedicated Inventory object
    vector<Customer *> customers;
    vector<Admin *> admins;
    vector<Order> allOrders;
    double taxRate;

    // --- File paths ---
    const string CUSTOMERS_FILE = "customers.txt";
    const string ORDERS_FILE = "orders.txt";

    // --- Helper: find product by ID (delegates to Inventory) ---
    product *findProduct(const string &id) const
    {
        return inventory.findById(id);
    }

    Customer *findCustomer(const string &id) const
    {
        for (auto *c : customers)
            if (c->getUserId() == id)
                return c;
        return nullptr;
    }

    User *findUser(const string &email, const string &password) const
    {
        for (auto *a : admins)
            if (a->getEmail() == email && a->checkPassword(password))
                return a;
        for (auto *c : customers)
            if (c->getEmail() == email && c->checkPassword(password))
                return c;
        return nullptr;
    }

    // --- Input helpers ---
    static int readInt(const string &prompt, int lo, int hi)
    {
        int v;
        while (true)
        {
            cout << prompt;
            if (cin >> v && v >= lo && v <= hi)
            {
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                return v;
            }
            cout << "  Invalid input. Enter a number between " << lo << " and " << hi << ".\n";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }
    }
    static double readDouble(const string &prompt)
    {
        double v;
        while (true)
        {
            cout << prompt;
            if (cin >> v && v >= 0)
            {
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                return v;
            }
            cout << "  Invalid input.\n";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }
    }
    static string readLine(const string &prompt)
    {
        string s;
        cout << prompt;
        getline(cin, s);
        return s;
    }

public:
    // --- Singleton accessor ---
    static StoreManager *getInstance()
    {
        if (!instance)
            instance = new StoreManager();
        return instance;
    }

    ~StoreManager()
    {
        // inventory destructor deletes its own products
        for (auto *c : customers)
            delete c;
        for (auto *a : admins)
            delete a;
    }

    // =========================================================================
    //  SEED DATA
    // =========================================================================
    void seedData()
    {
        // Products
        auto *s1 = ProductFactory::createSmartPhone("SP001", "Galaxy S25 Ultra", "Samsung", 1299.99, 15, 12, 256, "Android 15");
        s1->setHas5G(true);
        s1->setHasNFC(true);
        s1->setChipset("Snapdragon 8 Gen 4");
        inventory += s1;

        auto *s2 = ProductFactory::createSmartPhone("SP002", "iPhone 16 Pro", "Apple", 1199.99, 10, 8, 128, "iOS 18");
        s2->setHas5G(true);
        s2->setHasNFC(true);
        s2->setChipset("A18 Pro");
        inventory += s2;

        auto *s3 = ProductFactory::createSmartPhone("SP003", "Pixel 9", "Google", 799.99, 20, 8, 128, "Android 15");
        inventory += s3;

        auto *l1 = ProductFactory::createLaptop("LP001", "MacBook Pro 14", "Apple", 1999.99, 8, 16, 512, "Apple M4");
        inventory += l1;

        auto *l2 = ProductFactory::createLaptop("LP002", "XPS 15", "Dell", 1499.99, 5, 32, 1024, "Intel Core Ultra 9");
        inventory += l2;

        auto *l3 = ProductFactory::createLaptop("LP003", "ROG Strix G16", "ASUS", 1299.99, 12, 16, 512, "Intel Core i9");
        inventory += l3;

        auto *a1 = ProductFactory::createAccessory("AC001", "AirPods Pro 2", "Apple", 249.99, 30, AccessoryCategory::HEADPHONES);
        a1->setWireless(true);
        a1->addCompatibility("iPhone");
        a1->addCompatibility("iPad");
        inventory += a1;

        auto *a2 = ProductFactory::createAccessory("AC002", "USB-C 65W Charger", "Anker", 39.99, 50, AccessoryCategory::CHARGER);
        a2->setConnectivity("USB-C");
        inventory += a2;

        auto *a3 = ProductFactory::createAccessory("AC003", "MX Keys S Keyboard", "Logitech", 119.99, 25, AccessoryCategory::KEYBOARD);
        a3->setWireless(true);
        a3->addCompatibility("Windows");
        a3->addCompatibility("macOS");
        inventory += a3;

        // Default admin
        admins.push_back(new Admin("A001", "Admin One", "admin@store.com", "admin123", "Inventory"));

        // Sample customers
        customers.push_back(new Customer("C001", "Alice Johnson", "alice@mail.com", "pass1", "Cairo, EG"));
        customers.push_back(new Customer("C002", "Bob Smith", "bob@mail.com", "pass2", "Giza, EG"));

        cout << "  [SYSTEM] Seed data loaded.\n";
    }

    // =========================================================================
    //  FILE HANDLING
    // =========================================================================
    void saveProducts() const { inventory.saveToFile(); }

    void loadProductsFromFile() { inventory.loadFromFile(); }

    void saveCustomers() const
    {
        ofstream f(CUSTOMERS_FILE);
        if (!f)
        {
            cerr << "  Cannot open " << CUSTOMERS_FILE << "\n";
            return;
        }
        for (auto *c : customers)
        {
            f << c->getUserId() << "|"
              << c->getName() << "|"
              << c->getEmail() << "|"
              << c->getAddress() << "|"
              << fixed << setprecision(2) << c->getTotalSpent() << "\n";
        }
        cout << "  Customers saved to " << CUSTOMERS_FILE << "\n";
    }

    void saveOrders() const
    {
        ofstream f(ORDERS_FILE);
        if (!f)
        {
            cerr << "  Cannot open " << ORDERS_FILE << "\n";
            return;
        }
        for (auto &o : allOrders)
            f << o.toCSV() << "\n";
        // Also save each customer's orders
        for (auto *c : customers)
            for (auto &o : c->getOrderHistory())
                f << o.toCSV() << "\n";
        cout << "  Orders saved to " << ORDERS_FILE << "\n";
    }

    void saveAll() const
    {
        saveProducts();
        saveCustomers();
        saveOrders();
    }

    // =========================================================================
    //  REPORTING  (delegate product-side to Inventory)
    // =========================================================================
    void listAllProducts() const { inventory.printAll(); }

    void listByType(const string &type) const { inventory.printByType(type); }

    void searchByName(const string &keyword) const { inventory.printSearchResults(keyword); }

    void searchById(const string &id) const
    {
        product *p = inventory.findById(id);
        if (p)
            p->displayDetails();
        else
            cout << "  Product ID '" << id << "' not found.\n";
    }

    void outOfStockReport() const { inventory.printOutOfStock(); }
    void inventorySummary() const { inventory.printInventorySummary(); }
    void inventorySortedPrice() const { inventory.printSortedByPrice(); }

    void salesReport() const
    {
        cout << "\n  ===== SALES REPORT =====\n";
        double totalRevenue = 0;
        int totalOrders = 0;
        for (auto *c : customers)
        {
            totalRevenue += c->getTotalSpent();
            totalOrders += (int)c->getOrderHistory().size();
        }
        cout << "  Total Customers  : " << customers.size() << "\n";
        cout << "  Total Orders     : " << totalOrders << "\n";
        cout << "  Total Revenue    : $" << fixed << setprecision(2) << totalRevenue << "\n";
        cout << "\n  Top Customers:\n";
        // Sort customers by totalSpent (copy pointers)
        vector<Customer *> sorted = customers;
        sort(sorted.begin(), sorted.end(),
             [](Customer *a, Customer *b)
             { return a->getTotalSpent() > b->getTotalSpent(); });
        int rank = 1;
        for (auto *c : sorted)
            cout << "  #" << rank++ << " " << c->getName()
                 << "  –  $" << fixed << setprecision(2) << c->getTotalSpent() << "\n";
    }

    void listAllCustomers() const
    {
        cout << "\n  ===== REGISTERED CUSTOMERS (" << customers.size() << ") =====\n";
        for (auto *c : customers)
            cout << "  " << *c << "  | Address: " << c->getAddress()
                 << "  | Spent: $" << fixed << setprecision(2) << c->getTotalSpent() << "\n";
    }

    void listAllOrders() const
    {
        cout << "\n  ===== ALL ORDERS =====\n";
        bool any = false;
        for (auto *c : customers)
            for (auto &o : c->getOrderHistory())
            {
                cout << o;
                any = true;
            }
        if (!any)
            cout << "  No orders placed yet.\n";
    }

    // =========================================================================
    //  ADMIN OPERATIONS
    // =========================================================================
    void adminAddProduct()
    {
        cout << "\n  Add Product Type:\n";
        cout << "  [1] Smartphone  [2] Laptop  [3] Accessory\n";
        int choice = readInt("  Choice: ", 1, 3);

        string id = readLine("  Product ID  : ");
        if (findProduct(id))
        {
            cout << "  ID already exists.\n";
            return;
        }
        string name = readLine("  Name        : ");
        string brand = readLine("  Brand       : ");
        double price = readDouble("  Price ($)   : ");
        int stock = readInt("  Stock qty   : ", 0, 99999);

        product *p = nullptr;

        if (choice == 1)
        {
            int ram = readInt("  RAM (GB)    : ", 1, 64);
            int storage = readInt("  Storage (GB): ", 8, 2048);
            string os = readLine("  OS          : ");
            p = ProductFactory::createSmartPhone(id, name, brand, price, stock, ram, storage, os);
        }
        else if (choice == 2)
        {
            int ram = readInt("  RAM (GB)    : ", 1, 128);
            int storage = readInt("  Storage (GB): ", 64, 4096);
            string cpu = readLine("  CPU         : ");
            p = ProductFactory::createLaptop(id, name, brand, price, stock, ram, storage, cpu);
        }
        else
        {
            cout << "  Category [1]Case [2]Charger [3]Cable [4]Headphones [5]KeyBoard [6]Mouse: ";
            int cat = readInt("", 1, 6);
            AccessoryCategory cats[] = {
                AccessoryCategory::CASE_COVER, AccessoryCategory::CHARGER,
                AccessoryCategory::CABLE, AccessoryCategory::HEADPHONES,
                AccessoryCategory::KEYBOARD, AccessoryCategory::MOUSE};
            p = ProductFactory::createAccessory(id, name, brand, price, stock, cats[cat - 1]);
        }

        if (p)
        {
            inventory += p;
            cout << "  Product added successfully.\n";
        }
    }

    void adminRemoveProduct()
    {
        string id = readLine("  Enter Product ID to remove: ");
        inventory -= id;
        cout << "  Product removed.\n";
    }

    void adminRestock()
    {
        string id = readLine("  Product ID  : ");
        product *p = findProduct(id);
        if (!p)
        {
            cout << "  Not found.\n";
            return;
        }
        int qty = readInt("  Qty to add  : ", 1, 99999);
        try
        {
            p->restock(qty);
            cout << "  Restocked. New stock: " << p->getStockQuantity() << "\n";
        }
        catch (exception &e)
        {
            cout << "  Error: " << e.what() << "\n";
        }
    }

    void adminSetDiscount()
    {
        string id = readLine("  Product ID  : ");
        product *p = findProduct(id);
        if (!p)
        {
            cout << "  Not found.\n";
            return;
        }
        cout << "  [1] Percentage  [2] Fixed Amount\n";
        int t = readInt("  Type: ", 1, 2);
        double amt = readDouble("  Amount: ");
        try
        {
            p->setDiscountSale(amt, t == 1 ? DiscountType::PERCENTAGE : DiscountType::FIXED_AMOUNT);
            cout << "  Discount applied. New price: $"
                 << fixed << setprecision(2) << p->getCurrentPrice() << "\n";
        }
        catch (exception &e)
        {
            cout << "  Error: " << e.what() << "\n";
        }
    }

    void adminDiscontinue()
    {
        string id = readLine("  Product ID to discontinue: ");
        product *p = findProduct(id);
        if (!p)
        {
            cout << "  Not found.\n";
            return;
        }
        p->discontinue();
        cout << "  Product marked as discontinued.\n";
    }

    // =========================================================================
    //  CUSTOMER OPERATIONS
    // =========================================================================
    void customerBrowse(Customer *c)
    {
        cout << "\n  Filter: [1] All  [2] Smartphones  [3] Laptops  [4] Accessories\n";
        int ch = readInt("  Choice: ", 1, 4);
        if (ch == 1)
            listAllProducts();
        else if (ch == 2)
            listByType("smartphone");
        else if (ch == 3)
            listByType("laptop");
        else
            listByType("accessory");
    }

    void customerSearch(Customer *c)
    {
        cout << "  [1] By Name  [2] By ID\n";
        int ch = readInt("  Choice: ", 1, 2);
        if (ch == 1)
        {
            string kw = readLine("  Keyword: ");
            searchByName(kw);
        }
        else
        {
            string id = readLine("  ID: ");
            searchById(id);
        }
    }

    void customerAddToCart(Customer *c)
    {
        string id = readLine("  Product ID to add: ");
        product *p = findProduct(id);
        if (!p)
        {
            cout << "  Product not found.\n";
            return;
        }
        p->displayDetails();
        int qty = readInt("  Quantity: ", 1, 9999);
        try
        {
            c->addToCart(p, qty);
            cout << "  Added to cart.\n";
        }
        catch (exception &e)
        {
            cout << "  Error: " << e.what() << "\n";
        }
    }

    void customerRemoveFromCart(Customer *c)
    {
        string id = readLine("  Product ID to remove: ");
        try
        {
            c->removeFromCart(id);
            cout << "  Removed from cart.\n";
        }
        catch (exception &e)
        {
            cout << "  Error: " << e.what() << "\n";
        }
    }

    void customerCheckout(Customer *c)
    {
        c->viewCart();
        if (c->cartIsEmpty())
            return;
        cout << "\n  Confirm checkout? (y/n): ";
        char ans;
        cin >> ans;
        cin.ignore();
        if (ans != 'y' && ans != 'Y')
        {
            cout << "  Checkout cancelled.\n";
            return;
        }
        try
        {
            Order o = c->checkout();
            allOrders.push_back(o);
            cout << "\n  Order placed!\n"
                 << o;
        }
        catch (exception &e)
        {
            cout << "  Error: " << e.what() << "\n";
        }
    }

    void customerProfile(Customer *c)
    {
        cout << "\n  ===== PROFILE =====\n";
        cout << "  Name   : " << c->getName() << "\n";
        cout << "  Email  : " << c->getEmail() << "\n";
        cout << "  Address: " << c->getAddress() << "\n";
        cout << "  Spent  : $" << fixed << setprecision(2) << c->getTotalSpent() << "\n";
    }

    // =========================================================================
    //  REGISTRATION & LOGIN
    // =========================================================================
    void registerCustomer()
    {
        cout << "\n  === Register ===\n";
        string id = "C" + to_string(100 + (int)customers.size() + 1);
        string name = readLine("  Full Name   : ");
        string email = readLine("  Email       : ");
        // check duplicate email
        for (auto *c : customers)
            if (c->getEmail() == email)
            {
                cout << "  Email already registered.\n";
                return;
            }
        string pass = readLine("  Password    : ");
        string addr = readLine("  Address     : ");
        customers.push_back(new Customer(id, name, email, pass, addr));
        cout << "  Registered! Your ID: " << id << "\n";
    }

    User *login()
    {
        cout << "\n  === Login ===\n";
        string email = readLine("  Email   : ");
        string pass = readLine("  Password: ");
        User *u = findUser(email, pass);
        if (u)
            cout << "  Welcome, " << u->getName() << " [" << u->getRole() << "]\n";
        else
            cout << "  Invalid credentials.\n";
        return u;
    }

    // =========================================================================
    //  MAIN MENU LOOPS
    // =========================================================================
    void runAdminMenu(Admin *a)
    {
        while (true)
        {
            a->showMenu();
            int ch = readInt("  Choice: ", 0, 14);
            switch (ch)
            {
            case 0:
                cout << "  Logged out.\n";
                return;
            case 1:
                adminAddProduct();
                break;
            case 2:
                adminRemoveProduct();
                break;
            case 3:
                adminRestock();
                break;
            case 4:
                adminSetDiscount();
                break;
            case 5:
                adminDiscontinue();
                break;
            case 6:
                listAllProducts();
                break;
            case 7:
                listAllOrders();
                break;
            case 8:
                listAllCustomers();
                break;
            case 9:
                salesReport();
                break;
            case 10:
                outOfStockReport();
                break;
            case 11:
                inventorySummary();
                break;
            case 12:
                inventorySortedPrice();
                break;
            case 13:
                saveAll();
                break;
            case 14:
                loadProductsFromFile();
                break;
            default:
                break;
            }
        }
    }

    void runCustomerMenu(Customer *c)
    {
        while (true)
        {
            c->showMenu();
            int ch = readInt("  Choice: ", 0, 8);
            switch (ch)
            {
            case 0:
                cout << "  Logged out.\n";
                return;
            case 1:
                customerBrowse(c);
                break;
            case 2:
                customerSearch(c);
                break;
            case 3:
                c->viewCart();
                break;
            case 4:
                customerAddToCart(c);
                break;
            case 5:
                customerRemoveFromCart(c);
                break;
            case 6:
                customerCheckout(c);
                break;
            case 7:
                c->printOrderHistory();
                break;
            case 8:
                customerProfile(c);
                break;
            default:
                break;
            }
        }
    }

    void run()
    {
        cout << R"(
  ╔════════════════════════════════════════════╗
  ║    Tech Store – Inventory & Sales System   ║
  ║         OOP CSE122 – Spring 2026           ║
  ╚════════════════════════════════════════════╝
)";

        while (true)
        {
            cout << "\n  [1] Login\n  [2] Register\n  [0] Exit\n";
            int ch = readInt("  Choice: ", 0, 2);
            if (ch == 0)
            {
                cout << "  Goodbye!\n";
                break;
            }
            if (ch == 2)
            {
                registerCustomer();
                continue;
            }

            User *u = login();
            if (!u)
                continue;

            if (u->getRole() == "Admin")
                runAdminMenu(dynamic_cast<Admin *>(u));
            else
                runCustomerMenu(dynamic_cast<Customer *>(u));
        }
    }
};

// Static member definition
StoreManager *StoreManager::instance = nullptr;

// =============================================================================
//  MAIN
// =============================================================================
int main()
{
    StoreManager *store = StoreManager::getInstance();
    store->seedData();
    store->run();
    return 0;
}