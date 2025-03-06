# [Prakriti Dosha Application](https://aromadiffuserdevice.netlify.app/)

## Overview

The **Prakriti Dosha Application** is a web-based platform designed to help users determine their Ayurvedic dosha type (Vata, Pitta, Kapha) based on a series of questions related to physical and mental characteristics. The application also integrates with a hardware device (an aroma diffuser) that adjusts its functionality based on the user's dosha type. The system is built using HTML, CSS, Tailwind CSS, JavaScript, and Firebase for real-time database management.

![Prakriti Dosha Application](https://img.icons8.com/fluency/96/aroma.png)  
*Icon by Icons8*

---

## Features

1. **User Authentication**:
   - Login and registration system with email and password.
   - Password hashing for secure storage.
   - Device allocation and user association.

2. **Dosha Assessment**:
   - A comprehensive questionnaire covering hair, eye, body, and skin characteristics.
   - Automatic calculation of the user's dosha type based on their answers.
   - Ability to recheck and update the dosha type.

3. **Dashboard**:
   - Real-time display of device status (active/inactive).
   - Manual spray activation.
   - Adjustable spray delay settings.
   - Display of user and device information, including dosha type.

4. **Firebase Integration**:
   - Real-time synchronization of device status, spray delay, and dosha type.
   - Secure user data storage with Firebase Authentication and Realtime Database.

5. **Responsive Design**:
   - Built with Tailwind CSS for a modern and responsive user interface.

6. **Hardware Integration**:
   - Controls an ESP32-based aroma diffuser that adjusts its functionality based on the user's dosha type.
   - Real-time monitoring and control of the device via Firebase.

---

## How It Works

1. **User Registration**:
   - Users register by providing their email and password.
   - During registration, users answer a series of questions to determine their dosha type.
   - The dosha type is calculated and stored in the Firebase database.

2. **User Login**:
   - Users log in using their email, password, and device number.
   - Upon successful login, the dashboard displays the device status, user information, and dosha type.

3. **Dosha Recheck**:
   - Users can recheck their dosha type by answering the questionnaire again.
   - The updated dosha type is saved to the Firebase database and reflected in the dashboard.

4. **Device Control**:
   - Users can manually activate the aroma diffuser or set a spray delay.
   - The device status and spray settings are updated in real-time via Firebase.

5. **Logout and Account Deletion**:
   - Users can log out, which resets the device status and clears the session.
   - Users can also delete their account, which removes their data from the Firebase database.

---

## Firebase Database Schema

The Firebase Realtime Database is structured as follows:

```plaintext
Data
├── Devices
│   ├── Device1
│   │   ├── User: user                  // Associated user ID
│   │   ├── Status: Active/Inactive     // Device operational status
│   │   ├── Dosha: Pittaj/Vataj/Kaphaj/... // Current Dosha setting
│   │   ├── SprayDelay: 5               // Spray interval in minutes
│   │   ├── LastUpdate: "12:00:00"      // Last spray timestamp
│   │   ├── Spray: 0/1                  // Manual spray trigger
│   │
│   ├── Device2                         // Example for another device
│       ├── User: user2
│       ├── Status: Inactive
│       ├── Dosha: VatajKaphaj
│       ├── SprayDelay: 10
│       ├── LastUpdate: "13:30:00"
│       ├── Spray: 0
│
├── Users
    ├── user1
    │   ├── UserEmail: user1@example.com
    │   ├── UserPassword: usersecret
    │   ├── Dosha: Pittaj
    │
    ├── user2
        ├── UserEmail: user2@example.com
        ├── UserPassword: user2secret
        ├── Dosha: VatajKaphaj
```

---

## Hardware Integration

The application is designed to work with an ESP32-based aroma diffuser. The hardware is configured to receive commands from the Firebase database and adjust its operation based on the user's dosha type.

### Pin Configuration

```plaintext
- RELAY_P (Pittaj)        : GPIO 13
- RELAY_K (Kaphaj)        : GPIO 12
- RELAY_V (Vataj)         : GPIO 14
- RELAY_PK (Pittaj-Kaphaj) : GPIO 27
- RELAY_PV (Pittaj-Vataj)  : GPIO 26
- RELAY_VK (Vataj-Kaphaj)  : GPIO 25
- RELAY_FAN              : GPIO 33
- WIFI_STATUS_LED        : GPIO 2
- MANUAL_SPRAY_SWITCH    : GPIO 34 (Input only)
- I2C LCD Display        : SDA - GPIO 21, SCL - GPIO 22
```

### Operational Parameters

- **WiFi Configuration**:
  - SSID: `SmartEnergyControlSystem`
  - Password: `12345678`
  - Firebase RTDB: `smartenergycontrolsystem-default-rtdb`
  - NTP Server: `pool.ntp.org`
  - Time Zone: IST (UTC+5:30)

- **Timing Parameters**:
  - WiFi check interval: 5000ms
  - Display refresh: 2000ms
  - Spray duration: 5000ms
  - Default spray interval: 5 minutes

---
## Web app : [Visit here](https://aromadiffuserdevice.netlify.app/)

## Screenshots

### Login Page
![image](https://github.com/user-attachments/assets/2bad69f0-12a6-4f1f-86d6-30f83f1a9134)


### Registration Page
![image](https://github.com/user-attachments/assets/594d399f-bbc8-43be-8d36-982a675debc1)

### Dashboard
![image](https://github.com/user-attachments/assets/3cb289eb-2d93-44ad-b145-9747d5093b2b)


---

## Contributing

Contributions are welcome! Please fork the repository and submit a pull request with your changes.

---

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

---

## Acknowledgments

- **Tailwind CSS** for the responsive design framework.
- **Firebase** for real-time database and authentication services.
- **Icons8** for the application icon.

---

For any questions or issues, please open an issue on the GitHub repository.
