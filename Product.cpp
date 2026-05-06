#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include <stdexcept>
#include <algorithm>

using namespace std;

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

    void updateStatus()
    {
        if (status == ProductStatus::DISCONTINUED || status == ProductStatus::COMING_SOON)
            return;
        status = (stockQuantity > 0)
                     ? ProductStatus::AVAILABLE
                     : ProductStatus::OUT_OF_STOCK;
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
    // FIX: Default constructor now explicitly initialises all members
    product()
        : mainPrice(0.0), currentPrice(0.0), discount(0.0),
          onSale(false), stockQuantity(0),
          status(ProductStatus::OUT_OF_STOCK) {}

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

    virtual void displayDetails() const = 0;
    virtual string getType() const = 0;

    virtual void print(ostream &out) const
    {
        displayBase(out);
    }

    virtual ~product() {}

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

    // FIX: Added public statusToString wrapper so derived classes / callers can use it
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
                throw invalid_argument("Fixed discount must be between 0 and the product price.");
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

    // FIX: Validate taxes — negative tax makes no sense here
    double getFinalPrice(double taxes = 0.0) const
    {
        if (taxes < 0)
            throw invalid_argument("Tax rate cannot be negative.");
        return currentPrice + currentPrice * (taxes / 100.0);
    }

    // FIX: sell() now refuses to sell discontinued or coming-soon products
    bool sell(int quantity)
    {
        if (quantity <= 0)
            throw invalid_argument("Quantity to sell must be positive.");
        if (status == ProductStatus::DISCONTINUED)
            throw logic_error("Cannot sell a discontinued product.");
        if (status == ProductStatus::COMING_SOON)
            throw logic_error("Cannot sell a product that is not yet available.");
        if (stockQuantity < quantity)
            return false;
        stockQuantity -= quantity;
        updateStatus();
        return true;
    }

    // FIX: restock() now refuses to restock discontinued products
    void restock(int quantity)
    {
        if (quantity <= 0)
            throw invalid_argument("Quantity to restock must be positive.");
        if (status == ProductStatus::DISCONTINUED)
            throw logic_error("Cannot restock a discontinued product.");
        stockQuantity += quantity;
        updateStatus();
    }

    bool isAvailable() const
    {
        return status == ProductStatus::AVAILABLE && stockQuantity > 0;
    }

    // FIX: discontinue() guards against marking a coming-soon product as discontinued
    //      without explicit intent, and zeroes stock clearly.
    void discontinue()
    {
        status = ProductStatus::DISCONTINUED;
        stockQuantity = 0;
    }

    bool operator==(const product &other) const { return productId == other.productId; }
    bool operator<(const product &other) const { return currentPrice < other.currentPrice; }

    friend ostream &operator<<(ostream &out, const product &p)
    {
        p.print(out);
        return out;
    }
};

// =============================================================================
// smartPhone
// =============================================================================

class smartPhone : public product
{
private:
    int ram;
    int storage;
    int batteryMah;
    double inches;
    int cameraMP;
    string phoneOS;
    bool has5G;
    bool hasNFC;
    int refreshRateHz;
    string chipset;

public:
    // FIX: parameter renamed from 'os' to 'operatingSystem' to avoid shadowing member
    smartPhone(const string &id, const string &sku,
               const string &name, const string &brand,
               const string &model, double price, int stock,
               int ram, int storage, int battery,
               double display, int camera, const string &operatingSystem)
        : product(id, sku, name, brand, model, price, stock),
          ram(ram), storage(storage), batteryMah(battery),
          inches(display), cameraMP(camera), phoneOS(operatingSystem), has5G(false), hasNFC(false), refreshRateHz(60) {}

    // --- Setters for optional specs ---
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
    double estimatedBatteryDays(int mAhPerDay = 1000) const
    {
        if (mAhPerDay <= 0)
            throw invalid_argument("Usage must be positive.");
        return static_cast<double>(batteryMah) / mAhPerDay;
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
// laptop
// =============================================================================

enum class LaptopFormFactor
{
    ULTRABOOK,
    GAMING,
    WORKSTATION,
    CONVERTIBLE,
    CHROMEBOOK
};

class laptop : public product
{
private:
    int ramGB;
    int storageGB;
    bool isSSD;
    double displayInches;
    string operatingSystem;
    string cpu;
    string gpu;
    int batteryWh;
    LaptopFormFactor formFactor;
    bool hasTouchscreen;
    int usbPorts;
    bool hasThunderbolt;
    double weightKg;

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
           double display, const string &operatingSystem,
           const string &cpu, const string &gpu,
           int batteryWh, LaptopFormFactor form,
           double weightKg = 0.0)
        : product(id, sku, name, brand, model, price, stock),
          ramGB(ram), storageGB(storage), isSSD(ssd),
          displayInches(display), operatingSystem(operatingSystem),
          cpu(cpu), gpu(gpu), batteryWh(batteryWh), formFactor(form),
          hasTouchscreen(false), usbPorts(0), hasThunderbolt(false),
          weightKg(weightKg) {}
    void setTouchscreen(bool v) { hasTouchscreen = v; }
    void setUsbPorts(int n)
    {
        if (n < 0)
            throw invalid_argument("USB port count cannot be negative.");
        usbPorts = n;
    }
    void setThunderbolt(bool v) { hasThunderbolt = v; }
    void setFormFactor(LaptopFormFactor f) { formFactor = f; }
    int getRam() const { return ramGB; }
    int getStorage() const { return storageGB; }
    bool getSSD() const { return isSSD; }
    double getDisplay() const { return displayInches; }
    string getOS() const { return operatingSystem; }
    string getCPU() const { return cpu; }
    string getGPU() const { return gpu; }
    int getBatteryWh() const { return batteryWh; }
    LaptopFormFactor getFormFactor() const { return formFactor; }
    bool getTouchscreen() const { return hasTouchscreen; }
    int getUsbPorts() const { return usbPorts; }
    bool getThunderbolt() const { return hasThunderbolt; }
    double getWeightKg() const { return weightKg; }

    bool hasDiscreteGPU() const { return !gpu.empty(); }
    bool isWorkhorseReady() const { return ramGB >= 16 && isSSD && storageGB >= 512; }
    bool isUltrathin() const { return weightKg > 0 && weightKg < 1.5; }
    double estimatedBatteryHours(int avgWatts = 10) const
    {
        if (avgWatts <= 0)
            throw invalid_argument("Average watts must be positive.");
        return static_cast<double>(batteryWh) / avgWatts;
    }
    string getType() const override { return "laptop"; }
    void displayDetails() const override { cout << *this; }

    void print(ostream &out) const override
    {
        displayBase(out);
        out << " --- Laptop Details ---\n";
        out << " Form Factor: " << formFactorToString() << "\n";
        out << " CPU        : " << cpu << "\n";
        out << " GPU        : " << (hasDiscreteGPU() ? gpu : "Integrated") << "\n";
        out << " RAM        : " << ramGB << " GB\n";
        out << " Storage    : " << storageGB << " GB "
            << (isSSD ? "(SSD)" : "(HDD)") << "\n";
        out << " Display    : " << displayInches << "\"\n";
        out << " Battery    : " << batteryWh << " Wh\n";
        out << " OS         : " << operatingSystem << "\n";
        out << " Workhorse  : " << (isWorkhorseReady() ? "Yes" : "No") << "\n";
        out << "========================================\n";
    }
};

// =============================================================================
// accessory
// =============================================================================

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

class accessory : public product
{
private:
    AccessoryCategory category;
    vector<string> compatibleWith;
    string color;
    string material;
    string connectivity;
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
        compatibleWith.erase(
            remove(compatibleWith.begin(), compatibleWith.end(), device),
            compatibleWith.end());
    }

    void clearCompatibility() { compatibleWith.clear(); }

    bool isCompatibleWith(const string &device) const
    {
        return find(compatibleWith.begin(), compatibleWith.end(), device) != compatibleWith.end();
    }

    const vector<string> &getCompatibleDevices() const { return compatibleWith; }
    int getCompatibilityCount() const { return static_cast<int>(compatibleWith.size()); }

    // Setters
    void setConnectivity(const string &c) { connectivity = c; }
    void setWireless(bool v) { isWireless = v; }
    void setColor(const string &c) { color = c; }
    void setMaterial(const string &m) { material = m; }

    // Getters
    AccessoryCategory getCategory() const { return category; }
    string getColor() const { return color; }
    string getMaterial() const { return material; }
    string getConnectivity() const { return connectivity; }
    bool getIsWireless() const { return isWireless; }

    void displayDetails() const override { cout << *this; }
    string getType() const override { return "Accessory"; }
    void print(ostream &out) const override
    {
        displayBase(out);
        out << " --- Accessory Details ---\n";
        out << " Category   : " << categoryToString() << "\n";
        if (!color.empty())
            out << " Color      : " << color << "\n";
        if (!material.empty())
            out << " Material   : " << material << "\n";
        if (!connectivity.empty())
            out << " Connectivity: " << connectivity << "\n";
        out << " Wireless   : " << (isWireless ? "Yes" : "No") << "\n";
        if (!compatibleWith.empty())
        {
            out << " Compatible : ";
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