#include "Product.cpp"
#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
using namespace std;
class ProductFactory
{
public:

    static smartPhone *createSmartPhone(//constructorParameters
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
                                        //otherAttributes
                                        bool has5G=false, bool hasNFC = false,
                                        int refreshRate = 60,
                                        const string &chip = "Unkown")
    {

        string sku = "SP-" + id; 
        
        smartPhone* sp = new smartPhone(id, sku, name, brand, model, price, stock,ram,storage,battery
            ,display,camera,os);
        
        sp->setHas5G(has5G);
        sp->setHasNFC(hasNFC);
        sp->setRefreshRate(refreshRate);
        sp->setChipset(chip);
        return sp;
    }


    static laptop *createLaptop(//constructorParameters
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
                                LaptopFormFactor from,
                                double weight = 0.0,
                                //otherAttributes
                                bool touchScreen = false,int usbPorts= 0,
                                bool hasThunderbolt = false)
    {
        string sku = "LP-" + id;
        
        laptop* lp = new laptop(id, sku, name, brand, model, price, stock, ram, storage, ssd,
            display, os, cpu, gpu, batteryWh, from, weight);
        lp->setTouchscreen(touchScreen);
        lp->setUsbPorts(usbPorts);
        lp->setThunderbolt(hasThunderbolt);
        return lp;
    }


    static accessory *createAccessory(//constructorParameters
                                       const string &id,
                                       const string &name,
                                       const string &brand,
                                       const string &model,
                                       double price,
                                       int stock,
                                       AccessoryCategory cat,
                                       const string &color = "", const string &material = "",
                                       //otherAttributes
                                       const string &connectivity = "", bool isWireless = false)
    {
        string sku = "AC-" + id;
        accessory* ac = new accessory(id,sku,name,brand,model,price,stock,cat,color,material);
        ac->setConnectivity(connectivity);
        ac->setWireless(isWireless);
        return ac;

    }
};