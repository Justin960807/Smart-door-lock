# Smart Door Lock (智慧門鎖)

這是一個基於物聯網與微控制器的智慧門鎖專案。本專案透過多種驗證方式 (如 RFID、密碼輸入或指紋辨識) 控制伺服馬達/電磁鎖，實現門鎖的自動化與安全性升級。

## 🌟 功能特點 (Features)
* **多重解鎖方式**：支援密碼解鎖、RFID 感應解鎖 (可依需求新增藍牙或 Wi-Fi 遠端解鎖)。
* **安全機制**：密碼錯誤多次自動鎖定並發出警報聲。
* **狀態顯示**：透過 LCD 螢幕或 OLED 顯示目前門鎖狀態 (已解鎖、密碼錯誤等)。
* **自動上鎖**：開門後經過設定秒數，系統將自動恢復上鎖狀態。

## 🛠️ 硬體需求 (Hardware Requirements)
* **微控制器**：Arduino UNO / ESP8266 / ESP32 (請依實際情況修改)
* **執行機構**：Servo Motor (伺服馬達) 或 Solenoid Lock (電磁鎖)
* **輸入模組**：
  * 4x4 薄膜鍵盤 (Membrane Keypad)
  * MFRC522 RFID 模組
* **顯示與提示**：
  * LCD 1602 顯示器 (含 I2C 模組)
  * 蜂鳴器 (Buzzer)
  * LED 指示燈
* **其他**：跳線、麵包板、電源供應模組

## 💻 軟體與函式庫 (Software & Libraries)
請確保在 Arduino IDE 中安裝以下函式庫：
* `Keypad.h` (鍵盤控制)
* `Servo.h` (馬達控制)
* `SPI.h` 與 `MFRC522.h` (RFID 控制)
* `LiquidCrystal_I2C.h` (I2C LCD 控制)

## ⚙️ 安裝與使用方式 (Installation & Usage)
1. **硬體接線**：依照程式碼中的腳位定義，將各感測器與微控制器連接。
2. **燒錄程式**：
   * 下載本專案原始碼 `git clone https://github.com/Justin960807/Smart-door-lock.git`
   * 使用 Arduino IDE 開啟 `.ino` 檔案。
   * 選擇對應的開發板與 Port。
   * 點擊「上傳 (Upload)」。
3. **初始設定**：
   * 系統啟動後，使用預設密碼進行測試 (預設密碼設定於程式碼變數中，建議後續更改)。
   * 將預計使用的 RFID 磁扣靠上感應區以登錄卡號。

## 📂 腳位配置參考 (Pin Configuration)
| 模組 | 腳位名稱 | 微控制器腳位 |
| :--- | :--- | :--- |
| MFRC522 | SDA (SS) | Pin 10 |
| MFRC522 | SCK | Pin 13 |
| Servo | PWM | Pin 9 |
| I2C LCD | SDA | A4 |
| I2C LCD | SCL | A5 |
