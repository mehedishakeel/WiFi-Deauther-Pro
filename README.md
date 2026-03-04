# WiFi-Deauther-BW16
WiFi-Deauther-BW16 (RTL8720DN) Works Both For 2.4Ghz &amp; 5Ghz WiFi Networks Deauth Attack!

![WiFi Deauther BW16 UI](/images/UI.png)


---

## 📦 Installation Guide

### 1️⃣ Install Arduino IDE

Download and install the official Arduino IDE:

https://www.arduino.cc/en/software

---

### 2️⃣ Add BW16 (RTL8720DN) Board Support

1. Open **Arduino IDE**
2. Go to **File → Preferences**
3. In **Additional Boards Manager URLs**, add:

```
https://raw.githubusercontent.com/Ameba-AIoT/ameba-arduino-d/master/Arduino_package/package_realtek_amebad_index.json
```

4. Click **OK**
5. Go to **Tools → Board → Boards Manager**
6. Search for **Ameba**
7. Install the latest version

---

### 4️⃣ Upload the Code

1. Clone this repository or download the zip file.
2. Open the `WiFi-Deauther-BW16.ino` file in **Arduino IDE**.
3. Select the correct board:

```
Tools → Board → BW16 (RTL8720DN)
```

4. Select the correct port:

```
Tools → Port → [Your Device Port]
```

5. Click **Upload**

---

## 🚀 Usage Guide

### 1️⃣ Connect to the Device

1. Power on the **BW16 module**
2. Connect to the Wi-Fi network:

```
SSID: WiFi-Deauther-BW16
Password: 12345678
```

3. Open your browser and visit:

```
http://192.168.1.1
```

---

### 2️⃣ Web Interface

#### 📡 Deauthentication Section

- Scan for available Wi-Fi networks
- Select the target network(s)
- Start the deauthentication process

> ⚠️ Only test on networks you are authorized to audit.

---

## 🙏 Credits & Thanks

Special thanks and credit to the original project:

https://github.com/tesa-klebeband/RTL8720dn-Deauther

This project was inspired by and built upon the work of the above repository.

---

## 📌 Notes

- Ensure you are using the correct board configuration.
- Use a stable USB cable for reliable uploads.
- Testing should be performed in an isolated lab environment.

---

## ⚠️ DISCLAIMER

This tool has been developed **strictly for educational and authorized security testing purposes only**.

Any misuse, unauthorized access, disruption of networks, or illegal activity conducted using this tool is **strictly prohibited**.

The developer assumes **no responsibility** for any damage, legal consequences, or misuse arising from the use of this software.

> Use this tool only on networks you own or have explicit permission to test.

